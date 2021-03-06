// Write an action using cout. DON'T FORGET THE "<< endl"
// To debug: cerr << "Debug messages..." << endl;
// You have to output the target position
// followed by the power (0 <= thrust <= 100)
// i.e.: "x y thrust"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "math.h"

using namespace std;

const int MAP_WIDTH = 16000;
const int MAP_HEIGHT = 9000;
const int MAP_CENTER_X = MAP_WIDTH / 2;
const int MAP_CENTER_Y = MAP_HEIGHT / 2;
const int CHECKPOINT_RADIUS = 600;
const int POD_RADIUS = 400;	//Important seulement pour les collisions : seul le centre du pod passe les checkpoints
const int BOOST_THRUST_VALUE = 650;	//Le boost est une acceleration de 650

const int CHECKPOINT_BOOST_APPROACH_DIST = 6000;

const int CHECKPOINT_APPROACH_DIST_1 = 1500;
const int CHECKPOINT_APPROACH_THRUST_1 = 70;

const int CHECKPOINT_APPROACH_DIST_2 = 1300;
const int CHECKPOINT_APPROACH_THRUST_2 = 50;

const int CHECKPOINT_APPROACH_DIST_3 = 1100;
const int CHECKPOINT_APPROACH_THRUST_3 = 30;

const float PI = 3.14159;



//Retourne la troncature de la valeur (evite d'arrondir a l'inferieur les negatifs)
float truncate(float value){
	if (value >= 0) return floor(value);
	//Sinon, si la valeur est negative
	return ceil(value);
}

//Produit scalaire entre deux vecteurs
float dotProduct(float x1, float y1, float x2, float y2){
	return (x1*x2 + y1*y2);
}

//-------------------------------------------------------------------------------------


class Point {
public:
	int x, y;
	Point();
	Point(int t_x, int t_y);
	bool operator== (const Point& p);
	float distance(const Point& p);
	float distance2(const Point& p);
	Point* closest(const Point& a, const Point& b);
};

Point::Point(){
    this->x = 0;
    this->y =0;
}

Point::Point(int t_x, int t_y) {
	this->x = t_x;
	this->y = t_y;
}

bool Point::operator== (const Point& p) {
	if (x == p.x && y == p.y) return true;
	return false;
}

float Point::distance(const Point& p) {
    return sqrt(this->distance2(p));
}

//Evite d'utiliser sqrt si on veut le carre de la distance
float Point::distance2(const Point& p) {
    return (this->x - p.x)*(this->x - p.x) + (this->y - p.y)*(this->y - p.y);
}

//Retourne le point (tangente) le plus proche entre ce point et la droite (a;b)
Point* Point::closest(const Point& a, const Point& b){
	float da = b.y - a.y;
    float db = a.x - b.x;
    float c1 = da*a.x + db*a.y;
    float c2 = -db*this->x + da*this->y;
    float det = da*da + db*db;
    float cx = 0;
    float cy = 0;

    if (det != 0) {
        cx = (da*c1 - db*c2) / det;
        cy = (da*c2 + db*c1) / det;
    } else {
        // The point is already on the line
        cx = this->x;
        cy = this->y;
    }

    return new Point(cx, cy);
}


//-------------------------------------------------------------------------------------


class Unit : public Point{
public:
	int id;
	float r, vx, vy;
	Unit ();
	//void bounce(Unit u);
};

Unit::Unit (){
    this->x = 0;
    this->y = 0;
    this->id = 0;
    this->r = 0;
    this->vx = 0;
    this->vy = 0;
}



//-------------------------------------------------------------------------------------

/*
//TODO: declarer plus haut?
class Collision{
	public:
		Unit* a;
		Unit* b;
		float t; //Temps pendant le tour (compris entre 0.0 et 1.0)
		Collision (Unit* t_a, Unit* t_b, float t_t);
}

Collision::Collision (Unit* t_a, Unit* t_b, float t_t){
	this->a = t_a;
	this->b = t_b;
	this->t = t_t;
}
*/


//-------------------------------------------------------------------------------------


