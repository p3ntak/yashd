//
// Created by matt on 10/8/17.
//

#ifndef YASHD_SEMAPHORE_H
#define YASHD_SEMAPHORE_H

#include "daemon.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

extern sem_t mysem;

void write_to_log(char *buffer, size_t buffer_size, char *host_address, int host_port);

void write_to_log(char *buffer, size_t buffer_size, char *host_address, int host_port){
    FILE *log;
    time_t current_time;
    char *c_time_string;
    int ret;

    do {
        ret = sem_wait(&mysem);
        if (ret != 0) {
            /* the lock wasn't acquired */
            if (errno != EINVAL) {
                perror(" -- thread A -- Error in sem_wait. terminating -> ");
                pthread_exit(NULL);
            } else {
                /* sem_wait() has been interrupted by a signal: looping again */
                printf(" -- thread A -- sem_wait interrupted. Trying again for the lock...\n");
            }
        }
    } while (ret != 0);
    printf(" -- thread A -- lock acquired. Enter critical section\n");

    current_time = time(NULL);
    if (current_time == ((time_t)-1)){
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        return;
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);
    size_t string_size = strlen(c_time_string);
    c_time_string[string_size-1] = '\0';
    char yashd_str[] = " yashd[";
    size_t yashd_size = strlen(yashd_str);
    size_t host_addr_size = strlen(host_address);
    size_t port_length = sizeof(host_port);
    size_t final_string_size = string_size + yashd_size + host_addr_size + 5 + buffer_size + port_length;
    char port_string[port_length];
    sprintf(port_string, "%d", host_port);
    char *final_string = calloc(final_string_size, sizeof(char));
    char *colon = ":";
    char *right_brace = "]";
    char *space = " ";

    strcat(final_string, c_time_string);
    strcat(final_string, yashd_str);
    strcat(final_string, host_address);
    strcat(final_string, colon);
    strcat(final_string, port_string);
    strcat(final_string, right_brace);
    strcat(final_string, colon);
    strcat(final_string, space);
    strcat(final_string, buffer);

    log = fopen(u_log_path, "aw");
    if(log){
        fwrite(final_string,final_string_size,1,log);
    }
    fclose(log);
    printf(" -- thread A -- leaving critical section\n");
    ret = sem_post(&mysem);
    if (ret != 0) {
        perror(" -- thread A -- Error in sem_post");
        pthread_exit(NULL);
    }
    free(final_string);
}

#endif //YASHD_SEMAPHORE_H
