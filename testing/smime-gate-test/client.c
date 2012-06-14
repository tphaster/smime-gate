/**
 * smime-gate-test (client) - full automated SMTP cleint, loads mail object from
 *                            disc and sends them na a few sessions
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp.h"
#include "smtp-lib.h"
#include "smtp-types.h"

#define SMIME_GATE_PORT 5555

void str_cli (int sockfd, int n);

int main (int argc, char **argv)
{
    int sockfd, i;
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("usage: client <IPaddress>");

    /* send mails in three sessions */
    for (i = 1; i < 4; ++i) {
        /* create socket */
        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SMIME_GATE_PORT);
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

        /* connect to server */
        Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

        /* run client */
        str_cli(sockfd, i);
    }

    exit(0);
}

void str_cli (int sockfd, int n)
{
    int ret;
    struct mail_object mail;

    if (1 == n) {
        /* First session, one mail */

        /* mail should be signed */
        load_mail_from_file("./mailS", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NEW | SMTP_CLI_LST)))
            printf("MailS successfully sent!\n");
        else
            printf("MailS sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);
    }

    if (2 == n) {
        /* Second session, two mails */

        /* mail should be unchanged */
        load_mail_from_file("./mail1", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NEW | SMTP_CLI_CON)))
            printf("Mail1 successfully sent!\n");
        else
            printf("Mail1 sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);

        /* mail should be decrypted */
        load_mail_from_file("./mailD", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NXT | SMTP_CLI_LST)))
            printf("MailD successfully sent!\n");
        else
            printf("MailD sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);
    }

    if (3 == n) {
        /* Third session, three mails */

        /* mail should be verified */
        load_mail_from_file("./mailV", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NEW | SMTP_CLI_CON)))
            printf("MailV successfully sent!\n");
        else
            printf("MailV sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);

        /* mail should be encrypted */
        load_mail_from_file("./mailE", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NXT | SMTP_CLI_CON)))
            printf("MailE successfully sent!\n");
        else
            printf("MailE sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);

        /* mail should be unchanged */
        load_mail_from_file("./mail2", &mail);
        if (0 == (ret = smtp_send_mail(sockfd, &mail, SMTP_CLI_NXT | SMTP_CLI_LST)))
            printf("Mail2 successfully sent!\n");
        else
            printf("Mail2 sending ERROR (%d)!\n", ret);
        free_mail_object(&mail);
    }
}

