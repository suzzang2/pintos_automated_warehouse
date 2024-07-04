#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "aw_message.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"

#define up 1
#define down 2
#define right 3
#define left 4
#define p_up 5
#define p_down 6
#define p_right 7
#define p_left 8

struct payload_coordinate
{
        int row;
        int col;
};
struct payload_coordinate payload_coordinate[7] = { // 지도에서의 물건 위치를 키 값으로 좌표 표현
    {ROW_1, COL_1},
    {ROW_2, COL_2},
    {ROW_3, COL_3},
    {ROW_4, COL_4},
    {ROW_5, COL_5},
    {ROW_6, COL_6},
    {ROW_7, COL_7}};


struct robot *robots;
struct message_box *box_from_center;
struct message_box *box_from_robot;
int numOfRobot;
int* destination; //이것도 나중에 로봇으로 다 넣어서 메세지로만 전달해주기...
int step_cycle[16] = {up, up, //0~1
                        up, left, left, left, left, down, right, right, right, right, //2~11
                        up, left, left, left}; //12~15
int level = 1; //전체 반복(printmap기준)을 세는 변수
int stopCheck = 0;
int putReadyCheck = 0; // 모든 로봇이 작업을 완료하여 하역을 시작할 준비가 되었는지 확인하는 변수
int putCount = 0; // 몇 개의 로봇이 하역을 완료하고 대기 중인지 확인하는 변수(그룹 바뀌어도 초기화 안 됨)
int putting = 0; // 로봇이 하역을 위해 이동 중에 있는지 확인하는 변수


// int flag = 0; //cnt를 unblock할지 결정하는 flag
int count = 0;

//for 그룹화
int initRound = 0;
int totalRound;
int countInGroup = 0; //한 그룹 내에서만 사용할, 하역한 화물 수(0~6)

