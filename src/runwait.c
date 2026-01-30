#include "common.h"
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void usage(const char *a){
    fprintf(stderr,"Usage: %s <cmd> [args]\n",a); exit(1);
}
static double d(struct timespec a, struct timespec b){
    return (b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;
}

 int main(int c,char**v){

    // fork into parent and child
    // child runs the command, parent waits and measures time
    // child uses execvp to run the command
    // parent uses waitpid to wait for the child
    // Print child PID, exit code or signal, and elapsed wall-clock time 
    // Use CLOCK_MONOTONIC

    //if not enough args, usage error
    if(c<2) 
        usage(v[0]);

    struct timespec startTime, endTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    pid_t pid = fork();
    if (pid < 0) {
        printf("forking error\n");
        exit(1);
    }
    else if (pid == 0) {
        // Child process
        execvp(v[1], &v[1]);
        printf("execvp error\n");
        exit(1);
    } 
    else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        double elapsed = d(startTime, endTime);
        printf("Child PID: %d\n", pid);
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            printf("Exit code/signal: %d\n", WEXITSTATUS(status));
        }
        printf("Elapsed time: %.2f seconds\n", elapsed);
    }


 return 0;
}
