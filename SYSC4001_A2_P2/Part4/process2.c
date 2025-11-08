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

typedef struct {
    int multiple;
    int count;
} Shared;

int main(void)
{
    key_t key = 1234;
    int shmid = shmget(key, sizeof(Shared), 0666);
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    Shared *sh = (Shared *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("shmat");
        return 1;
    }

    while (sh->count <= 100) {
        sleep(1);
    }

  
    while (sh->count <= 500) {
        if (sh->count != 0 && (sh->count % sh->multiple) == 0) {
            printf("PID %d - Cycle number: %d (multiple of %d)\n", (int)getpid(), sh->count, sh->multiple);
        } else {
            printf("PID %d - Cycle number: %d\n", (int)getpid(), sh->count);
        }
        sleep(1);
    }

    shmdt(sh);
    return 0;
}

