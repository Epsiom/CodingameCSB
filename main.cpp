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


//(x - (speed.valueX() * 3)
//It corrects your aim for centrifugal force

/*
rotation is limited to +/- 18 deg,
accelleration is ignored,
velocity is (to - cur).normalize() * thrust * friction,
and friction is 0.85
*/


class Point {
public:
	int x, y;
	Point(int t_x, int t_y);
	bool operator== (const Point& p);
	float distance(const Point& p);
	float distance2(const Point& p);
	Point closest(const Point& a, const Point& b);
};

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

Point Point::closest(const Point& a, const Point& b){
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



class Unit : public Point{
public:
	int id;
	float r, vx, vy;
};



class Pod : public Unit{
public:
	float angle;	//(0:est, 90:sud, 180:ouest, 270:nord)
	int nextCheckpointId;
	int checked;	//TODO: a comprendre
	int timeout;	//100 tours sans passer de checkpoint = elimination
	Pod partner;
	bool shield;	//Le shield est active 3 tours
	float getAngle(const Point& p);
	float diffAngle(const Point& p);
	void rotate(const Point& p);
	void boost(int thrust);
	void move(float t);
};

//Retourne l'angle (0-359) d'un vecteur fait entre les deux points (0:est, 90:sud, 180:ouest, 270:nord)
float Pod::getAngle(const Point& p) {
    float d = this->distance(p);
    float dx = (p->x - this->x) / d;
    float dy = (p->y - this->y) / d;

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

//Retourne la troncature de la valeur (evite d'arrondir a l'inferieur les negatifs)
float truncate(float value){
	if (value >= 0) return floor(value);
	//Sinon, si la valeur est negative
	return ceil(value);
}

//Applique la friction et tronque les valeurs
void end() {
    this.x = round(this->x);
    this.y = round(this->y);
    this.vx = truncate(this->vx * 0.85);
    this.vy = truncate(this->vy * 0.85);

    // Don't forget that the timeout goes down by 1 each turn. It is reset to 100 when you pass a checkpoint
    this.timeout -= 1;
}

//Permet de simuler les deplacement d un tour entier
void play(const Point& p, int thrust) {
    this.rotate(p);
    this.boost(thrust);
    this.move(1.0);
    this.end();
}



/*
class Checkpoint : public Unit{
public:
	
};*/



int main()
{

	/*
	vector<Checkpoint> checkpointList;
	int lapNumber = 0;

	*/

	//Permet temporairement de verifier si on a change de checkpoint
	int previousCheckpointX = -1;
	int previousCheckpointY = -1;
	int turnsSinceLastCheckpoint = 0;
	int checkpointNumber = 0;
	bool isBoostAvailable = true;

	int previousX = 0;
	int previousY = 0;


	// game loop
	while (1) {
		int x;
		int y;
		int nextCheckpointX; // x position of the next check point
		int nextCheckpointY; // y position of the next check point
		int nextCheckpointDist; // distance to the next checkpoint
		int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
		cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
		int opponentX;
		int opponentY;
		cin >> opponentX >> opponentY; cin.ignore();


		int thrust = 0;
		int destinationX = nextCheckpointX;
		int destinationY = nextCheckpointY;

		int velocityX = x - previousX;
		int velocityY = y - previousY;
		int velocity = (int)sqrt(velocityX*velocityX + velocityY * velocityY);

		Point pod(x, y);
		Point nextCheckpoint(nextCheckpointX, nextCheckpointY);


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

		turnsSinceLastCheckpoint++;
		previousX = x;
		previousY = y;

	}
}