class Pod : public Unit{
public:
	float angle;	//(0:est, 90:sud, 180:ouest, 270:nord)
	int nextCheckpointId;
	int checked;	//TODO: idee non utilisee issue de Magnus : a retirer
	int timeout;	//100 tours sans passer de checkpoint = elimination
	Pod* partner;
	bool shield;	//Le shield est actif 3 tours
	Pod (int t_id);
	Pod (const Pod& podToClone); //clone
	Pod (float t_x, float t_y, int t_id, float t_vx, float t_vy, float t_angle, int t_nextCheckpointId, int t_checked, int t_timeout, Pod* t_partner, bool t_shield);
	void update(float pod_x, float pod_y, float pod_vx, float pod_vy, float pod_angle, int pod_nextCheckPointId);
	float getAngle(const Point& p);
	float diffAngle(const Point& p);
	void rotate(const Point& p);
	void boost(int thrust);
	void move(float t);
	void end();
	void play(const Point& p, int thrust);
	bool collision(const Unit& u);
	bool isCollisionPossibleWithMovement(const Point& myTarget, int thrust, Pod* pod, const Point& hisTarget, int hisThrust);
	float isCheckpointReachable(const Point& checkpoint, int thrust, const Point& movementTarget, float timeToReach);
};

Pod::Pod (int t_id){
	this->x = 0;
	this->y = 0;
	
	this->id = t_id;
	this->r = POD_RADIUS;
	this->vx = 0;
	this->vy = 0;
	
	this->angle = 0;
	this->nextCheckpointId = 1;
	this->checked = 0;
	this->timeout = 100;
	this->partner = nullptr;
	this->shield = false;
}

//Fonction de clonage
Pod::Pod (const Pod& podToClone){
	this->x = podToClone.x;
	this->y = podToClone.y;
	
	this->id = podToClone.id;
	this->r = podToClone.r;
	this->vx = podToClone.vx;
	this->vy = podToClone.vy;
	
	this->angle = podToClone.angle;
	this->nextCheckpointId = podToClone.nextCheckpointId;
	this->checked = podToClone.checked;
	this->timeout = podToClone.timeout;
	this->partner = podToClone.partner;
	this->shield = podToClone.shield;
}

Pod::Pod (float t_x, float t_y, int t_id, float t_vx, float t_vy, float t_angle, int t_nextCheckpointId, int t_checked, int t_timeout, Pod* t_partner, bool t_shield){
	this->x = t_x;
	this->y = t_y;
	
	this->id = t_id;
	this->r = POD_RADIUS;
	this->vx = t_vx;
	this->vy = t_vy;
	
	this->angle = t_angle;
	this->nextCheckpointId = t_nextCheckpointId;
	this->checked = t_checked;
	this->timeout = t_timeout;
	this->partner = t_partner;
	this->shield = t_shield;
}

void Pod::update(float pod_x, float pod_y, float pod_vx, float pod_vy, float pod_angle, int pod_nextCheckPointId){
    this->x = pod_x;
	this->y = pod_y;
	this->vx = pod_vx;
	this->vy = pod_vy;
	this->angle = pod_angle;
	this->nextCheckpointId = pod_nextCheckPointId;
}

//Retourne l'angle (0-359) d'un vecteur fait entre les deux points (0:est, 90:sud, 180:ouest, 270:nord)
float Pod::getAngle(const Point& p) {
    float d = this->distance(p);
    float dx = (p.x - this->x) / d;
    float dy = (p.y - this->y) / d;

    // Simple trigonometry. We multiply by 180.0 / PI to convert radiants to degrees.
    float a = acos(dx) * 180.0 / PI;

    // If the point I want is below me, I have to shift the angle for it to be correct
    if (dy < 0) {
        a = 360.0 - a;
    }

    return a;
}

//Retourne la rotation a effectuer pour faire face au point p (-180(gauche) a 180(droite))
float Pod::diffAngle(const Point& p) {
    float a = this->getAngle(p);

    // To know whether we should turn clockwise or not we look at the two ways and keep the smallest
    // The ternary operators replace the use of a modulo operator which would be slower
    float right = this->angle <= a ? a - this->angle : 360.0 - this->angle + a;
    float left = this->angle >= a ? this->angle - a : this->angle + 360.0 - a;

    if (right < left) {
        return right;
    } else {
        // We return a negative angle if we must rotate to left
        return -left;
    }
}

