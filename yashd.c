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
#include "my_semaphore.h"
#include "helpers.h"

#define MAXHOSTNAME 80
void reusePort(int sock);
void *EchoServe(void *arg);
struct proc_info{
    pthread_t my_tid;
    int my_socket;
    int shell_pid;
    int pthread_pipe_fd[2];
};

// Global Vars
int pid_ch1, pid_ch2, pid;
int activeJobsSize; //goes up and down as jobs finish
struct Job *jobs;
int *pactiveJobsSize = &activeJobsSize;
int psd;
char *buf;
int rc;

struct proc_info *proc_info_table;
int table_index_counter = 0;

// TODO: put main in a main loop so we can exit main without killing the yashd
int ret;
sem_t mysem;

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
    proc_info_table = malloc(sizeof(struct proc_info) * 500);
    listen(sd,4);
    fromlen = sizeof(from);
    for(;;){
        pthread_t thr;
        ssize_t rc;
/**     create a thread_data_t argument array */
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
        thread_data_t thr_data;
        thr_data.from = from;
        thr_data.psd = psd;
        proc_info_table[table_index_counter].my_socket = psd;
        if ((rc = pthread_create(&thr, NULL, EchoServe, &thr_data))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", (int) rc);
            close(psd);
            return EXIT_FAILURE;
        }
    }
}

