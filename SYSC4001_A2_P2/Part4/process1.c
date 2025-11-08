/*
 * SYSC4001 Assignment2
 * Part II
 * StudentName: Emeka Anonyei 101209704
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

typedef struct {
    int multiple;   
    int count;     
} Shared;

int main(void)
{
    key_t key = 1234;
    int shmid = shmget(key, sizeof(Shared), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    
    Shared *sh = (Shared *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("shmat");
        return 1;
    }

    
    sh->multiple = 3;
    sh->count    = 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("error-fork");
        shmdt(sh);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    if (pid == 0) {
        
        execl("./process2", "process2", (char *)NULL);
        perror("error-exec");
        exit(1);
    }

    
    while (1) {
        if (sh->count > 500) {
            break;                  
        }

        if (sh->count != 0 && (sh->count % sh->multiple) == 0) {
            printf("PID %d -Cycle number: %d (multiple of %d)\n", (int)getpid(), sh->count, sh->multiple);
        } else 
            printf("PID %d - Cycle  number: %d\n",  (int)getpid(), sh->count);
        
	sh->count++;
        sleep(1);
    }

    int status;
    waitpid(pid, &status, 0);

    shmdt(sh);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

