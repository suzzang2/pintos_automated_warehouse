#ifndef _PROJECTS_PROJECT1_ROBOT_H__
#define _PROJECTS_PROJECT1_ROBOT_H__

/**
 * A Structure representing robot
 */
struct robot {
    const char* name;
    int row;
    int col;
    int required_payload;
    int current_payload; 
    /**/
    char dest; //하역장소 목적지
    int cycle_step_index;

    int payloadFlag;
    int moveFlag;
};

void setRobot(struct robot* _robot, const char* name, int row, int col, int required_payload, int current_payload, char dest);
void stopOtherRobots(struct robot* robots, int numOfRobot, int moveRobotIdx, int Round);//moveRobotIdx는 1부터 명시적 숫자
int all_robots_have_payload(struct robot *robots, int numOfRobots, int Round);
int check_twotwo(struct robot* robots, int numOfRobot, int Round);

#endif