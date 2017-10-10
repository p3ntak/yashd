// client requires host ip address and should connect to port 3826
// client should start by command yash <IP_Address_of_Server>

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
#include <signal.h>


#define MAXHOSTNAME 80
#define BUFSIZE 1024

static void client_sig_handler(int signo);

char buf[BUFSIZE];
char rbuf[BUFSIZE];
void GetUserInput();
void cleanup(char *buf);
int sigint_flag = 0;
int sigtstp_flag = 0;


int rc, cc;
int   sd;

int main(int argc, char **argv ) {
    int childpid;
    struct   sockaddr_in server;
    struct   sockaddr_in client;
    struct  hostent *hp, *gethostbyname();
    struct  servent *sp;
    struct sockaddr_in from;
    struct sockaddr_in addr;
    int fromlen;
    int length;
    char ThisHost[80];
    uint16_t server_port = 3826;

    sp = getservbyname("echo", "tcp");

    /** get TCPClient Host information, NAME and INET ADDRESS */

    gethostname(ThisHost, MAXHOSTNAME);
    /* OR strcpy(ThisHost,"localhost"); */

    printf("----TCP/Client running at host NAME: %s\n", ThisHost);
    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
        fprintf(stderr, "Can't find host %s\n", argv[1]);
        exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    printf("    (TCP/Client INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

    /** get TCPServer-ex2 Host information, NAME and INET ADDRESS */

    if (argc == 1){
        printf("Host address is required...exiting\n");
        exit(EXIT_FAILURE);
    }

    if  ( (hp = gethostbyname(argv[1])) == NULL ) {
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        if ((hp = gethostbyaddr((char *) &addr.sin_addr.s_addr,
                                sizeof(addr.sin_addr.s_addr),AF_INET)) == NULL) {
            fprintf(stderr, "Can't find host %s\n", argv[1]);
            exit(-1);
        }
    }
    printf("----TCP/Server running at host NAME: %s\n", hp->h_name);
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

    /* Construct name of socket to send to. */
    server.sin_family = AF_INET;
    /* OR server.sin_family = hp->h_addrtype; */

    server.sin_port = htons(server_port);
    /*OR    server.sin_port = sp->s_port; */

    /*   Create socket on which to send  and receive */

    sd = socket (AF_INET,SOCK_STREAM,0);
    /* sd = socket (hp->h_addrtype,SOCK_STREAM,0); */

    if (sd<0) {
        perror("opening stream socket");
        exit(-1);
    }

    /** Connect to TCPServer-ex2 */
    if ( connect(sd, (struct sockaddr *) &server, sizeof(server)) < 0 ) {
        close(sd);
        perror("connecting stream socket");
        exit(0);
    }
    fromlen = sizeof(from);
    if (getpeername(sd,(struct sockaddr *)&from,&fromlen)<0){
        perror("could't get peername\n");
        exit(1);
    }
    printf("Connected to TCPServer1: ");
    printf("%s:%d\n", inet_ntoa(from.sin_addr),
           ntohs(from.sin_port));
    if ((hp = gethostbyaddr((char *) &from.sin_addr.s_addr,
                            sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
    else
        printf("(Name is : %s)\n", hp->h_name);
    childpid = fork();
    if (childpid == 0) {
        GetUserInput();
    }

    /** get data from USER, send it SERVER,
      receive it from SERVER, display it back to USER  */

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    for(;;) {
        cleanup(rbuf);
        if( (rc=recv(sd, rbuf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
            rbuf[rc]='\0';
            printf("Received: %s\n", rbuf);
        }else {
            printf("Disconnected..\n");
            close (sd);
            exit(0);
        }

    }
}

void cleanup(char *buf)
{
    int i;
    for(i=0; i<BUFSIZE; i++) buf[i]='\0';
}

void GetUserInput()
{
    if(signal(SIGINT, client_sig_handler) == SIG_ERR)
        printf("signal(SIGINT)error");
    if(signal(SIGTSTP, client_sig_handler) == SIG_ERR)
        printf("signal(SIGTSTP)error");
    for(;;) {
//        printf("\nType anything followed by RETURN, or type CTRL-D to exit\n");
        cleanup(buf);
        if ((rc = read(0, buf, sizeof(buf))) >= 0 || sigint_flag || sigtstp_flag) {
            if (rc == 0 && sigint_flag == 0 && sigtstp_flag == 0) break;
            if (sigint_flag)
                strcpy(buf, "CTL c\n");
            if (sigtstp_flag)
                strcpy(buf, "CTL z\n");
            rc = strlen(buf);
            if(rc > 0) {
                if (send(sd, buf, rc, 0) < 0)
                    perror("sending stream message");
            }
            else {
                printf("one of the flags was set\n");
            }
            sigint_flag = 0;
            sigtstp_flag = 0;
        }
    }
        printf("EOF... exit\n");
        close(sd);
        kill(getppid(), 9);
        exit(0);
}

static void client_sig_handler(int signo) {
    if(signo == SIGINT) {
        sigint_flag = 1;
        printf("sigint caught\n");
    }
    if(signo == SIGTSTP) {
        sigtstp_flag = 1;
        printf("sigtstp caught\n");
    }
}
//static void client_sig_handler(int signo) {
//    if(signo == SIGINT) {
//        strcpy(buf, "CTL c\n");
//        printf("sigint caught %s\n", buf);
//        if(send(sd, buf, rc, 0) < 0)
//            perror("sending stream message");
//    }
//    if(signo == SIGTSTP) {
//        strcpy(buf, "CTL z\n");
//        printf("sigtstp caught\n");
//    }
//}