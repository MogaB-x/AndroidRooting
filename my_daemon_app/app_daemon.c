#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
extern char** environ;

int main(int argc, char** argv) {
    char* daemon_cmd = "/system/bin/mydaemon";
    char* mysu_cmd = "/system/bin/mysu";
    char* cmd = "/system/bin/app_process64_original";

    printf("Launching mydaemon...\n");
    if (fork() == 0) {
        execl(daemon_cmd, daemon_cmd, NULL);
        perror("Failed to launch mydaemon");
        exit(EXIT_FAILURE);
    }

    sleep(1); 

    printf("Launching mysu...\n");
    if (fork() == 0) {
        execl(mysu_cmd, mysu_cmd, NULL);
        perror("Failed to launch mysu");
        exit(EXIT_FAILURE);
    }

    sleep(1);

    //app_process64_original
    printf("Launching app_process64_original...\n");
    if (execve(cmd, argv, environ) == -1) {
        perror("Failed to launch app_process64_original");
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}
