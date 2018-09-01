// Write an action using cout. DON'T FORGET THE "<< endl"
// To debug: cerr << "Debug messages..." << endl;
// You have to output the target position
// followed by the power (0 <= thrust <= 100)
// i.e.: "x y thrust"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAP_WIDTH = 16000;
const int MAP_HEIGHT = 9000;
const int MAP_CENTER_X = MAP_WIDTH/2;
const int MAP_CENTER_Y = MAP_HEIGHT/2;
const int CHECKPOINT_RADIUS = 350;

const int CHECKPOINT_APPROACH_DIST = 1000;
const int TURN_APPROACH_MIN = 30;

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
		
		
		//Le premier tour, ajoute les checkpoint apres leur passage
        Checkpoint currentCheckpoint (currentCheckpointNumber, nextCheckpointX, nextCheckpointY);
		if (lapNumber == 0){
			bool checkpointChanged = addNewCheckpoint(checkpointList, currentCheckpoint, currentCheckpointNumber);
			if (checkpointChanged){
				currentCheckpointNumber++;
				turnsSinceLastCheckpoint = 0;
			}
		}
		
		
		//Permet temporairement de verifier si on a change de checkpoint
		if (previousCheckpointX != nextCheckpointX || previousCheckpointY != nextCheckpointY){
			
		}
        
        int thrust = 0;
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90 || nextCheckpointDist < 1000){
            thrust = 0;
        }
        else {
            thrust = 100;
            if (nextCheckpointDist < 1000) thrust = 30;
        }
		
		
		//
		if (nextCheckpointDist < CHECKPOINT_APPROACH_DIST){
			
		}

        cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust << endl;
    }
}

