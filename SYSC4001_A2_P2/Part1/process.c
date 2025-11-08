/*
 * SYSC4001 Assignment2
 * Part II - Question 1
 * StudentName: Emeka Anonyei 101209704
 */


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid = fork();  

    if (pid < 0)
    {
        //fork failed
        perror("error-fork");
        return 1;
    }

   if (pid == 0)
    {
        
        int count = 0;
        while (1)
        {
            count++;
            printf("PID %d -Process 2 (Child) : %d\n", (int)getpid(), count);
            sleep(1);        
        }
    }
    else
    {

        int count = 0;
        while (1)
        {
            count++;
            printf("PID %d - Process 1 (Parent): %d\n", (int)getpid(), count);
            sleep(1);
        }
    }

    return 0;
}
 
