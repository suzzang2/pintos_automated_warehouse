#include "projects/automated_warehouse/robot.h"

/**
 * A function setting up robot structure
 */
void setRobot(struct robot* _robot, const char* name, int row, int col, int required_payload, int current_payload, char dest){
    _robot->name = name;
    _robot->row = row;
    _robot->col = col;
    _robot->required_payload = required_payload;
    _robot->current_payload = current_payload;
    _robot->dest = dest;
    
    _robot->cycle_step_index = 0;

    _robot->payloadFlag = 0; //이것도 나중에는 message로 보내는 로로 바꿔야함....
    _robot->moveFlag = 0; //0이면 움직이는 거
}

void stopOtherRobots(struct robot* robots, int numOfRobot, int moveRobotIdx, int Round){ 
    for(int i = Round*6; i<(Round+1)*6 && i<numOfRobot; i++){
        if(i != (moveRobotIdx)){ //해당 로봇이 아닌 로봇들은
            robots[i].moveFlag = 1; //모두 정지
        }
    }
}

int all_robots_have_payload(struct robot *robots, int numOfRobot, int Round) {
    for (int i = Round*6; i<(Round+1)*6 && i<numOfRobot; i++) {
        if (robots[i].current_payload <= 0) {
            return 0;
        }
    }
    return 1;
}

int check_twotwo(struct robot* robots, int numOfRobot, int Round){
    for(int i = Round*6; i<(Round+1)*6 && i<numOfRobot; i++){
        if(robots[i].row == 2 && robots[i].col == 2){
            return 1; // (2,2)에 로봇이 존재함
        }
    }
    return 0;
}