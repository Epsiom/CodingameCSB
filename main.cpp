// Write an action using cout. DON'T FORGET THE "<< endl"
// To debug: cerr << "Debug messages..." << endl;
// You have to output the target position
// followed by the power (0 <= thrust <= 100)
// i.e.: "x y thrust"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "math.h";

using namespace std;

const int MAP_WIDTH = 16000;
const int MAP_HEIGHT = 9000;
const int MAP_CENTER_X = MAP_WIDTH/2;
const int MAP_CENTER_Y = MAP_HEIGHT/2;
const int CHECKPOINT_RADIUS = 600;
const int POD_RADIUS = 400;	//Important seulement pour les collisions : seul le centre du pod passe les checkpoints

const int CHECKPOINT_BOOST_APPROACH_DIST = 6000;
const int CHECKPOINT_APPROACH_DIST = 4000;
const int CHECKPOINT_APPROACH_THRUST = 70;

//TODO : enlever
const int TURN_APPROACH_MIN = 30;

//(x - (speed.valueX() * 3)
//It corrects your aim for centrifugal force

/*
rotation is limited to +/- 18 deg,
accelleration is ignored,
velocity is (to - cur).normalize() * thrust * friction,
and friction is 0.85
*/

/*
class Checkpoint {
    private:
        int number;   //"nom" du checkpoint : 0 est le point de depart
        int x;
        int y;
	public:
        Checkpoint(int t_number, int t_x, int t_y) : number(t_number), x(t_x), y(t_y) {}
		bool operator==(const Checkpoint& a, const Checkpoint& b) {
			if (a.getNumber() == b.getNumber() && a.getX() == b.getX() && a.getY() == b.getY()) return true;
			return false;
		}
		int getNumber() const { return number; }
		int getX() const { return x; }
		int getY() const { return y; }
};

//Ajoute un nouveau checkpoint a la liste s'il est different du depart et de l'actuel, et retourne alors true dans ce cas
bool addNewCheckpoint(vector<Checkpoint> checkpointList, Checkpoint currentCheckpoint, int currentCheckpointNumber){
	//TODO: PASSAGES PAR POINTEUR ET ALLOCATION!
	
    //Pas de probleme sur back, qui n'est pas evalue si la liste est vide
	if (checkpointList.empty() || checkpointList.back() != currentCheckpoint){
		
		//TODO: ajouter le depart initialement si la liste est vide
		
		checkpointList.push_back(currentCheckpoint);
		return true;
	}
	return false;
}
*/

class Point {
    private:
        int x;
        int y;
	public:
		Point(int t_x, int t_y);
		bool operator== (const Point& p);
		double getDistance(const Point& p);
		int getX() const { return x; }
		int getY() const { return y; }
};

Point::Point(int t_x, int t_y) {
	this->x = t_x;
	this->y = t_y;
}

bool Point::operator== (const Point& p) {
	if (x == p.getX() && y == p.getY()) return true;
	return false;
}

double Point::getDistance(const Point& p){
	int distancex = (p.getX() - x) * (p.getX() - x);
	int distancey = (p.getY() - y) * (p.getY() - y);
	return sqrt(distancex + distancey); 
}



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
		
		int velocityX = x-previousX;
		int velocityY = y-previousY;
		int velocity = (int)sqrt(velocityX*velocityX + velocityY*velocityY);
		
		Point pod (x, y);
		Point nextCheckpoint (nextCheckpointX, nextCheckpointY);
		
		
		//Si le checkpoint est derriere nous, on accellere pas, pour ne pas s'en eloigner
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90){
            thrust = 0;
        }
		else{
			thrust = 100;
			
			//On ralentit en approchant
			if (nextCheckpointDist < CHECKPOINT_APPROACH_DIST) {
				thrust = CHECKPOINT_APPROACH_THRUST;
			}


			//Calcul de la position approximative du tour suivant, pour verifier si on sera dans le checkpoint
			Point nextPosition(x - 3 * velocityX, y - 3 * velocityY);

			cerr << nextCheckpoint.getDistance(nextPosition) << endl;

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
					&& nextCheckpointAngle > -20){
			    thrust = 999;   //BOOST
			    isBoostAvailable = false;
			}
        }
		
		turnsSinceLastCheckpoint++;
		if (thrust <= 100){
		    cout << destinationX << " " << destinationY << " " << thrust << " " << thrust << endl;
		}
		else{
			cout << destinationX << " " << destinationY << " " << "BOOST BOOST" << endl;
		}
    }
}