//Permet d'effectuer la rotation du pod pour faire face au point p
//en prenant en compte la limite de rotation de 18 degres par tour
void Pod::rotate(const Point& p) {
    float a = this->diffAngle(p);

    // Can't turn by more than 18 degrees in one turn
    if (a > 18.0) {
        a = 18.0;
    } else if (a < -18.0) {
        a = -18.0;
    }

    this->angle += a;

    // The % operator is slow. If we can avoid it, it s better.
    if (this->angle >= 360.0) {
        this->angle = this->angle - 360.0;
    } else if (this->angle < 0.0) {
        this->angle += 360.0;
    }
}

//Augmente la vitesse du pod selon la poussee (Le boost est une accelleration de 650)
void Pod::boost(int thrust) {
	// Don't forget that a pod which has activated its shield cannot accelerate for 3 turns
    if (this->shield) {
        return;
    }

    // Conversion of the angle to radiants
    float ra = this->angle * PI / 180.0;

    // Trigonometry
    this->vx += cos(ra) * thrust;
    this->vy += sin(ra) * thrust;
}

//Change la position du pod selon sa vitesse (t est le temps. t=1 pour effectuer un tour de jeu entier)
//t est ici utile pour calculer les collisions qui se produisent pendant le tour
void Pod::move(float t) {
    this->x += this->vx * t;
    this->y += this->vy * t;
}

//Applique la friction et tronque les valeurs
void Pod::end() {
    this->x = round(this->x);
    this->y = round(this->y);
    this->vx = truncate(this->vx * 0.85);
    this->vy = truncate(this->vy * 0.85);

    // Don't forget that the timeout goes down by 1 each turn. It is reset to 100 when you pass a checkpoint
    this->timeout -= 1;
}

//Permet de simuler les deplacement d un tour entier
void Pod::play(const Point& p, int thrust) {
    this->rotate(p);
    this->boost(thrust);
    this->move(1.0);
    this->end();
}

//True si une collision est possible dans les vitesses utilisees, false sinon
//TODO: COLLISIONS AVEC UN CHECKPOINT A REVOIR: LE CENTRE DOIT PASSER DANS LE RAYON, PAS DE CONTACT DE RADIUS!
bool Pod::collision(const Unit& u) {
	
	// We take everything squared to avoid calling sqrt uselessly. It is better for performances
    float dist = this->distance2(u);
	int sr = 2*POD_RADIUS*2*POD_RADIUS;
	
    if (dist < sr) {
        //Les pods sont deja en contact
        return true;
    }

    // On se place dans le referentiel de u, qui est stationnaire et a l'origine
	Point up = Point(0, 0);	//vx et vy = 0 pour u
	
    float x = this->x - u.x;
    float y = this->y - u.y;
    Point myp = Point(x, y);
    float vx = this->vx - u.vx;
    float vy = this->vy - u.vy;
    
	//Optimisation : des objets ne se rapprochant pas ne peuvent pas entrer en collision
	//On verifie avec le produit scalaire entre le vecteur de vitesse et celui allant de this a u
	//S'il est negatif, on s'ecarte de u. S'il est nul, vx et vy sont nuls
	float dotProductBetweenThisAndU = dotProduct(vx, vy, -x, -y);
	if (dotProductBetweenThisAndU <= 0){
		return false;
	}
	
    // We look for the closest point to u (which is in (0,0)) on the line described by our speed vector
    Point* p = up.closest(myp, Point(x + vx, y + vy));

    // Square of the distance between u and the closest point to u on the line described by our speed vector
    float pdist = up.distance2(*p);

    // Square of the distance between us and that point
    float mypdist = myp.distance2(*p);

    // If the distance between u and this line is less than the sum of the radii, there might be a collision
    if (pdist < sr) {
     // Our speed on the line
        float length = sqrt(vx*vx + vy*vy);

        // We move along the line to find the point of impact
        float backdist = sqrt(sr - pdist);
        p->x = p->x - backdist * (vx / length);
        p->y = p->y - backdist * (vy / length);

        // If the point is now further away it means we are not going the right way, therefore the collision won't happen
        if (myp.distance2(*p) > mypdist) {
            return false;
			//return null;
        }

        pdist = p->distance(myp);

        // The point of impact is further than what we can travel in one turn
        if (pdist > length) {
            return false;
			//return null;
        }

        // Time needed to reach the impact point
        float t = pdist / length;

		return true;
        //return new Collision(this, &u, t);
    }

	return false;
    //return null;
}

