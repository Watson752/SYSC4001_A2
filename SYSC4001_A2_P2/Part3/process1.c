/*
 * SYSC4001 Assignment2
 * Part II
 * StudentName: Emeka Anonyei 101209704
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    

    pid_t pid = fork();
    if (pid < 0) {
        perror("error-fork");
        return 1;
    }

    if (pid == 0) {
        
        execlp("./process2", "process2", NULL);
        perror("error-exec");
        return(1);
    }
    
        int count = 0;
        int status;
        pid_t waitFlag;
        
        while (1) {
         
            waitFlag = waitpid(pid, &status, WNOHANG);  

            if (waitFlag == pid) {  
                printf("Process 2 finished, Process 1 exiting.\n");
                break;
            }

            if (count !=0 && count % 3 == 0)
            printf("PID %d  –  Cycle number: %d (multiple of 3)\n", (int)getpid(), count);
        else
            printf("PID %d –  Cycle number: %d\n", (int)getpid(), count);
        count++;
        sleep(1);
    }
    return 0;
}

