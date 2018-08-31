#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Write an action using cout. DON'T FORGET THE "<< endl"
// To debug: cerr << "Debug messages..." << endl;

int main()
{
    final int CHECKPOINT_APPROACH_DIST = 1000;
    
    vector<Checkpoint> checkpointList;
    int currentCheckpointNumber = 0;
    

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        
        Checkpoint currentCheckpoint = new Checkpoint
        addNewCheckpoint
        
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();


        
        int thrust = 0;
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90 || nextCheckpointDist < 1000){
            thrust = 0;
        }
        else {
            thrust = 100;
            if (nextCheckpointDist < 1000) thrust = 30;
        }

        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust << endl;
    }
}

//Ajoute un nouveau checkpoint a la liste s'il est different du depart et de l'actuel
void addNewCheckpoint(vector<Checkpoint> checkpointList, Checkpoint current, int checkpointNumber){
    if ()
}

class Checkpoint{
    public:
        int number;   //0 est le point de depart
        int x;
        int y;
        Checkpoint ()
}