//Retourne true si une collision est cense arriver avec ce mouvement avec le pod
bool Pod::isCollisionPossibleWithMovement(const Point& myTarget, int thrust, Pod* pod, const Point& hisTarget, int hisThrust){
	Pod* clone = new Pod( ((const Pod&)*this));
	clone->rotate(myTarget);
    clone->boost(thrust);	//Rappel : augmente la vitesse mais ne deplace pas
	Pod* clone2 = new Pod( (const Pod&) *pod ); 
	clone2->rotate(hisTarget);
	clone2->boost(hisThrust);
	return clone->collision(*pod);
}

//Retourne le moment ou ce deplacement peut valider le checkpoint avec la puissance et le point target donne, et -1 sinon
float Pod::isCheckpointReachable(const Point& checkpoint, int thrust, const Point& movementTarget, float timeToReach){
	Pod* clone = new Pod( ((const Pod&)*this));
	clone->rotate(movementTarget);
	clone->boost(thrust);	//Rappel : augmente la vitesse mais ne deplace pas
    for (float time = 0.1; time <= timeToReach; time+= 0.1){
		clone->move(0.1);	//Rappel : augmente la vitesse mais ne deplace pas
		Point position (clone->x, clone->y);
		if (position.distance(checkpoint) < CHECKPOINT_RADIUS) return time;
	}
	return -1;
}



//-------------------------------------------------------------------------------------


class Checkpoint : public Unit{
public:
	Checkpoint (float t_x, float t_y, int t_id);
};

Checkpoint::Checkpoint (float t_x, float t_y, int t_id){
	this->x = t_x;
	this->y = t_y;
	
	this->id = t_id;
	this->r = CHECKPOINT_RADIUS;
	this->vx = 0;
	this->vy = 0;
}



//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------



