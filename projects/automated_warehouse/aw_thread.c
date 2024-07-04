#include "projects/automated_warehouse/aw_thread.h"
#include <stdio.h>

//
// You need to read carefully thread/synch.h and thread/synch.c
// 
// In the code, a fucntion named "sema_down" implements blocking thread and 
// makes list of blocking thread
// 
// And a function named "sema_up" implements unblocing thread using blocking list
//
// You must implement blocking list using "blocking_threads" in this code.
// Then you can also implement unblocking thread.
//


struct list blocked_threads;

/**
 * A function unblocking all blocked threads in "blocked_threads" 
 * It must be called by robot threads
 */
void block_thread(){
    // printf("<< block_thread 호출 >>\n\n");
    enum intr_level old_level;
    old_level = intr_disable ();
    list_push_back(&blocked_threads, &thread_current()->elem);
    thread_block (); //thread를 block!!
    intr_set_level (old_level);
}

/**
 * A function unblocking all blocked threads in "blocked_threads" 
 * It must be called by central control thread
 */
void unblock_threads(){
    // printf("<< unblock_thread 호출 >>\n");
    while(!list_empty(&blocked_threads)){
        struct thread* t = list_entry(list_pop_front(&blocked_threads), struct thread, elem);
        thread_unblock(t);
    }
}

//blocked list 출력
void print_blocked_threads(){
    printf("blocked thread name: ");
    struct list_elem *e;
    for (e = list_begin(&blocked_threads); e != list_end(&blocked_threads); e = list_next(e)){
        struct thread *t = list_entry(e, struct thread, elem);
        printf("%s ", t->name);
    }
    printf("\n");
}