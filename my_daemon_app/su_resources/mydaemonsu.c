/* 
 * File:   mydaemonsu.c
 * Author: Zhuo Zhang, Syracuse University
 *         zzhan38@syr.edu
 *
 */
 

/* Version 1.0 - First Release
 * This project is a server
 * It runs under root privilege and wait for client's connect
 * After connection, it launch a terminal for the client and redirect
 * the terminal's input and output to the client
 */
 

 /* This project is based on open source su project
 * Source: https://github.com/koush/Superuser
 * Original License:
 *   ** Copyright 2010, Adam Shanks (@ChainsDD)
 *   ** Copyright 2008, Zinx Verituse (@zinxv)
 *   **
 *   ** Licensed under the Apache License, Version 2.0 (the "License");
 *   ** you may not use this file except in compliance with the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>     //socket() bind() listen() accept() AF_UNIX
#include <fcntl.h>          //fcntl()
#include <string.h>         //strerror()
#include <errno.h>          //errno
#include <sys/un.h>         //struct sockaddr_un
#include <sys/stat.h>       //umask() mkdir()
#include <stdbool.h>        //bool true false

#include "../socket_util/socket_util.h"
#include "../server_loc.h"

#define ERRMSG(msg) fprintf(stderr, "%s", msg)

#define DEFAULT_SHELL "/system/bin/sh"

#define SHELL_ENV "SHELL=/system/bin/sh"
#define PATH_ENV "PATH=/system/bin:/system/xbin"

#define APP_PROCESS "/system/bin/app_process_original"

extern char** environ;

//create a UNIX domain socket and return its file descriptor
int creat_socket() {
    int socket_fd;
    struct sockaddr_un sun;
    
    //open socket
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        ERRMSG("failed to open socket\n");
        exit(EXIT_FAILURE);
    }
    
    //set the socket file descriptor
    //with flag FD_CLOEXEC, socket_fd will stay valid through fork()
    //but will be destroyed by all exec family functions (e.g. execve())
    if (fcntl(socket_fd, F_SETFD, FD_CLOEXEC)) {
        ERRMSG("failed to fcntl\n");
        goto err;
    }
    
    //set struct sockaddr_un
    /*    
        struct sockaddr_un {
            sa_family_t sun_family;               //AF_UNIX
            char        sun_path[108];            //pathname
        };
    */
    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, SERVER_LOC, sizeof(sun.sun_path)); 
    
    //get rid of potential existing file due to previous error
    unlink(sun.sun_path);
    unlink(SERVER_DIR);
    
    //backup current umask
    //and change umask to allow all permissions
    int previous_umask = umask(0);
    
    //make new server path
    mkdir(SERVER_DIR, 0777);
    
    //bind socket
    if (bind(socket_fd, (struct sockaddr*)&sun, sizeof(sun)) < 0) {
        ERRMSG("failed to bind socket\n");
        goto err;
    }
    
    //restore umask
    umask(previous_umask);
    
    //start listening on the socket
    if (listen(socket_fd, 10) < 0) {
        ERRMSG("failed to listen\n");
        goto err;
    }
    
    return socket_fd;
    
err:    
    close(socket_fd);
    exit(EXIT_FAILURE);
}

//the code executed by the child process
//it launches default shell and link file descriptors passed from client side
int child_process(int socket, char** argv){
    //handshake
    handshake_server(socket);
    
    int client_in = recv_fd(socket);
    int client_out = recv_fd(socket);
    int client_err = recv_fd(socket);

    
    dup2(client_in, STDIN_FILENO);      //STDIN_FILENO = 0
    dup2(client_out, STDOUT_FILENO);    //STDOUT_FILENO = 1
    dup2(client_err, STDERR_FILENO);    //STDERR_FILENO = 2
    
    //change current directory
    chdir("/");

    char* env[] = {SHELL_ENV, PATH_ENV, NULL};
    char* shell[] = {DEFAULT_SHELL, NULL};

    execve(shell[0], shell, env);

    //expect no return from execve
    //only if execve fails
    ERRMSG("Failed on launching shell: ");
    ERRMSG(strerror(errno));
    ERRMSG("\n");
    
    close(socket);
    
    exit(EXIT_FAILURE);
}

//start the daemon and keep waiting for connections from client
void run_daemon( char** argv) {
    if (getuid() != 0) {
        ERRMSG("Daemon require root privilege\n");
        exit(EXIT_FAILURE);
    }
    
    //get a UNIX domain socket file descriptor
    int socket = creat_socket();
    
    //wait for connection
    //and handle connections
    int client;
    while ((client = accept(socket, NULL, NULL)) > 0) {
        if (0 == fork()) {
            close(socket);
            ERRMSG("Child process start handling the connection\n");
            exit(child_process(client,argv));
            child_process(client, argv);
        }
        else {
            close(client);
        }
    }
    
    //expect daemon never end execution
    //unless socket failed
    ERRMSG("Daemon quits: ");
    ERRMSG(strerror(errno));
    ERRMSG("\n");
    
    close(socket);
    close(client);
    
    exit(EXIT_FAILURE);
}

//try to connect to the daemon to determine whether it is running
bool detect_daemon() {

    struct sockaddr_un sun;
    
    //create socket fd
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        ERRMSG("failed to create socket fd\n");
        exit (EXIT_FAILURE);
    }
    
    //set socket fd
    if (fcntl(socket_fd, F_SETFD, FD_CLOEXEC)) {
        ERRMSG("failed on fcntl\n");
        exit (EXIT_FAILURE);
    }
    
    //set sun
    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, SERVER_LOC, sizeof(sun.sun_path));
    
    //connect to server
    //return false if connection failed (daemon is not running)
    if (0 != connect(socket_fd, (struct sockaddr*)&sun, sizeof(sun))) {
        return false;
    }
    
    //close the socket and return true if connection succeeded (daemon is running)
    close(socket_fd);
    return true;
}

int main(int argc, char** argv) {
    pid_t pid = fork();
    if (pid == 0) {
        //initialize the daemon if not running
        if (!detect_daemon())
            run_daemon(argv);
        }
    else {
        argv[0] = APP_PROCESS;
        execve(argv[0], argv, environ);
    }
}