int main()
{
	//Initialisation des inputs
	int laps;										//the number of laps to complete the race
	int checkpointCount; 							//the number of checkpoints in the circuit
	cin >> laps >> checkpointCount; cin.ignore();
	
	Checkpoint* checkpointList [checkpointCount];
	for (int i=0; i<checkpointCount; i++){
		int cx, cy;
		cin >> cx >> cy; cin.ignore();
		checkpointList[i] = new Checkpoint (cx, cy, i);
	}

	//Instanciation des pods
	Pod* podList [4];	//0 et 1 sont mes pods, 2 et 3 sont les ennemis
	for (int i=0; i<4; i++){
		podList[i] = new Pod(i);
	}
	podList[0]->partner = podList[1];
	podList[1]->partner = podList[0];
	podList[2]->partner = podList[3];
	podList[3]->partner = podList[2];

	bool isBoostAvailable = true;
	int turn = 0;
	
	int thrust, destinationX, destinationY;
	
	
	// game loop
	while (1) {
		
		//Mise a jour des attributs des pods pour ce tour
		for (int i=0; i<4; i++){
			Pod* pod = podList[i];
			int pod_x, pod_y, pod_vx, pod_vy, pod_angle, pod_nextCheckPointId;
			cin >> pod_x >> pod_y >> pod_vx >> pod_vy >> pod_angle >> pod_nextCheckPointId; cin.ignore();
			
			pod->update(pod_x, pod_y, pod_vx, pod_vy, pod_angle, pod_nextCheckPointId);
			pod->timeout = pod->timeout-1;
		}
		
		//Temporairement : on fait tourner un algo pour les deux pods
		for (int podNumber = 0; podNumber<2; podNumber++){
			Pod* pod = podList[podNumber];
			Checkpoint* nextCheckpoint = checkpointList[pod->nextCheckpointId];
			Point* nextCheckpointCoords = new Point(nextCheckpoint->x, nextCheckpoint->y);
			
			
			int distTowardCheckpoint = pod->distance(*nextCheckpointCoords);
			int diffAngleTowardCheckpoint = pod->diffAngle(*nextCheckpointCoords);
			
			
			destinationX = nextCheckpoint->x;
			destinationY = nextCheckpoint->y;
			
			//Si le checkpoint est derriere nous, on accellere pas, pour ne pas s'en eloigner
			if (diffAngleTowardCheckpoint > 90 || diffAngleTowardCheckpoint < -90) {
				thrust = 0;
			}
			else {
				thrust = 100 - (diffAngleTowardCheckpoint*100/90);	//TODO: retirer? Ameliorer?
				
				//On ralentit en approchant
				if (distTowardCheckpoint < CHECKPOINT_APPROACH_DIST_1) {
					thrust = CHECKPOINT_APPROACH_THRUST_1;
				}
				if (distTowardCheckpoint < CHECKPOINT_APPROACH_DIST_2) {
					thrust = CHECKPOINT_APPROACH_THRUST_2;
				}
				if (distTowardCheckpoint < CHECKPOINT_APPROACH_DIST_3) {
					thrust = CHECKPOINT_APPROACH_THRUST_3;
				}
				
				Checkpoint* checkpointAfterward;
				//On verifie si on peut tourner tout de suite vers le prochain checkpoint, en diminuant la vitesse au fur et a mesure
				if (pod->nextCheckpointId + 1 >= checkpointCount){
					checkpointAfterward = checkpointList[0];
				}
				else{
					checkpointAfterward = checkpointList[pod->nextCheckpointId + 1];
				}
				Point checkpointAfterwardCoords (checkpointAfterward->x, checkpointAfterward->y);
				float timeTakenToReachCheckpoint = -1;
				bool activateShield = false;
				int testThrust = thrust; //100;
				for (; timeTakenToReachCheckpoint > -1 || testThrust > 10; testThrust-=10){
					timeTakenToReachCheckpoint = pod->isCheckpointReachable(*nextCheckpointCoords, testThrust, checkpointAfterwardCoords, 3.0);
				}
				if (timeTakenToReachCheckpoint != -1){
					for (int i=0; activateShield || i<4; i++){
						if (i != pod->id){
							Pod* otherPod = podList[i];
							Checkpoint* otherPodCheckpoint = checkpointList[otherPod->nextCheckpointId];
							Point otherPodTarget (otherPodCheckpoint->x, otherPodCheckpoint->y);
							activateShield = pod->isCollisionPossibleWithMovement(checkpointAfterwardCoords, testThrust, otherPod, otherPodTarget, 100);
						}
					}
					//Si on peut tourner tout de suite en validant le checkpoint actuel, on le fait
					if (activateShield){
						thrust = -1;
					}
					else{
						cerr << "On peut passer le checkpoint : on tourne directement" << endl;
						thrust = testThrust;
					}
					destinationX = checkpointAfterwardCoords.x;
					destinationY = checkpointAfterwardCoords.y;
				}
				else{
					//Si le boost est disponible et qu'on est assez loin, on boost
					if (isBoostAvailable
						&& distTowardCheckpoint >= CHECKPOINT_BOOST_APPROACH_DIST
						&& diffAngleTowardCheckpoint < 20
						&& diffAngleTowardCheckpoint > -20) {
						thrust = 999;   //BOOST
						isBoostAvailable = false;
					}
				
				}
				
			}
			
			if (thrust <= 100 && thrust >= 0) {
				cout << destinationX << " " << destinationY << " " << thrust << " " << diffAngleTowardCheckpoint << endl;
			}
			if (thrust > 100) {
				cout << destinationX << " " << destinationY << " " << "BOOST BOOST" << endl;
			}
			if (thrust < 0) {
				cout << destinationX << " " << destinationY << " " << "SHIELD SHIELD" << endl;
			}
			
			
		}
		
		turn++;
	}
}