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
	//Collision* collision(Unit u);
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

/*
//Cherche et retourne la potentielle collision effectuee avec une autre unite (null sinon)
//TODO: temps reel? Dixiemes de tours de rotation max 1.8 ?
//TODO: COLLISIONS AVEC UN CHECKPOINT A REVOIR: LE CENTRE DOIT PASSER DANS LE RAYON, PAS DE CONTACT DE RADIUS!
Collision* Unit::collision(Unit u) {
    // Square of the distance
    float dist = this->distance2(u);

    // Sum of the radii squared
    float sr = (this->r + u.r)*(this->r + u.r);

    // We take everything squared to avoid calling sqrt uselessly. It is better for performances

    if (dist < sr) {
        // Objects are already touching each other. We have an immediate collision.
        return new Collision(this, u, 0.0);
    }

	//TODO: ameliorer (vitesse plus faible que u en etant derriere, ou vitesses d'angles > 90)
    // Optimisation. Objects with the same speed will never collide
    if (this->vx == u.vx && this->vy == u.vy) {
        return null;
    }

    // We place ourselves in the reference frame of u. u is therefore stationary and is at (0,0)
    float x = this->x - u.x;
    float y = this->y - u.y;
    Point myp = new Point(x, y);
    float vx = this->vx - u.vx;
    float vy = this->vy - u.vy;
    Point up = new Point(0, 0)

    // We look for the closest point to u (which is in (0,0)) on the line described by our speed vector
    Point p = up.closest(myp, new Point(x + vx, y + vy));

    // Square of the distance between u and the closest point to u on the line described by our speed vector
    float pdist = up.distance2(p);

    // Square of the distance between us and that point
    float mypdist = myp.distance2(p);

    // If the distance between u and this line is less than the sum of the radii, there might be a collision
    if (pdist < sr) {
     // Our speed on the line
        float length = sqrt(vx*vx + vy*vy);

        // We move along the line to find the point of impact
        float backdist = sqrt(sr - pdist);
        p.x = p.x - backdist * (vx / length);
        p.y = p.y - backdist * (vy / length);

        // If the point is now further away it means we are not going the right way, therefore the collision won't happen
        if (myp.distance2(p) > mypdist) {
            return null;
        }

        pdist = p.distance(myp);

        // The point of impact is further than what we can travel in one turn
        if (pdist > length) {
            return null;
        }

        // Time needed to reach the impact point
        float t = pdist / length;

        return new Collision(this, &u, t);
    }

    return null;
}

//Calcul du rebond apres une collision (dans le cas d'un checkpoint, il est nul)
//TODO: COLLISIONS AVEC UN CHECKPOINT A REVOIR: LE CENTRE DOIT PASSER DANS LE RAYON, PAS DE CONTACT DE RADIUS!
void Unit::bounce(Unit u) {
    if (u instanceof Checkpoint) {
        // Collision with a checkpoint
        this.bounceWithCheckpoint((Checkpoint) u);
    } else {
        // If a pod has its shield active its mass is 10 otherwise it's 1
        float m1 = this.shield ? 10 : 1;
        float m2 = u.shield ? 10 : 1;
        float mcoeff = (m1 + m2) / (m1 * m2);

        float nx = this.x - u.x;
        float ny = this.y - u.y;

        // Square of the distance between the 2 pods. This value could be hardcoded because it is always 800²
        float nxnysquare = nx*nx + ny*ny;

        float dvx = this.vx - u.vx;
        float dvy = this.vy - u.vy;

        // fx and fy are the components of the impact vector. product is just there for optimisation purposes
        float product = nx*dvx + ny*dvy;
        float fx = (nx * product) / (nxnysquare * mcoeff);
        float fy = (ny * product) / (nxnysquare * mcoeff);

        // We apply the impact vector once
        this.vx -= fx / m1;
        this.vy -= fy / m1;
        u.vx += fx / m2;
        u.vy += fy / m2;

        // If the norm of the impact vector is less than 120, we normalize it to 120
        float impulse = sqrt(fx*fx + fy*fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // We apply the impact vector a second time
        this.vx -= fx / m1;
        this.vy -= fy / m1;
        u.vx += fx / m2;
        u.vy += fy / m2;

        // This is one of the rare places where a Vector class would have made the code more readable.
        // But this place is called so often that I can't pay a performance price to make it more readable.
    }
}
*/


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
	int checked;	//TODO: a comprendre
	int timeout;	//100 tours sans passer de checkpoint = elimination
	Pod* partner;
	bool shield;	//Le shield est actif 3 tours
	Pod (int t_id);
	Pod (float t_x, float t_y, int t_id, float t_vx, float t_vy, float t_angle, int t_nextCheckpointId, int t_checked, int t_timeout, Pod* t_partner, bool t_shield);
	void update(float pod_x, float pod_y, float pod_vx, float pod_vy, float pod_angle, int pod_nextCheckPointId);
	float getAngle(const Point& p);
	float diffAngle(const Point& p);
	void rotate(const Point& p);
	void boost(int thrust);
	void move(float t);
	void end();
	void play(const Point& p, int thrust);
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
			
			int newAngle = pod->angle + diffAngleTowardCheckpoint;
			if (newAngle >= 360.0) {
				newAngle = newAngle - 360.0;
			} else if (newAngle < 0.0) {
				newAngle += 360.0;
			}
			//Passage en radian
			newAngle = newAngle * PI / 180.0;
			//Recuperation d'un point eloigne situe dans la bonne direction
			float angleTargetX = pod->x + cos(newAngle) * 10000.0;
			float angleTargetY = pod->y + sin(newAngle) * 10000.0;
			destinationX = angleTargetX;
			destinationY = angleTargetY;
			
			thrust = 0;
			//Si le checkpoint est derriere nous, on accellere pas, pour ne pas s'en eloigner
			if (diffAngleTowardCheckpoint > 90 || diffAngleTowardCheckpoint < -90) {
				thrust = 0;
			}
			else {
				thrust = 100;
				
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
				
				//Si le boost est disponible et qu'on est assez loin, on boost
				if (isBoostAvailable
					&& distTowardCheckpoint >= CHECKPOINT_BOOST_APPROACH_DIST
					&& diffAngleTowardCheckpoint < 20
					&& diffAngleTowardCheckpoint > -20) {
					thrust = 999;   //BOOST
					isBoostAvailable = false;
				}
				
				//TODO: Evidemment temporaire pour tester
				//Si on pense avoir le checkpoint
				if ( (diffAngleTowardCheckpoint < 45 || diffAngleTowardCheckpoint > -45) && distTowardCheckpoint < CHECKPOINT_RADIUS*3) {
					int otherCheckpointId = pod->nextCheckpointId + 1;
					if (otherCheckpointId >= checkpointCount) otherCheckpointId = 0;
					Checkpoint* otherCheckpoint = checkpointList[otherCheckpointId];
					
					thrust = 10;
					destinationX = otherCheckpoint->x;
					destinationY = otherCheckpoint->y;
				}
			}
			
			if (thrust <= 100 && thrust >= 0) {
				cout << destinationX << " " << destinationY << " " << thrust << " " << thrust << endl;
			}
			if (thrust > 100) {
				cout << destinationX << " " << destinationY << " " << "BOOST BOOST" << endl;
			}
			if (thrust < 0) {
				cout << destinationX << " " << destinationY << " " << "SHIELD SHIELD" << endl;
			}
			
			/*
			void output(Move* move) {
				float a = angle + move.angle;

				if (a >= 360.0) {
					a = a - 360.0;
				} else if (a < 0.0) {
					a += 360.0;
				}

				// Look for a point corresponding to the angle we want
				// Multiply by 10000.0 to limit rounding errors
				a = a * PI / 180.0;
				float px = this.x + cos(a) * 10000.0;
				float py = this.y + sin(a) * 10000.0;

				if (move.shield) {
					print(round(px), round(py), "SHIELD");
					activateShield();
				} else {
					print(round(px), round(py), move.power);
				}
			}
			*/
			
			
		}
		
		
		
		/*
		int thrust = 0;

		//Si le checkpoint est derriere nous, on accellere pas, pour ne pas s'en eloigner
		if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90) {
			thrust = 0;
		}
		else {
			thrust = 100;


			//On ralentit en approchant
			if (nextCheckpointDist < CHECKPOINT_APPROACH_DIST_1) {
				thrust = CHECKPOINT_APPROACH_THRUST_1;
			}
			if (nextCheckpointDist < CHECKPOINT_APPROACH_DIST_2) {
				thrust = CHECKPOINT_APPROACH_THRUST_2;
			}
			if (nextCheckpointDist < CHECKPOINT_APPROACH_DIST_3) {
				thrust = CHECKPOINT_APPROACH_THRUST_3;
			}



			//Calcul de la position approximative du tour suivant, pour verifier si on sera dans le checkpoint
			Point nextPosition(x + 3 * velocityX, y + 3 * velocityY);

			cerr << "nextCheckpointX : " << x + 2 * velocityX << endl;
			cerr << "nextCheckpointY : " << y + 2 * velocityY << endl;
			cerr << "nextCheckpoint.getDistance(nextPosition) : " << nextCheckpoint.distance(nextPosition) << endl;

			//Si on sait qu'on sera dans le point, on tourne deja vers le centre pour se preparer
			if (nextCheckpoint.getDistance(nextPosition) < CHECKPOINT_RADIUS) {
				thrust = 10;
				destinationX = MAP_CENTER_X;
				destinationY = MAP_CENTER_Y;
			}


			//Si le boost est disponible et qu'on est assez loin, on boost
			if (isBoostAvailable
				&& nextCheckpointDist >= CHECKPOINT_BOOST_APPROACH_DIST
				&& nextCheckpointAngle < 20
				&& nextCheckpointAngle > -20) {
				thrust = 999;   //BOOST
				isBoostAvailable = false;
			}
		}


		if (thrust <= 100 && thrust >= 0) {
			cout << destinationX << " " << destinationY << " " << thrust << " " << thrust << endl;
		}
		if (thrust > 100) {
			cout << destinationX << " " << destinationY << " " << "BOOST BOOST" << endl;
		}
		if (thrust < 0) {
			cout << destinationX << " " << destinationY << " " << "SHIELD SHIELD" << endl;
		}
		*/
		turn++;
	}
}