void cnt_thread()
{       
        // printf("<<cnt_thread 호출>>\n");
        while (1)
        {       printf(" ***지금까지 하역한 로봇 수 : %d\n", putCount);
                // numOfRobot이 6 이하일 때
                if(numOfRobot <= 6){
                        if(putCount == numOfRobot){ //모든 로봇이 하역을 완료하면 프로그램 종료
                                printf("❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️모든 물건 적재 완료❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️\n");
                                printf("------------ 최종 적재 Info ------------\n");
                                print_map(robots, numOfRobot);
                                shutdown_power_off();
                                block_thread();
                        }
                }

                // numOfRobot이 6 초과일 때
                else{
                        if(putCount == numOfRobot){ //모든 로봇이 하역을 완료하면 프로그램 종료
                                printf("❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️모든 물건 적재 완료❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️\n");
                                printf("------------ 최종 적재 Info ------------\n");
                                print_map(robots, numOfRobot);
                                shutdown_power_off();
                                block_thread();
                        }
                        if(countInGroup == 6){
                                if(initRound == totalRound-1){ //마지막 round일 때 -> 로봇이 6개 이하일 때와 같은 조건
                                        printf("❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️모든 물건 적재 완료❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️\n");
                                        printf("------------ 최종 적재 Info ------------\n");
                                        print_map(robots, numOfRobot);
                                        shutdown_power_off();
                                        block_thread();
                                }ㄷ
                                else{   //남은 로봇 수가 6개 초과일 때 -> 다음 그룹으로 넘어가기 구현
                                        countInGroup = 0; //countInGroup 초기화
                                        initRound++; //다음 그룹으로 넘어가기
                                        putReadyCheck = 0; //다시 하역 준비 안 됨으로 초기화
                                        putting = 0; //다시 하역 이동 중이 아님으로 초기화
                                        stopCheck = 0; //다시 stopCheck 초기화
                                        level = initRound*6 + 1; //다음 그룹의 level로 초기화
                                        printf("////////// 다음 그룹으로 넘어갑니다!!! ///////////\n");
                                }
                        }
                }

                if(robots[initRound*6].row == 2 && robots[initRound*6].col == 2 
                        && all_robots_have_payload(robots, numOfRobot, initRound) == 1)
                                {   putReadyCheck = 1; }

                printf("#########  %d단계  #########\n", step);
                print_map(robots, numOfRobot);
                // thread_sleep(100);
                increase_step(); 

                // 구현 -> @@@@@@@@@@@메세지 보내기@@@@@@@@@@@@@중요
                // 진짜 가장 맨 처음으로 확인해야 할 것 ->> 하역 장소로 가기 위한 준비 완료인지(2, 2)!!!

                // 아직 하역 준비 안 됨
                if(putReadyCheck == 0){ 
                        printf("////////// 하역 준비 중... ///////////\n");
                        // 보내기 전에, 위 or 아래에 집을 물건이 있는지 확인해야함
                        for (int i=initRound*6; i<((initRound+1)*6) && i<numOfRobot ; i++)
                        {       struct message msgFromRobot = box_from_robot[i].msg; //이거로만 데이터 접근해야함!
                                struct message msg;
                                printf("cycle_step_index : %d\n", robots[i].cycle_step_index);
                                /////////////////총 3개의 조건문//////////////////
                                // <1> 이미 출발한 로봇인지
                                if(level > i){ //level이 i보다 크거나 같을 때만 움직임 -> 동시에 같이 출발하는 거 방지!!
                                        //0주기 & 1주기(step_cycle이 0~11)
                                        if(msgFromRobot.cycle_step_index <= 11){
                                                if(msgFromRobot.payloadFlag == 1){ //이미 로봇이 물건들기 왔다갔다를 했으면 -> 이제는 step_cycle따라서 이동
                                                        msg.cmd = step_cycle[msgFromRobot.cycle_step_index];
                                                }
                                                else{ //왔다갔다를 해야되면 ->다른 로봇들 대기 필요!!@@@
                                                if(msgFromRobot.current_payload != 0){ //물건을 들고 있으면
                                                        if(destination[i]>=1 && destination[i]<=4){ //1~4번 물건이면
                                                                msg.cmd = p_down; //다시 내려오기(갖고 있는 상태로) 
                                                                robots[i].payloadFlag = 1; //물건 들고 있음을 표시
                                                        }
                                                        else if(destination[i]>=5){ //5~7번 물건이면(갖고 있는 상태로)
                                                                msg.cmd = p_up; //다시 올라가기
                                                                robots[i].payloadFlag = 1; //물건 들고 있음을 표시
                                                        }

                                                        //근데 이미 앞에서 stopOtherRobots함수를 통해 다른 로봇들을 stop시켜줬으면, 해당 로봇만 그냥 moveFlag를 0으로 바꿔주면 됨
                                                        if(stopCheck == 1){ //이미 stopOtherRobots함수를 통해 다른 로봇들을 stop시켜줬으면, 다시 함수 호출 안함
                                                                robots[i].moveFlag = 0;
                                                        }
                                                        else{ //처음이면 sotpOtherRobots함수 호출하고 stopCheck를 1로 바꿔줌
                                                                stopOtherRobots(robots, numOfRobot, i, initRound);
                                                                stopCheck = 1;
                                                        }
                                                }
                                                else{ //물건을 들고 있지 않으면 -> 올라갈때/내려갈때 다른 로봇들 대기 필요!!@@@
                                                        if(box_from_robot[i].msg.row+1 == payload_coordinate[destination[i]-1].row && box_from_robot[i].msg.col == payload_coordinate[destination[i]-1].col){ //아래에 물건이 있으면 아래로 이동
                                                                msg.cmd = p_down; //물건이 아래 있으면, 가지러 가기
                                                                // printf("...[%d]로봇이 이제 [%d]를 가지러 갑니다...\n", i+1, destination[i]);
                                                                
                                                                if(stopCheck == 1){ //이미 stopOtherRobots함수를 통해 다른 로봇들을 stop시켜줬으면, 다시 함수 호출 안함
                                                                        robots[i].moveFlag = 0;
                                                                }
                                                                else{ //처음이면 sotpOtherRobots함수 호출하고 stopCheck를 1로 바꿔줌
                                                                        stopOtherRobots(robots, numOfRobot, i, initRound);
                                                                        stopCheck = 1;
                                                                }

                                                        }
                                                        else if(box_from_robot[i].msg.row-1 == payload_coordinate[destination[i]-1].row && box_from_robot[i].msg.col == payload_coordinate[destination[i]-1].col){ //위에 물건이 있으면 위로 이동
                                                                msg.cmd = p_up; // 물건이 위에 있으면, 가지러 가기
                                                                // printf("...[%d]로봇이 이제 [%d]를 가지러 갑니다...\n", i+1, destination[i]);
                                                                
                                                                if(stopCheck == 1){ //이미 stopOtherRobots함수를 통해 다른 로봇들을 stop시켜줬으면, 다시 함수 호출 안함
                                                                        robots[i].moveFlag = 0;
                                                                }
                                                                else{
                                                                        stopOtherRobots(robots, numOfRobot, i, initRound);
                                                                        stopCheck = 1;
                                                                }
                                                        }
                                                        else{ // 둘 다 아니면 그냥 원래대로 이동
                                                                msg.cmd = step_cycle[msgFromRobot.cycle_step_index];
                                                        }
                                                }
                                                }
                                        }
                                        //2주기(step_cycle이 12~15) : (2, 2) 도착할 때까지 그냥 쭉 이동 
                                        else{
                                                msg.cmd = step_cycle[msgFromRobot.cycle_step_index];
                                        }

                                }
                                else{
                                        msg.cmd = 0; //아직 출발 안 했으면 아무 동작도 안 하게 함
                                }

                                box_from_center[i].msg = msg;  
                                box_from_center[i].dirtyBit = 1; // 메세지 전송했으니 1로 바꿔줌
                                // printf("robot %d에 보내는 cmd message : row %d | col %d | cp %d | rp %d | cmd %d\n", 
                                        // i + 1,
                                        // box_from_center[i].msg.row,
                                        // box_from_center[i].msg.col,
                                        // box_from_center[i].msg.current_payload,
                                        // box_from_center[i].msg.required_payload,
                                        // box_from_center[i].msg.cmd);
                        }

                        printf("\n");
                        print_blocked_threads(); //확인용

                        level++;
                        printf("@@@@@@level : %d@@@@@@@@@\n", level);
                        stopCheck = 0;

                        unblock_threads(); //모든 로봇이 메세지를 받았으면 unblock
                        block_thread(); //center는 다시 block

                }
                // 하역 준비 완료
                else{ 
                        printf("////////// 하역 준비 완료!!! ///////////\n");

                        for(int i=initRound*6; i<((initRound+1)*6) && i<numOfRobot ; i++){
                                struct message msgFromRobot = box_from_robot[i].msg;
                                struct message msg;

                                //<0> 도착한 로봇이면 그냥 무조건 제자리에서 대기하기
                                if(msgFromRobot.dest == map_draw_default[msgFromRobot.row][msgFromRobot.col]){ //dest와 현재 위치가 같으면
                                        msg.cmd = 0; //아무것도 안함
                                        // printf("robot %d : %c 도착 후 대기 중\n", i+1, robots[i].dest);//아무것도 안함
                                        // 최종으로 메세지 보내기
                                        box_from_center[i].msg = msg;  
                                        box_from_center[i].dirtyBit = 1; // 메세지 전송했으니 1로 바꿔줌
                                        continue; //바로 다음 로봇으로 넘어가기
                                } 

                                // 1) (2,2)부터 차례대로 줄 서있는 경우 -> 해당 칸의 로봇은 하역 장소로 이동 시작, 나머지 로봇들은 대기
                                if(check_twotwo(robots, numOfRobot, initRound) == 1){

                                        // 이동할 로봇 case
                                        if(msgFromRobot.row==2 && msgFromRobot.col==2){ //현재 위치가 (2,2)인 로봇이면, 즉 하역할 차례가 된 로봇이면
                                                // 하역 장소를 기준으로 나누기
                                                if(msgFromRobot.dest == 'A'){
                                                        msg.cmd = p_up;
                                                        putting = 1; //이제 이동 시작
                                                }
                                                else if(msgFromRobot.dest == 'B'){
                                                        msg.cmd = p_left;
                                                        putting = 1; //이제 이동 시작
                                                }
                                                else if(msgFromRobot.dest == 'C'){
                                                        msg.cmd = p_down;
                                                        putting = 1; //이제 이동 시작
                                                }
                                        }
                                        // 나머지 로봇들 case(대기)
                                        else{
                                                msg.cmd = 0; //아무것도 안함
                                        }
                                }
                                // 2) (2,2) 비어있는 경우
                                else if(check_twotwo(robots, numOfRobot, initRound) == 0){
                                        // 2-1)로봇이 하역 장소로 이동 중인 경우 -> 계속 이동해서 완료시켜야함
                                        if(putting == 1){
                                                //이동할 로봇 case
                                                if(msgFromRobot.row==1 && msgFromRobot.col==2){ //A로 이동중 
                                                        msg.cmd = p_up;
                                                        putCount++; //이제 이렇게 하면 다음 로봇이 첫 번째(맨 앞에 있는) 로봇이 되게 해야함
                                                        putting = 0; //이제 이동 완료
                                                        countInGroup++;
                                                }
                                                else if(msgFromRobot.row==2 && msgFromRobot.col==1){ //B로 이동중 
                                                        msg.cmd = p_left;
                                                        putCount++;
                                                        putting = 0; //이제 이동 완료
                                                        countInGroup++;
                                                }
                                                else if(msgFromRobot.row==3 && msgFromRobot.col==2){ //C로 이동중 - 1
                                                        msg.cmd = p_down;
                                                }
                                                else if(msgFromRobot.row==4 && msgFromRobot.col==2){ //C로 이동중 - 2
                                                        msg.cmd = p_down;
                                                        putCount++;
                                                        putting = 0; //이제 이동 완료
                                                        countInGroup++;
                                                }
                                                // 뒤쪽에 대기하고 있는 나머지 로봇 case
                                                else{ 
                                                        msg.cmd = 0; //아무것도 안함
                                                }
                                        }
                                        // 2-2) 로봇이 하역 장소로 이동 중이 아닌 경우((2,3)부터 차례대로 줄 서있는 경우) -> cycle_step대로 땡겨줘야함
                                        else{
                                                //이동할 로봇 case -> all
                                                msg.cmd = step_cycle[msgFromRobot.cycle_step_index];
                                        }

                                }
\
                                // 최종으로 메세지 보내기
                                box_from_center[i].msg = msg;  
                                box_from_center[i].dirtyBit = 1; // 메세지 전송했으니 1로 바꿔줌
                        }

                        level++;
                        stopCheck = 0;
                        printf("###### 지금까지 하역한 개수 : %d개\n", putCount);
                        printf("이번 턴에만만 하역한 개수 : %d개\n", countInGroup);

                        // 3)
                        unblock_threads();
                        block_thread();
                }

        }

}

