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
#include <string.h>
#include <stdlib.h>
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
#include "my_semaphore.h"
#include "yash_program.c"

#define MAXHOSTNAME 80
void reusePort(int sock);
void *EchoServe(void *arg);
thread_data_t thread_arr[100];
pthread_t thr[100];
static pthread_key_t psd_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static void make_key(){
    (void) pthread_key_create(&psd_key, NULL);
}

// TODO: put main in a main loop so we can exit main without killing the yashd
int ret;
sem_t mysem;

int main(int argc, char **argv ) {
    int   sd, psd;
    struct   sockaddr_in server;
    struct  hostent *hp, *gethostbyname();
    struct  servent *sp;
    int fromlen;
    int length;
    char ThisHost[80];
    int pn;
    uint16_t server_port = 3826;

    ret = sem_init(&mysem, 0, 1);
    if (ret != 0) {
        /* error. errno has been set */
        perror("Unable to initialize the semaphore");
        abort();
    }

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

    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
        fprintf(stderr, "Can't find host %s\n", argv[1]);
        exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);



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

    /** accept TCP connections from clients and fork a process to serve each */
    listen(sd,4);
//    for(int tid=0; tid<100; tid++){
    (void) pthread_once(&key_once, make_key);
    int tid = 0;
    for(;;){
        struct sockaddr_in from;
        fromlen = sizeof(from);
        ssize_t rc;
/**     create a thread_data_t argument array */
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
//        thread_data_t thread_data;
//        thread_data.psd = psd;
//        thread_data.from = from;
        thread_arr[tid].from = &from;
        thread_arr[tid].psd = psd;
        thread_arr[tid].tid = tid;
        if ((rc = pthread_create(&thr[tid], NULL, EchoServe, &thread_arr[tid].tid))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", (int) rc);
            close(psd);
            return EXIT_FAILURE;
        }
    }
}

// parent should do non blocking waitpid followed up a non blocking receive so a CTL signal can be received
void *EchoServe(void *arg) {
//    thread_data_t *data = (thread_data_t *)arg;
    int *tid_temp = (int*) arg;
    int tid = *tid_temp;
    struct  hostent *hp, *gethostbyname();
    if ((hp = gethostbyaddr((char *)&thread_arr[tid].from->sin_addr,
                            sizeof(thread_arr[tid].from->sin_addr),AF_INET)) == NULL)
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(thread_arr[tid].from->sin_addr));
//    int psd = data->psd;
//    int my_tid = data->tid;
//    void *thread_psd;

//    struct sockaddr_in *from = data->from;
    if(pipe(thread_arr[tid].pthread_pip_fd) ==-1){
        perror("pthread pipe\n");
        exit(-1);
    }
    dup2(thread_arr[tid].psd, STDOUT_FILENO);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    FILE * client = fdopen(thread_arr[tid].psd, "w");

    int new_pid = fork();

    if(new_pid == 0) {
        close(thread_arr[tid].pthread_pip_fd[1]);
        dup2(thread_arr[tid].pthread_pip_fd[0], STDIN_FILENO);
        thread_arr[tid].psd = fileno(client);
        yash_prog_loop(thread_arr[tid].tid);
    }
    else if (new_pid > 0){
        close(thread_arr[tid].pthread_pip_fd[0]);
        char *prompt;
        prompt = strdup("# ");
        rc = strlen(prompt);
        if (send(thread_arr[tid].psd, prompt, (size_t) rc, 0) < 0)
            perror("sending stream message");
        for(;;){
            char buf[512];
            char **args;
            char *buf_copy;
            cleanup(buf);
            if( (rc=recv(thread_arr[tid].psd, buf, sizeof(buf), 0)) < 0){
                perror("receiving stream  message");
                exit(-1);
            }
            if (rc > 0){
                buf[rc]='\0';
                buf_copy = strdup(buf);
                args = parseLine(buf_copy);
                if (strcmp(args[0], "CTL") == 0) {
                    if (strcmp(args[1], "c") == 0) kill(new_pid, SIGINT);
                    if (strcmp(args[1], "z") == 0) kill(new_pid, SIGTSTP);
                }
                if (strcmp(args[0], "CMD") == 0) {
                    printf("%s",buf);
                    fflush(stdout);
                }
                write_to_log(buf, (size_t) rc, inet_ntoa(thread_arr[tid].from->sin_addr), ntohs(thread_arr[tid].from->sin_port));
            }
        }
    }
    /**  get data from  clients and send it back */





//        else {
//            printf("exiting\n");
//            close (psd);
//            pthread_exit(NULL);
//        }

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
