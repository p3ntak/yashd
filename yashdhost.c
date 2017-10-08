/*
NAME:        TCPServer
SYNOPSIS:    TCPServer [port]

DESCRIPTION:  The program creates a TCP socket in the inet
              listen for connections from TCPClients,
              accept clients into private sockets, and
              fork an echo process to ``serve'' the client.
              If [port] is not specified, the program uses any available port.

*/
#include <stdio.h>
/* socket(), bind(), recv, send */
#include <sys/types.h>
#include <sys/socket.h> /* sockaddr_in */
#include <netinet/in.h> /* inet_addr() */
#include <arpa/inet.h>
#include <netdb.h> /* struct hostent */
#include <string.h> /* memset() */
#include <unistd.h> /* close() */
#include <stdlib.h> /* exit() */
#include "threads.h"
#include "daemon.h"

#define MAXHOSTNAME 80
void reusePort(int sock);
void *EchoServe(void *arg);

// TODO: put main in a main loop so we can exit main without killing the yashd


int main(int argc, char **argv ) {
    int   sd, psd;
    struct   sockaddr_in server;
    struct  hostent *hp, *gethostbyname();
    struct  servent *sp;
    struct sockaddr_in from;
    int fromlen;
    int length;
    char ThisHost[80];
    int pn;
    uint16_t server_port = 3826;

    if (argc > 1)
        strncpy(u_server_path, argv[1], PATHMAX); /* use argv[1] */
    strncat(u_server_path, "/", PATHMAX-strlen(u_server_path));
    strncat(u_server_path, argv[0], PATHMAX-strlen(u_server_path));
    strcpy(u_socket_path, u_server_path);
    strcpy(u_pid_path, u_server_path);
    strncat(u_pid_path, ".pid", PATHMAX-strlen(u_pid_path));
    strcpy(u_log_path, u_server_path);
    strncat(u_log_path, ".log", PATHMAX-strlen(u_log_path));

    daemon_init(u_server_path, 0); /* We stay in the u_server_path directory and file
                                    creation is not restricted. */

    unlink(u_socket_path); /* delete the socket if already existing */

    sp = getservbyname("echo", "tcp");
    /* get TCPServer1 Host information, NAME and INET ADDRESS */

    gethostname(ThisHost, MAXHOSTNAME);
    /* OR strcpy(ThisHost,"localhost"); */

    printf("----TCP/Server running at host NAME: %s\n", ThisHost);
    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
        fprintf(stderr, "Can't find host %s\n", argv[1]);
        exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));



    /** Construct name of socket */
    server.sin_family = AF_INET;
    /* OR server.sin_family = hp->h_addrtype; */

    server.sin_addr.s_addr = htonl(INADDR_ANY);
    pn = htons(server_port);
    server.sin_port =  (in_port_t) pn;
    /*OR    server.sin_port = sp->s_port; */

    /** Create socket on which to send  and receive */

    sd = socket (AF_INET,SOCK_STREAM,IPPROTO_TCP);
    /* OR sd = socket (hp->h_addrtype,SOCK_STREAM,0); */
    if (sd<0) {
        perror("opening stream socket");
        // TODO: if socket fails do not exit just return to main loop
        exit(-1);
    }
    /** this allow the server to re-start quickly instead of fully wait
	for TIME_WAIT which can be as large as 2 minutes */
    reusePort(sd);
    if ( bind( sd, (struct sockaddr *) &server, sizeof(server) ) < 0 ) {
        close(sd);
        perror("binding name to stream socket");
        exit(-1);
    }

    /** get port information and  prints it out */
    length = sizeof(server);
    if ( getsockname (sd, (struct sockaddr *)&server,&length) ) {
        perror("getting socket name");
        exit(0);
    }
    printf("Server Port is: %d\n", ntohs(server.sin_port));

    /** accept TCP connections from clients and fork a process to serve each */
    listen(sd,4);
    fromlen = sizeof(from);
    for(;;){
        pthread_t thr;
        int i, rc;
//     create a thread_data_t argument array
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
        thread_data_t thr_data;
        thr_data.from = from;
        thr_data.psd = psd;
        if ((rc = pthread_create(&thr, NULL, EchoServe, &thr_data))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            close(psd);
            return EXIT_FAILURE;
        }
//        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
//        childpid = fork();
//        if ( childpid == 0) {
//            close (sd);
//            EchoServe(psd, from);
//        }
//        else{
//            printf("My new child pid is %d\n", childpid);
//            close(psd);
//        }
    }
}

//void EchoServe(int psd, struct sockaddr_in from) {
void *EchoServe(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int psd = data->psd;
    struct sockaddr_in from = data->from;
    char buf[512];
    int rc;
    struct  hostent *hp, *gethostbyname();

    printf("Serving %s:%d\n", inet_ntoa(from.sin_addr),
           ntohs(from.sin_port));
    if ((hp = gethostbyaddr((char *)&from.sin_addr.s_addr,
                            sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
    else
        printf("(Name is : %s)\n", hp->h_name);

    /**  get data from  clients and send it back */
    for(;;){
        printf("\n...server is waiting...\n");
        if( (rc=recv(psd, buf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
            buf[rc]='\0';
            printf("Received: %s\n", buf);
            printf("From TCP/Client: %s:%d\n", inet_ntoa(from.sin_addr),
                   ntohs(from.sin_port));
            printf("(Name is : %s)\n", hp->h_name);
            if (send(psd, buf, rc, 0) <0 )
                perror("sending stream message");
        }
        else {
            printf("TCP/Client: %s:%d\n", inet_ntoa(from.sin_addr),
                   ntohs(from.sin_port));
            printf("(Name is : %s)\n", hp->h_name);
            printf("Disconnected..\n");
            close (psd);
            exit(0);
        }
    }
}
void reusePort(int s)
{
    int one=1;

    if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *) &one,sizeof(one)) == -1 )
    {
        printf("error in setsockopt,SO_REUSEPORT \n");
        exit(-1);
    }
}