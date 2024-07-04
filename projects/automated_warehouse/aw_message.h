#ifndef _PROJECTS_PROJECT1_AW_MESSAGE_H__
#define _PROJECTS_PROJECT1_AW_MESSAGE_H__

/**
 * For easy to implement, combine robot and central control node message
 * If you want to modify message structure, don't split it
 */
struct message {
    //
    // To central control node
    //
    /** current row of robot */
    int row;
    /** current column of robot */
    int col;
    /** current payload of robot */
    int current_payload;
    /** required payload of robot */
    int required_payload;

    int cycle_step_index;
    int payloadFlag;
    int moveFlag;
    int dest;

    //
    // To robots
    //
    /** next command for robot */
    int cmd;
};

/** 
 * Simple message box which can receive only one message from sender
*/
struct message_box {
    /** check if the message was written by others */
    int dirtyBit;
    /** stored message */
    struct message msg;
};

/** message boxes from central control node to each robot */
extern struct message_box* box_from_center;
/** message boxes from robots to central control node */
extern struct message_box* box_from_robot;


#endif