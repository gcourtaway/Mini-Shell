#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

//pid_t parentPID;
const int MAX = 13;

static void doFib(int n, int doPrint);


/*
 * unix_error - unix-style error routine.
 */
inline static void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}


int main(int argc, char **argv)
{
    int arg;
    int print=1;

    if(argc != 2){
        fprintf(stderr, "Usage: fib <num>\n");
        exit(-1);
    }

    arg = atoi(argv[1]);
    if(arg < 0 || arg > MAX){
        fprintf(stderr, "number must be between 0 and %d\n", MAX);
        exit(-1);
    }

    doFib(arg, print);

    return 0;
}

/*
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 */

static void doFib(int n, int doPrint)
{

  int ans1 = 0, ans2 = 0;
  int s1;//for status to wexitstatus
  int s2;
  pid_t pid1;//for double fork
  pid_t pid2;
  if(n <= 1) {//recursion base condition
    // printf("returning 0\n");
    exit(n);
  }
  else{
    pid1 = fork();
    if(pid1 == 0){//child 1
      // printf("New Child1 %d\n", n-1);
      doPrint = 0;
      doFib(n-1, doPrint);
      // exit(n-1);//3-1 = 2
    }//double child fork writen by Gage 
    pid2 = fork();
    if(pid2 == 0){//child 2
      doPrint = 0;
      // printf("New Child2 %d\n", n-2);
      doFib(n-2, doPrint);
      // exit(n-2);
    }
    else {//parent
      wait(&s1);//wait and exit miguel
      wait(&s2);
      ans1 = WEXITSTATUS(s1);
      ans2 = WEXITSTATUS(s2);
      // printf("i got back %d and %d when n was %d\n",ans1, ans2, n);
      int fibNum = ans1 + ans2;
      // printf("fibonacci(%d) = %d+%d = %d\n", n,ans1, ans2, fibNum);
      if(doPrint){
        printf("%d\n", fibNum);
      }
      // printf("%d\n", fibNum);
      exit(fibNum);
    }
  }

}