void robot_thread(void *aux)
{
        thread_sleep(100);

        int idx = *((int *)aux);
        int test = 0;

        while (1)
        {
                // thread_sleep(idx*100); // 이렇게 하는 이유는??->로봇 스레드가 텀 두고 순서대로 실행되게 하려고
                // printf("thread %d : 실행 횟수 %d회\n", idx, (test++)+1);

                // 구현 -> 메세지 받기
                // printf("/////// robot에서 recieve message ///////\n");
                struct message msg = box_from_center[idx - 1].msg;
                // printf("robot %d에서 받은 message : row %d | col %d | cp %d | rp %d | cmd %d\n", idx, msg.row, msg.col, msg.current_payload, msg.required_payload, msg.cmd);

                // printf("1) robot %d의 이동 전 위치 : (%d, %d)\n", idx, robots[idx - 1].row, robots[idx - 1].col);
                // printf("1) robot %d의 이동 전 위치 : (%d, %d)\n", idx, msg.row, msg.col);

                

                // 구현 -> 메세지로 받은 cmd대로 robot 이동!!!!!!!!!!!!$$$@$@$$@@$@$@
                //맨 처음에는 하나의 로봇만 순서대로 나와야함. 아니면 전체가 한꺼번에 같이 움직이게 됨
                //가장 먼저 moveFlag부터 확인!!
                if(robots[idx-1].moveFlag == 1){
                        //아무것도 안함
                        printf("로봇 %d이 움직이지 않습니다.\n", idx);
                        //다시 풀어주기
                        robots[idx-1].moveFlag = 0;
                }
                else{
                        if (msg.cmd == up)
                        {
                                robots[idx - 1].row -= 1;
                                robots[idx - 1].cycle_step_index++;
                        }
                        else if (msg.cmd == down)
                        {
                                robots[idx - 1].row += 1;
                                robots[idx - 1].cycle_step_index++;

                        }
                        else if (msg.cmd == right)
                        {
                                robots[idx - 1].col += 1;
                                robots[idx - 1].cycle_step_index++;

                        }
                        else if (msg.cmd == left)
                        {
                                robots[idx - 1].col -= 1;
                                robots[idx - 1].cycle_step_index++;

                        }
                        else if(msg.cmd == p_up){
                                robots[idx - 1].row -= 1;
                        }
                        else if(msg.cmd == p_down){
                                robots[idx - 1].row += 1;
                        }
                        else if(msg.cmd == p_left){
                                robots[idx - 1].col -= 1;
                        }
                        else if(msg.cmd == 0){
                                //아무 동작도 안 하게 함
                        }
                }
                
                // 물건 적재 표시
                if(robots[idx - 1].row==payload_coordinate[destination[idx-1]-1].row && 
                        robots[idx - 1].col==payload_coordinate[destination[idx-1]-1].col){ // 현재 위치와 물건 목적지가 같으면
                        robots[idx - 1].current_payload = destination[idx - 1]; //robot변수의 current_payload에 물건 번호 저장하여 print_map을 할 때 로봇 옆에 출력되도록
                        // printf("로봇 %d이 %d번 물건을 적재했습니다.\n", idx, destination[idx-1]);
                }

                // 구현-> 현재 위치 메세지 보내기
                msg.row = robots[idx - 1].row;
                msg.col = robots[idx - 1].col;
                msg.current_payload = robots[idx - 1].current_payload;
                msg.required_payload = robots[idx - 1].required_payload;
                msg.cycle_step_index = robots[idx - 1].cycle_step_index;
                msg.payloadFlag = robots[idx - 1].payloadFlag;
                msg.moveFlag = robots[idx - 1].moveFlag;
                msg.dest = robots[idx - 1].dest;
                msg.cmd = 0; // cmd는 0으로 초기화

                box_from_robot[idx - 1].msg = msg;   //통으로 넣어줌 
                box_from_robot[idx - 1].dirtyBit = 1; // 메세지 전송했으니 1로 바꿔줌

                // printf("2) robot %d의 이동 후 위치 : (%d, %d)\n", idx, robots[idx - 1].row, robots[idx - 1].col); //확인용, 원래는 이렇게 직접 접근하면 안 됨

                count++;
                if (count == numOfRobot)
                {
                        // center만 unblock
                        // printf("\n@@@@@@unblock할거다!!!@@@@@\n");
                        struct thread *t = list_entry(list_pop_front(&blocked_threads), struct thread, elem);
                        count = 0;
                        thread_unblock(t);
                }
                block_thread();
                // thread_sleep(100*idx);
        }
}

