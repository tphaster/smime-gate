/**
 * File:        src/smime-gate.c
 * Description: S/MIME Gate client service function.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "smtp.h"
#include "system.h"

/** Constants **/
#define FNMAXLEN    32  /* filename maximum length */
#define MAILBUF      5  /* mail buffer size */


/* smime_gate_service - receive mails from client, process them and *
 *                      send to mail server <forward-path>          */
void smime_gate_service (int sockfd)
{
    int i, srvfd, ok;
    int srv = SMTP_SRV_NEW;
    int no_mails = 0;
    char **fns = Calloc(MAILBUF, sizeof(char *));
    char *filename = Malloc(FNMAXLEN);
    struct mail_object **mails = Calloc(MAILBUF, sizeof(struct mail_object *));
    struct mail_object *mail = Malloc(sizeof(struct mail_object));

    /* TODO: generate filename */

    while (0 == smtp_recv_mail(sockfd, mail, filename, srv)) {
        mails[no_mails] = mail;
        fns[no_mails] = filename;
        ++no_mails;

        if (NULL == (filename = malloc(FNMAXLEN))) {
            srv = SMTP_SRV_ERR;
            filename = NULL;
            mail = NULL;
            continue;
        }

        if (NULL == (mail = malloc(sizeof(struct mail_object)))) {
            srv = SMTP_SRV_ERR;
            free(filename);
            filename = NULL;
            mail = NULL;
            continue;
        }

        if (MAILBUF == no_mails)
            /* TODO: extending mail buffer */
            srv = SMTP_SRV_ERR;
        else
            srv = SMTP_SRV_NXT;
    }

    if (0 == no_mails)
        return;     /* no mails to process */

    /* TODO: encrypt/sign mail according to rules */

    /* forward all received mail objects */
    srvfd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(srvfd, (SA *) &(conf.mail_srv), sizeof(conf.mail_srv));
    srv = SMTP_SRV_NEW;
    ok = 0;

    for (i = 0; i < no_mails; ++i) {
        if (0 == smtp_send_mail(srvfd, mails[i], srv)) {
            remove(fns[i]);
        }
        else
            ok = 1;

        srv = SMTP_SRV_NXT;
    }

    if (0 != ok)
        err_msg("some mail objects cannot be send, they're postponed");
}

