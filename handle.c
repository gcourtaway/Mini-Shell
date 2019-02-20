#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"


/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 *///Writen by Gage
void sigint_handler(int sig) {//using DR. Normans write code again for this part
  ssize_t bytes;
  const int STDOUT = 1;
  bytes = write(STDOUT, "Nice try.\n", 10);
  if(bytes != 10)
    exit(-999);
  //this is from the handout from project 0 in order to print safely
}
void sigusr1_handler(int sig) {//Writen by miguel
  ssize_t bytes;
  const int STDOUT = 1;
  bytes = write(STDOUT, "exiting\n", 10);
  if(bytes != 10)
    exit(-999);
  exit(1);
  //this is from the handout from project 0 in order to print safely
}


int main(int argc, char **argv)
{
  char buf[MAXLINE];//buffer
  pid_t pid = getpid();
  sprintf(buf,"%d", pid);
  puts(buf);
  while(1){
    /* this code is on pg 763 of
    from Randal E. Bryant and David R. O'Hallaron's Computer Systems: A Programmers Perspective.
    but using wrapper signal instead. This protects our signals*/
    if(Signal(SIGINT, sigint_handler) == SIG_ERR) {//signal if statements found by Gage in book writen by miguel
      unix_error("SIGINT signal error");
    }
    if(Signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
      unix_error("SIGUSR1 signal error");
    }
    nanosleep((const struct timespec[]){{1, 0}}, NULL);//nonosleep and sprint found by miguel
    sprintf(buf,"waiting!\n");
    puts(buf);
  }
  // pause();
  return 0;
}
