/*
 * Name1: Miguel Mendoza
 * CS login: miguelmcell
 *
 * Name2: Gage Courtaway
 * CS login: gtc375
 *
 * 9/11 - 9/20
 *
 * msh - A mini shell program with job control
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "msh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {
    	/* Read command line */
    	if (emit_prompt) {
    	    printf("%s", prompt);
    	    fflush(stdout);
    	}
    	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
    	    app_error("fgets error");
      /* implemented the SIGINT signal handler */
      if(Signal(SIGINT, sigint_handler) == SIG_ERR) {
        unix_error("SIGINT signal error");
      }
      /* implemented the SIGSTP signal handler */
      if(Signal(SIGTSTP, sigtstp_handler) == SIG_ERR) {
        unix_error("SIGTSTP signal error");
      }
      /* implemented the SIGCHLD signal handler */
      if(Signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        unix_error("SIGCHLD signal error");
      }
    	if (feof(stdin)) { /* End of file (ctrl-d) */
    	    fflush(stdout);
    	    exit(0);
    	}

    	/* Evaluate the command line */
    	eval(cmdline);
    	fflush(stdout);
    	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline)
{
    char *argv[MAXARGS];  /* argument list */
    char buf[MAXLINE];    /* holds modified command line */
    int bg; /* flag that determines if the given process is a background job */
    pid_t pid;            /* pid for the parent and child process */
    strcpy(buf, cmdline); /* copies the given command line to the buffer */
    /* parseline returns 1 if the given process is going to run in the
       background, otherwise returns 0 */
    bg = parseline(buf, argv);
    //sig mask and stop signals mostly done by miguel
    sigset_t mask, prev;  /* instantiates masks used to block given signals */
    sigemptyset(&prev);   /* initializes prev mask */
    sigemptyset(&mask);   /* initializes mask called mask */
    /* adds the given signals to mask */
    sigaddset(&mask, SIGCHLD | SIGINT | SIGTSTP);

    /* shell will ignore empty lines */
    if(argv[0] == NULL)
      return;

    if(!builtin_cmd(argv)) {//taken from part 1, implementation mostly gage
      /* uses sigprocmask to block SIGCHLD, SIGINT, and SIGSTP */
      sigprocmask(SIG_BLOCK, &mask, &prev);

      if((pid = fork()) == 0){ /* will only run in the child process */
        /* puts child in a new process group in order to prevent multiple
           processes in the foreground process group */
        setpgid(0, 0);
        /* unblocks signals before executing execv, so that none of the
           children have any blocked signals */
        sigprocmask(SIG_SETMASK, &prev, NULL);
        /* checks if the given executable path is valid */
        if(execv(argv[0], argv) < 0){
          printf("%s: Command not found\n", argv[0]);
          exit(-1);
        }
      }
      else{ /* will only run in the parent process *///miguel
        /* if the given process is a background job, add it to the job list
           and unblock any signals, otherwise if its a foreground job also add
           it to the job list but block until the job finishes */
        if(bg){
          addjob(jobs,pid,BG,cmdline);
          printf("[%d] (%d) %s", maxjid(jobs),pid,cmdline);
          sigprocmask(SIG_SETMASK, &prev, NULL);
        }
        else {
          addjob(jobs,pid,FG,cmdline);
          sigprocmask(SIG_SETMASK, &prev, NULL);
          waitfg(pid);
        }
      }
    }
    return;
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 * Return 1 if a builtin command was executed; return 0
 * if the argument pabgssed in is *not* a builtin command.
 */
int builtin_cmd(char **argv)//joint
{
    /* function will return 1 if the user typed a valid builtin command,
       otherwise it will return 0, quit will not return but rather terminate
       the shell */
    if(!strcmp(argv[0], "quit"))
      exit(0);
    else if(!strcmp(argv[0], "jobs")) {
      listjobs(jobs);
      return 1;
    }
    else if(!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg")){
      do_bgfg(argv);
      return 1;
    }
    return 0;
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) //Most implementation Miguel, Gage's code had to be deleted bc scrub
{
    /* first checks if the command contains a secod argument */
    if(argv[1] == NULL){
      printf("%s command requires PID or %%jobid argument\n",argv[0]);
      return;
    }
    /* the function checks if the given ID is a JID or PID */
    if(argv[1][0] == '%') {
      int jidNum;
      char *jidString = argv[1]+1;

      /* checks if its argument is a valid number */
      if( (jidNum = atoi(jidString)) == 0){
        printf("%s: argument must be a PID or %%jobid\n",argv[0]);
        return;
      }
      /* checks if there is a job with that JID */
      if(getjobjid(jobs, jidNum) == NULL){
        printf("%%%d: No such job\n", jidNum);
        return;
      }
      /* restart the process by sending it a SIGCONT signal, then
         change the job's state to the corresponding command */
      kill(-(getjobjid(jobs,jidNum) -> pid), SIGCONT);
      if(!strcmp(argv[0], "fg")){
        getjobjid(jobs, jidNum) -> state = FG;
        waitfg(getjobjid(jobs,jidNum) -> pid);
      }
      else if(!strcmp(argv[0], "bg")){
        getjobjid(jobs,jidNum) -> state = BG;
        printf("[%d] (%d) %s", jidNum, getjobjid(jobs,jidNum) -> pid, getjobjid(jobs,jidNum) -> cmdline);
      }
    }
    else{
      pid_t pid;
      /* checks if the PID is a valid number */
      if((pid = atoi(argv[1])) == 0){
        printf("%s: argument must be a PID or %%jobid\n",argv[0]);
        return;
      }
      /* checks if the PID exists for an existing job */
      if(getjobpid(jobs, pid) == NULL){
        printf("(%d): No such process\n", pid);
        return;
      }
      /* restart the process by sending it a SIGCONT signal, then
         change the job's state to the corresponding command */
      kill(-pid, SIGCONT);
      if(!strcmp(argv[0], "fg")){
        getjobpid(jobs, pid) -> state = FG;
        waitfg(pid);
      }
      else if(!strcmp(argv[0], "bg")){
        getjobpid(jobs,pid) -> state = BG;
        printf("[%d] (%d) %s", pid2jid(jobs, pid), pid, getjobpid(jobs,pid) -> cmdline);
      }
    }
    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)//Miguel did this
{
    sigset_t mask, prev; /* instantiates the masks */

    sigemptyset(&mask); /* initializes mask */
    sigemptyset(&prev); /* initializes mask */
    /* adds the given signals to mask */
    sigaddset(&mask, SIGINT | SIGCHLD | SIGSTOP );
    sigprocmask(SIG_BLOCK, &mask, &prev); /* Block  so no change between check and process */
    /* continues checking until the foregound process has finished */
    while(pid == fgpid(jobs)){
      /* sigsuspend blocks the process until it recieves a valid signal */
      sigsuspend(&prev);
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);/* Unblocks signals */
    return;
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig)//final implementation done by Gage
{
    /* handles when a child is either interupted or stopped,
    will display a status when a job has been changed */
    int status;
    char buffer[40];
    const int STDOUT = 1;
    ssize_t bytes;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
      //sigprocmask(SIG_BLOCK, &maskAll, &prev); /* Block  SIGCHILD */
      if(WIFSIGNALED(status)){
		    sprintf(buffer, "Job [%d] (%d) terminated by signal 2\n", pid2jid(jobs, pid), pid);
        bytes = write(STDOUT, buffer, 40);
        if(bytes > 40)
          exit(-999);
      }
      else if(WIFSTOPPED(status))
		  {
        sprintf(buffer, "Job [%d] (%d) stopped by signal 20\n", pid2jid(jobs, pid), pid);
        bytes = write(STDOUT, buffer, 40);
        if(bytes > 40)
          exit(-999);
			  return;
      }
      deletejob(jobs, pid);
      //sigprocmask(SIG_SETMASK, &prev, NULL);/* Unblock SIGCHILD */
    }
    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig)//Done by Gage
{
  /* gets the pid of the foreground process and sends a SIGINT to it */
    pid_t pid = fgpid(jobs);
    if(pid)
      kill(-pid, 2);
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig)//Done by Gage
{
  /* gets the foreground process if there is one and stops the process */
  pid_t pid = fgpid(jobs);
    if(pid) {
      kill(-pid, 20);
      getjobpid(jobs,pid)->state = ST;
    }
    return;
}

/*********************
 * End signal handlers
 *********************/



/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig)
{
    ssize_t bytes;
    const int STDOUT = 1;
    bytes = write(STDOUT, "Terminating after receipt of SIGQUIT signal\n", 45);
    if(bytes != 45)
       exit(-999);
    exit(1);
}
