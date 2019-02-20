#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


int main(int argc, char **argv)
{
  if(argc != 2) {
    return -1;
  }
  pid_t pid = atoi(argv[1]);
  if(kill(pid, SIGUSR1) == -1){
    printf("Process ID not found\n");
    return -1;
  }
  return 0;
}
