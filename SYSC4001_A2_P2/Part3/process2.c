/*
 * SYSC4001 Assignment2
 * Part II
 * StudentName: Emeka Anonyei 101209704
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    int count = 0;
    

    while (count > -500) {
         if (count !=0 && count % 3 == 0)
            printf("PID %d –  Cycle number: %d (multiple of 3)\n", (int)getpid(), count);
          
            
        else
            printf("PID %d –  Cycle number: %d\n", (int)getpid(), count);
            count--;
            sleep(1);
    }
    return 0;

}