// entry point of simulator
void run_automated_warehouse(char **argv)
{
        init_automated_warehouse(argv); // do not remove this

        printf("[[[ implement automated warehouse! ]]]\n");
        // argv[1] - 로봇 개수, argv[2] - 하역장소 쌍 리스트
        
        // 인수 설정
        numOfRobot = atoi(argv[1]);
        char *pairs = argv[2];

        //setting destination
        destination = malloc(sizeof(int) * numOfRobot);

        // 로봇 세팅
        robots = malloc(sizeof(struct robot) * numOfRobot);
        for (int i = 0; i < numOfRobot; i++)
        {
                int num;
                char alpha;

                num = pairs[3 * i] - '0';
                alpha = pairs[3 * i + 1];
                // printf("%d번째 로봇 -> num: %d, alpha: %c\n", i, num, alpha);

                char *robot_name; // Assume the maximum length of the name is 5
                robot_name = malloc(sizeof(char) * 5);
                snprintf(robot_name, sizeof(robot_name), "R%d", i + 1);
                setRobot(&robots[i], robot_name, ROW_W, COL_W, num, 0, alpha); //(5,5), 즉 W에 로봇을 세팅
                destination[i] = num; //전역변수 물건 번호 세팅
                // printf("<<로봇 세팅 확인>>\n");
                // printf("로봇 이름: %s, row: %d, col: %d, required_payload: %d, current_payload: %d, dest: %c\n", robots[i].name, robots[i].row, robots[i].col, robots[i].required_payload, robots[i].current_payload, robots[i].dest);
        }

        /*setting message box*/
        box_from_center = malloc(sizeof(struct message_box) * numOfRobot); /**/
        box_from_robot = malloc(sizeof(struct message_box) * numOfRobot);  /**/
        for (int i = 0; i < numOfRobot; i++)
        {
                box_from_center[i].dirtyBit = 0;
                box_from_robot[i].dirtyBit = 0;
        }

        // example of create thread
        tid_t *threads = malloc(sizeof(tid_t) * numOfRobot + 1);

        int *idxs = malloc(sizeof(int) * numOfRobot);
        threads[0] = thread_create("CNT", 0, &cnt_thread, NULL); // center thread(thread 1)
        for (int i = 0; i < numOfRobot; i++)
        {
                idxs[i] = i + 1;
                char robot_name[5]; // Assume the maximum length of the name is 5
                snprintf(robot_name, sizeof(robot_name), "R%d", i + 1);
                threads[i + 1] = thread_create(robot_name, 0, &robot_thread, &idxs[i]); // robot threads(thread 2, 3, 4)
        }
        
        totalRound = (numOfRobot/6) + (numOfRobot%6 != 0); //전체 반복 횟수
}