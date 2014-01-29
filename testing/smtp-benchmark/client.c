/**
 * smime-gate-test (client) - full automated SMTP client, loads mail objects
 *                            from disc and sends them in a few sessions
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp.h"
#include "smtp-lib.h"
#include "smtp-types.h"

#define SMIME_GATE_PORT 5555

void str_cli (int sockfd, int n);
void* thread_r (void* arg);

int mails = 0, sessions = 0, kbytes = 0, errors = 0;
time_t interval;
int mails_per_session;
char *addr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int main (int argc, char **argv)
{
    pthread_t th_id;
    int ret = 0, threads;

    /* read arguments, very primitive */
    if (argc != 4)
        err_quit("usage: client <IPaddress> <mails per session> <threads>");

    addr = argv[1];
    mails_per_session = atoi(argv[2]);
    threads = atoi(argv[3]);
    interval = time(NULL) + 60;

    /* start threads */
    for (;threads > 0; --threads) {
        ret += pthread_create(&th_id, NULL, thread_r, NULL);
    }

    if (ret != 0)
        err_quit("ERROR: Some threads did not start, exiting...\n");

    /* read and print statistics */
    printf("mails,kbytes,errors,sessions\n");

    for (;;) {
        usleep(200);

        if (time(NULL) > interval) {
            interval += 60;

            pthread_mutex_lock(&mutex);
            printf("%d,%d,%d,%d\n", mails, kbytes, errors, sessions);
            mails = 0;
            kbytes = 0;
            errors = 0;
            sessions = 0;
            pthread_mutex_unlock(&mutex);
        }
    }

    return 0;
}

void* thread_r (void* arg __attribute__((__unused__)))
{
    int sockfd;
    struct sockaddr_in servaddr;

    for (;;) {
        /* create socket */
        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SMIME_GATE_PORT);
        inet_pton(AF_INET, addr, &servaddr.sin_addr);

        /* connect to server */
        Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

        /* run client */
        str_cli(sockfd, mails_per_session);
    }
    exit(0);
}

void str_cli (int sockfd, int mps)
{
    int ret, i, cli, inc = 1;
    struct mail_object mail;

    /* zero means infinity */
    if (mps == 0) {
        inc = 0;
        mps = 5;
    }

    /* one is the last */
    if (mps == 1)
        cli = SMTP_CLI_NEW | SMTP_CLI_LST;
    else
        cli = SMTP_CLI_NEW | SMTP_CLI_CON;

    pthread_mutex_lock(&mutex);
    ++sessions;
    pthread_mutex_unlock(&mutex);

    /* send given number of mail messages in one session */
    for (i = 0; i < mps; i += inc) {
        if (i+1 == mps && mps != 1)
            cli = SMTP_CLI_NXT | SMTP_CLI_LST;

        /* functions from smtp-lib are not thread safe */
        pthread_mutex_lock(&mutex);
        load_mail_from_file("./mail-test", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, cli))) {
            ++mails;
            kbytes += 96;
        }
        else {
            ++errors;
        }
        free_mail_object(&mail);
        pthread_mutex_unlock(&mutex);

        cli = SMTP_CLI_NXT | SMTP_CLI_CON;
    }
}