// parent should do non blocking waitpid followed up a non blocking receive so a CTL signal can be received
void *EchoServe(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    proc_info_table[table_index_counter].my_tid = pthread_self();
    int psd = data->psd;
    struct sockaddr_in from = data->from;
    char buf[512];
    ssize_t rc;
    struct  hostent *hp, *gethostbyname();
    int new_pid;
    char **args;
    char *buf_copy;
//    dup2(psd, STDOUT_FILENO);

    char *prompt;
    prompt = strdup("# ");
    rc = strlen(prompt);
    if (send(psd, prompt, (size_t) rc, 0) < 0)
        perror("sending stream message");

    if ((hp = gethostbyaddr((char *)&from.sin_addr.s_addr,
                            sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));

    if(pipe(proc_info_table[table_index_counter].pthread_pipe_fd) ==-1){
        perror("pthread pipe\n");
        exit(-1);
    }

    /**  get data from  clients and send it back */
    new_pid = fork();
    if(new_pid == 0) {
        close(proc_info_table[table_index_counter].pthread_pipe_fd[1]);
        dup2(proc_info_table[table_index_counter].pthread_pipe_fd[0], STDIN_FILENO);
        proc_info_table[table_index_counter].shell_pid = getpid();


        yash_prog_loop(buf, psd);
    }
    else if (new_pid > 0){
        int returned_index = get_proc_info_index_pid(getpid());
        printf("my_socket: %d, returned index: %d, pid: %d\n",proc_info_table[returned_index].my_socket, returned_index, getpid());
        close(proc_info_table[table_index_counter].pthread_pipe_fd[0]);

        table_index_counter++;
        for(;;){
            cleanup(buf);
            if( (rc=recv(psd, buf, sizeof(buf), 0)) < 0){
                perror("receiving stream  message");
                exit(-1);
            }
            dup2(proc_info_table[get_proc_info_index_by_tid(pthread_self())].pthread_pipe_fd[1],STDOUT_FILENO);
            if (rc > 0){
                buf[rc]='\0';
                buf_copy = strdup(buf);
                args = parseLine(buf_copy);
                if (strcmp(args[0], "CTL") == 0) {
                    if (strcmp(args[1], "c") == 0) kill(new_pid, SIGINT);
                    if (strcmp(args[1], "z") == 0) kill(new_pid, SIGTSTP);
                }
                if (strcmp(args[0], "CMD") == 0) {
                    write_to_log(buf, (size_t) rc, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                    printf("%s",buf);
                    fflush(stdout);
                }
            }
        }
    }
}

int get_proc_info_index_pid(int pid){
    for(int i=0; i<table_index_counter + 1; i++){
        if(proc_info_table[i].shell_pid == pid){
            return i;
        }
    }
    return 0;
}

int get_proc_info_index_by_tid(pthread_t tid){
    for(int i=0; i<table_index_counter ; i++){
        if(pthread_equal(proc_info_table[i].my_tid, tid) != 0){
            return i;
        }
    }
    return 0;
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


//main to take arguments and start a loop
//int yash_prog_loop(int argc, char **argv)
void yash_prog_loop(char *buf_passed, int psd_passed)
{
    jobs = malloc(sizeof(struct Job) * MAX_NUMBER_JOBS);
    psd = psd_passed;
    buf = strdup(buf_passed);

    mainLoop();
//    return EXIT_SUCCESS;
}

void mainLoop(void)
{
    int status = 0;
    char *line;
    char **args;
    activeJobsSize = 0;

    //read input line
    //parse input
    //stay in loop until an exit is requested
    //while waiting for user input SIGINT is ignored so ctrl+c will not stop the shell
    // ignore sigint and sigtstp while waiting for input

    char *prompt;
    prompt = strdup("\n# ");
    do {
        signal(SIGINT, sig_handler);
        signal(SIGTSTP, sig_handler);
        line = readLineIn();
        int returned_index = get_proc_info_index_pid(getpid());
        dup2(proc_info_table[returned_index].my_socket, STDOUT_FILENO);
        printf("my_socket: %d, returned index: %d, pid: %d\n",proc_info_table[returned_index].my_socket, returned_index, getpid());
        printf("table index %d\n", table_index_counter);
        for(int i=0; i<table_index_counter+1; i++){
            printf("table elem[%d]: socket: %d, pid: %d\n", i,proc_info_table[i].my_socket, proc_info_table[i].shell_pid);
        }

        if (line == NULL) {
            printf("\n");
            killProcs(jobs, pactiveJobsSize);
            return;
        }
        if (strcmp(line, "") == 0) return;
        char *lineCpy = strdup(line);
        args = parseLine(line);
        char **fixed_args;
        if (strcmp(args[0], "CMD") == 0) {
            fixed_args = &args[1];
        }
        status = executeLine(fixed_args, lineCpy);
        fflush(stdout);

        printf("%s",prompt);
        fflush(stdout);
    } while(status);
    return;
}


int executeLine(char **args, char *line)
{
    if(!*args) return FINISHED_INPUT;
    int returnVal;
    int inBackground = containsAmp(args);   //check if args contains '&'
    int inputPiped = pipeQty(args);         //get number of pipes in the command

    if(!(
            (strcmp(args[0], BUILT_IN_BG) == 0) ||
            (strcmp(args[0], BUILT_IN_FG) == 0) ||
            (strcmp(args[0], BUILT_IN_JOBS) == 0)))
    {
        addToJobs(jobs, line, pactiveJobsSize);
    }

    // check if command is a built in command
    if(strcmp(args[0], BUILT_IN_BG) == 0)
    {
        yash_bg(jobs, activeJobsSize);
        return FINISHED_INPUT;
    }
    if(strcmp(args[0], BUILT_IN_FG) == 0)
    {
        yash_fg(jobs, activeJobsSize);
        return FINISHED_INPUT;
    }
    if(strcmp(args[0], BUILT_IN_JOBS) == 0)
        return yash_jobs(jobs, activeJobsSize);

    //make sure & and | are not both in the argument
    if(!pipeBGExclusive(args))
    {
        printf("Cannot background and pipeline commands "
                       "('&' and '|' must be used separately).");
        return FINISHED_INPUT;
    }

    // if there are more than 1 or less than 0 pipes reject the input as it is not a valid command
    if (inputPiped > 1 || inputPiped < 0)
    {
        printf("Only one '|' allowed per line");
        return FINISHED_INPUT;
    }

    //if there is a | in the argument then
    if(inputPiped == 1)
    {
        struct PipedArgs pipedArgs = getTwoArgs(args);
        returnVal = startPipedOperation(pipedArgs.args1, pipedArgs.args2);
        return returnVal;
    }

    if(inBackground)
    {
        returnVal = startBgOperation(args);
    } else
    {
        returnVal = startOperation(args);
    }
    return returnVal;
}

int startBgOperation(char **args)
{
    removeAmp(args);
    FILE *writeFilePointer = NULL;
    FILE *readFilePointer = NULL;
    int argCount = countArgs(args);
    int redirIn = containsInRedir(args);
    int redirOut = containsOutRedir(args);
    int fd = open("/dev/null", O_WRONLY);

    pid_ch1 = fork();
    if (pid_ch1 == 0)
    {
        setsid();
        if (redirOut >= 0)
        {
            if(setRedirOut(args, redirOut, writeFilePointer, argCount) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if (redirIn >= 0)
        {
            if(setRedirIn(args, redirIn, readFilePointer, argCount) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if((redirIn < 0 && redirOut <0) || (redirIn >= 0 && redirOut <0))
            dup2(fd, STDOUT_FILENO);
        if(execvp(args[0], args) == -1)
        {
            perror("Problem executing command");
            removeLastFromJobs(jobs, pactiveJobsSize);
            _Exit(EXIT_FAILURE);
        }

    } else if (pid_ch1 < 0)
    {
        perror("error forking");
    } else if (pid_ch1 > 0)
    {
        signal(SIGCHLD, proc_exit);
        startJobsPID(jobs, pid_ch1, activeJobsSize);
    }
    if(writeFilePointer != NULL) fclose(writeFilePointer);
    if(readFilePointer != NULL) fclose(readFilePointer);
    return FINISHED_INPUT;
}

int startOperation(char **args)
{
    int status;
    removeAmp(args);
    FILE *writeFilePointer = NULL;
    FILE *readFilePointer = NULL;
    int argCount = countArgs(args);
    int redirIn = containsInRedir(args);
    int redirOut = containsOutRedir(args);

    if (signal(SIGINT, sig_int) == SIG_ERR)
    {
        printf("signal(SIGINT)_error");
    }
    if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
    {
        printf("signal(SIGTSTP)_error");
    }

    pid_ch1 = fork();
    if(pid_ch1 == 0)
    {
        // child process
        if (redirOut >= 0)
        {
            if(setRedirOut(args, redirOut, writeFilePointer, argCount) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if (redirIn >= 0) {
            if (setRedirIn(args, redirIn, readFilePointer, argCount) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if(execvp(args[0], args) == -1)
        {
            perror("Problem executing command");
            removeLastFromJobs(jobs, pactiveJobsSize);
            _Exit(EXIT_FAILURE);
        }
    } else if(pid_ch1 < 0)
    {
        perror("error forking");
    } else
    {
        // Parent process
        startJobsPID(jobs, pid_ch1, activeJobsSize);
        // change sig catchers back to not ignore signals
        int count = 0;
        while(count<1)
        {
            pid = waitpid(pid_ch1, &status, WUNTRACED | WCONTINUED);
            if (pid == -1) {
                perror("waitpid");
            }
            if (WIFEXITED(status)) {
                removeFromJobs(jobs, pid_ch1, pactiveJobsSize);
                count++;
            } else if (WIFSTOPPED(status)) {
                setJobStatus(jobs, pid_ch1, activeJobsSize, STOPPED);
                count++;
            }
        }
    }
    if(writeFilePointer != NULL) fclose(writeFilePointer);
    if(readFilePointer != NULL) fclose(readFilePointer);
    return FINISHED_INPUT;
}


int startPipedOperation(char **args1, char **args2)
{
    int status;
    int pfd[2];
    FILE *writeFilePointer = NULL;
    FILE *readFilePointer = NULL;
    int argCount1 = countArgs(args1);
    int argCount2 = countArgs(args2);
    int redirIn1 = containsInRedir(args1);
    int redirIn2 = containsInRedir(args2);
    int redirOut1 = containsOutRedir(args1);
    int redirOut2 = containsOutRedir(args2);

    if (pipe(pfd) == -1)
    {
        perror("pipe");
        return FINISHED_INPUT;
    }

    pid_ch1 = fork();
    if(pid_ch1 > 0)
    {
        //parent
        pid_ch2 = fork();
        if(pid_ch2 > 0)
        {
            if(signal(SIGINT, sig_handler) == SIG_ERR)
            {
                printf("signal(SIGINT)_error");
            }
            if(signal(SIGTSTP, sig_tstp) == SIG_ERR)
            {
                printf("signal(SIGTSTP)_error");
            }
            close(pfd[0]);
            close(pfd[1]);
            int count = 0;
            while(count<2)
            {
                pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
                startJobsPID(jobs, pid_ch1, activeJobsSize);
                if(pid == -1)
                {
                    perror("waitpid");
                    return FINISHED_INPUT;
                }
                if(WIFEXITED(status))
                {
                    removeFromJobs(jobs, pid_ch1, pactiveJobsSize);
                    count++;
                } else if(WIFSIGNALED(status))
                {
                    count++;
                } else if(WIFSTOPPED(status))
                {
                    setJobStatus(jobs, pid_ch1, activeJobsSize, STOPPED);
                    count++;
                } else if(WIFCONTINUED(status))
                {
                    setJobStatus(jobs, pid_ch1, activeJobsSize, RUNNING);
                    pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
                }
            }
            return FINISHED_INPUT;
        } else
        {
            // child 2
            setpgid(0, pid_ch1);
            close(pfd[1]);
            dup2(pfd[0],STDIN_FILENO);

            if (redirOut2 >= 0)
            {
                if(setRedirOut(args2, redirOut2, writeFilePointer, argCount2) == -1)
                {
                    removeLastFromJobs(jobs, pactiveJobsSize);
                    _exit(EXIT_FAILURE);
                    return FINISHED_INPUT;
                }
            }

            if (redirIn2 >= 0)
            {
                if(setRedirIn(args2, redirIn2, readFilePointer, argCount2) == -1)
                {
                    removeLastFromJobs(jobs, pactiveJobsSize);
                    _exit(EXIT_FAILURE);
                    return FINISHED_INPUT;
                }
            }

            if(execvp(args2[0], args2) == -1)
            {
                perror("Problem executing command 2");
                removeLastFromJobs(jobs, pactiveJobsSize);
                _Exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }
    } else
    {
        // child 1
        setsid();
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);

        if (redirOut1 >= 0)
        {
            if(setRedirOut(args1, redirOut1, writeFilePointer, argCount1) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if (redirIn1 >= 0)
        {
            if(setRedirIn(args1, redirIn1, readFilePointer, argCount1) == -1)
            {
                removeLastFromJobs(jobs, pactiveJobsSize);
                _exit(EXIT_FAILURE);
                return FINISHED_INPUT;
            }
        }

        if(execvp(args1[0], args1) == -1)
        {
            perror("Problem executing command 1");
            removeLastFromJobs(jobs, pactiveJobsSize);
            _Exit(EXIT_FAILURE);
            return FINISHED_INPUT;
        }
    }
    if(writeFilePointer != NULL) fclose(writeFilePointer);
    if(readFilePointer != NULL) fclose(readFilePointer);
    return FINISHED_INPUT;
}


static void sig_int(int signo)
{
    printf("killing %d\n", pid_ch1);
    kill(pid_ch1, SIGINT);
}


static void sig_tstp(int signo)
{
    kill(pid_ch1, SIGTSTP);
    kill(-pid_ch1, SIGTSTP);
}

void proc_exit(int signo)
{
    pid_t	sig_chld_pid;
    sig_chld_pid = wait(NULL);
    for(int i=0; i<activeJobsSize; i++)
    {
        if(jobs[i].pid_no == sig_chld_pid)
            printf("\n[%d] DONE    %s\n", jobs[i].task_no, jobs[i].line);
    }
    printf("# ");
    fflush(stdout);
    removeFromJobs(jobs, sig_chld_pid, pactiveJobsSize);
    signal(SIGCHLD,SIG_DFL);
    return;
}

void fg_handler(int signo)
{
    pid_t	sig_chld_pid;

    sig_chld_pid = wait(NULL);
    removeFromJobs(jobs, sig_chld_pid, pactiveJobsSize);
    signal(SIGCHLD,SIG_DFL);
}

static void sig_handler(int signo) {
    char *resp;
    switch(signo){
        case SIGINT:
            signal(signo,SIG_IGN);
            signal(SIGINT,sig_handler);
            resp = strdup("ctrl c received\n");
            send_response(resp);
            break;
        case SIGTSTP:
            signal(signo,SIG_IGN);
            signal(SIGTSTP,sig_handler);
            resp = strdup("ctrl z received\n");
            send_response(resp);
            break;
        case SIGCHLD:
            signal(signo,SIG_IGN);
            signal(SIGCHLD, sig_handler);
            break;
        default:
            return;
    }

}

void cleanup(char *buf)
{
    int i;
    int buf_size = (int) strlen(buf) + 1;
    for(i=0; i<buf_size; i++) buf[i]='\0';
}

void send_response(char *send_str) {
    rc = (int) strlen(send_str);
    if (send(psd, send_str, (size_t) rc, 0) < 0)
        perror("sending stream message");
    cleanup(send_str);
}

//returns how many '|' are in the arguments
int pipeQty(char **args)
{
    int pipeCount = 0;
    int numArgs = countArgs(args);
    for (int i=0; i<numArgs;i++)
    {
        if(strstr(args[i],"|") && strlen(args[i]) == 1) pipeCount++;
    }
    return pipeCount;
}

//returns 0 if pipe and & are not exclusive
int pipeBGExclusive(char **args)
{
    int pipeCount = 0;
    int backgroundCount = 0;
    int numArgs = countArgs(args);
    for (int i=0; i<numArgs; i++)
    {
        if(strstr(args[i],"|")) pipeCount++;
        if(strstr(args[i],"&")) backgroundCount++;
    }
    if((pipeCount>0) && (backgroundCount>0))
    {
        return 0;
    }
    return 1;
}

//determine how many arguments were given to input
int countArgs(char **args)
{
    if(!*args) return 0;
    int numArgs = 0;
    int i=0;
    while(args[i] != NULL)
    {
        numArgs++;
        i++;
    }
    return numArgs;
}

//read input until end of file or new line
char *readLineIn(void)
{
    char *line = calloc(MAX_INPUT_LENGTH + 1, sizeof(char));

    if(!line)
    {
        fprintf(stderr,"line in memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    line = fgets(line,MAX_INPUT_LENGTH+1,stdin);
    if(line == NULL)
        return NULL;
    if(strcmp(line,"\n") == 0) return "";
    char *lineCopy = strdup(line);

    free(line);
    return lineCopy;
}

// the following line parser was taken from https://brennan.io/2015/01/16/write-a-shell-in-c/
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **parseLine(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// splits a piped argument into a struct containing two separate arguments
struct PipedArgs getTwoArgs(char **args)
{
    char **args1 = malloc(sizeof(char*) * MAX_INPUT_LENGTH);
    char **args2 = malloc(sizeof(char*) * MAX_INPUT_LENGTH);
    int numArgs = countArgs(args);
    int i = 0;
    int k = 0;
    while(!strstr(args[i],"|"))
    {
        args1[k] = args[i];
        i++;
        k++;
    };
    args1[k] = NULL;
    i++;
    int j = 0;
    while(i < numArgs)
    {
        args2[j] = args[i];
        i++;
        j++;
    };
    args2[j] = NULL;
    struct PipedArgs pipedArgs = {args1, args2};
    return pipedArgs;
}

// add a process to jobs table
void addToJobs(struct Job *jobs, char *line, int *activeJobsSize)
{
    jobs[*activeJobsSize].line = strdup(line);

    if(*activeJobsSize == 0)
        jobs[*activeJobsSize].task_no = 1;
    else
        jobs[*activeJobsSize].task_no = jobs[*activeJobsSize-1].task_no + 1;

    jobs[*activeJobsSize].runningStatus = STOPPED;
    jobs[*activeJobsSize].pid_no = 0;

    (*activeJobsSize)++;
    return;
}

// built in jobs command. prints out each job's pid number, jobs number, and status
int yash_jobs(struct Job *jobs, int activeJobsSize)
{
    for(int i=0; i<activeJobsSize; i++)
    {
        char *runningStr;

        if(jobs[i].runningStatus)
            runningStr = "Running";
        else
            runningStr = "Stopped";

        if(i == activeJobsSize-1)
            printf("[%d] + %s    %s\n", jobs[i].task_no, runningStr , jobs[i].line);
        else
            printf("[%d] - %s    %s\n", jobs[i].task_no, runningStr, jobs[i].line);
    }
    if(activeJobsSize == 0) printf("No active jobs\n");
    return FINISHED_INPUT;
}

// built in fg command. puts the most recent command from the jobs table into the foreground
void yash_fg(struct Job *jobs, int activeJobSize)
{
    int status;
    signal(SIGCONT, SIG_DFL);

    if(activeJobSize == 0)
    {
        printf("yash: No active jobs");
        return;
    }

    int pid = jobs[activeJobSize - 1].pid_no;
    setJobStatus(jobs, pid, activeJobSize, RUNNING);
    for(int i=0; i<activeJobSize; i++)
    {
        char *runningStr;
        if(jobs[i].runningStatus)
            runningStr = "Running";
        else
            runningStr = "Stopped";
        if(i == activeJobSize-1)
        {
            if(jobs[i].pid_no == pid)
                printf("[%d] + %s    %s\n", jobs[i].task_no, runningStr , jobs[i].line);

        } else
        {
            if(jobs[i].pid_no == pid)
                printf("[%d] - %s    %s\n", jobs[i].task_no, runningStr, jobs[i].line);
        }
    }
    signal(SIGCHLD, fg_handler);
    kill(pid, SIGCONT);
    wait(NULL);
    return;
}

// build in bg command. puts the most recent stopped job in the background
void yash_bg(struct Job *jobs, int activeJobSize)
{
    int pid=0;
    signal(SIGCONT, SIG_DFL);

    if(activeJobSize == 0)
    {
        printf("yash: No active jobs");
        return;
    }
    for(int i=activeJobSize-1; i>=0; i--)
    {
        if(pipeQty(parseLine(jobs[i].line)) != 0) continue;
        if(jobs[i].runningStatus == STOPPED)
        {
            pid = jobs[i].pid_no;
            break;
        }
    }
    setJobStatus(jobs, pid, activeJobSize, RUNNING);
    for(int i=0; i<activeJobSize; i++)
    {
        char *runningStr;

        if(jobs[i].runningStatus)
            runningStr = "Running";
        else
            runningStr = "Stopped";

        if(i == activeJobSize-1)
        {
            if(jobs[i].pid_no == pid)
                printf("[%d] + %s    %s\n", jobs[i].task_no, runningStr , jobs[i].line);

        } else
        {
            if(jobs[i].pid_no == pid)
                printf("[%d] - %s    %s\n", jobs[i].task_no, runningStr, jobs[i].line);
        }
    }
    signal(SIGCHLD, proc_exit);
    kill(pid, SIGCONT);
    return;
}

// updates the jobs table with the pid number and gives the job a 'running' status
void startJobsPID(struct Job *jobs, int pid, int activeJobsSize)
{
    jobs[activeJobsSize-1].pid_no = pid;
    jobs[activeJobsSize-1].runningStatus = RUNNING;
    return;
}

// removes a job by pid number from the jobs table. should be used when a job is finished or killed.
void removeFromJobs(struct Job *jobs, int pid, int *activeJobsSize)
{
    for(int i=0; i<*activeJobsSize; i++)
    {
        if((jobs[i].pid_no == pid))
        {
            for(int j=i; j<(*activeJobsSize-1); j++)
            {
                jobs[j].pid_no = jobs[j+1].pid_no;
                jobs[j].runningStatus = jobs[j+1].runningStatus;
                jobs[j].task_no = jobs[j+1].task_no;
                jobs[j].line = strdup(jobs[j+1].line);
            }
            jobs[*activeJobsSize-1].pid_no = 0;
            jobs[*activeJobsSize-1].runningStatus = STOPPED;
            jobs[*activeJobsSize-1].task_no = 0;
            jobs[*activeJobsSize-1].line = NULL;
            (*activeJobsSize)--;
        }
    }
    return;
}

// changes the status of a job in the jobs table in the event that it is stopped or restarted
void setJobStatus(struct Job *jobs, int pid, int activeJobsSize, int runningStatus)
{
    for(int i=0; i<activeJobsSize; i++)
    {
        if(jobs[i].pid_no == pid) jobs[i].runningStatus = runningStatus;
    }
    return;
}

// kills all process in the jobs table in the event that the shell is killed with ctrl + d
void killProcs(struct Job *jobs, int *activeJobsSize)
{
    for(int i=0; i<*activeJobsSize; i++)
    {
        kill(-jobs[i].pid_no, SIGINT);
        removeFromJobs(jobs, jobs[i].pid_no, activeJobsSize);
        (*activeJobsSize)--;
    }
}

// checks if the input arguments contain an & as an argument
int containsAmp(char **args)
{
    int argCount = countArgs(args);
    for (int i=0; i<argCount; i++)
    {
        if(strstr(args[i],"&")) return 1;
    }
    return 0;
}

// removes the most recent job from the jobs table in the event that the job was put in the table but killed before
// the pid no was assigned
void removeLastFromJobs(struct Job *jobs, int *activeJobsSize)
{
    jobs[*activeJobsSize-1].pid_no = 0;
    jobs[*activeJobsSize-1].runningStatus = STOPPED;
    jobs[*activeJobsSize-1].task_no = 0;
    jobs[*activeJobsSize-1].line = NULL;

    (*activeJobsSize)--;
    return;
}

// removes the '&' from the input arguments so the path command can be executed
void removeAmp(char **args)
{
    int argCount = countArgs(args);
    if(strcmp(args[argCount-1],"&") == 0)
        args[argCount-1] = NULL;
    return;
}

// checks if input arguments have a '<' symbol and returns the index of the symbol in the args array
int containsInRedir(char **args)
{
    int symbolPos = -1; // return -1 if '<' is not in args
    int argCount = countArgs(args);

    for(int i=0; i<argCount; i++)
    {
        if(strcmp(args[i],"<") == 0)
            symbolPos = i;
    }

    return symbolPos;
}

// checks if input arguments have a '>' symbol and returns the index of the symbol in the args array
int containsOutRedir(char **args)
{
    int symbolPos = -1; // return -1 if '<' is not in args
    int argCount = countArgs(args);

    for(int i=0; i<argCount; i++)
    {
        if(strcmp(args[i],">") == 0)
            symbolPos = i;
    }

    return symbolPos;
}

// removes the redirect argument and the next argument in the array. assumes the next argument is the name of the file
// any additional input in the args command will not be recognized by path command
void removeRedirArgs(char **args, int redirIndex)
{
    args[redirIndex] = NULL;
    args[redirIndex + 1] = NULL;
    return;
}

// set stdout to go to file specified by the writeFilePointer
int setRedirOut(char **args, int redirOut, FILE *writeFilePointer, int argCount)
{
    if (redirOut + 1 < argCount)
    {
        writeFilePointer = fopen(args[redirOut + 1], "w+");
        if (writeFilePointer)
        {
            dup2(fileno(writeFilePointer), STDOUT_FILENO);
            removeRedirArgs(args, redirOut);
        } else
        {
            fprintf(stderr, "Cannot open file %s\n", args[redirOut + 1]);
            return -1; // return -1 as error value
        }
    } else
    {
        fprintf(stderr, "Invalid Expression\n");
        return -1; // return -1 as error value
    }

    return 1; // Finished without error
}

int setRedirIn(char **args, int redirIn, FILE *readFilePointer, int argCount)
{
    if (redirIn + 1 < argCount)
    {
        readFilePointer = fopen(args[redirIn + 1], "r");
        if (readFilePointer)
        {
            dup2(fileno(readFilePointer), STDIN_FILENO);
            removeRedirArgs(args, redirIn);
        } else
        {
            fprintf(stderr, "Cannot open file %s\n", args[redirIn + 1]);
            return -1;
        }
    } else
    {
        fprintf(stderr, "Invalid Expression\n");
        return -1;
    }

    return 1;
}
