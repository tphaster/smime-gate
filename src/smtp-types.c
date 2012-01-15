/**
 * File:        src/smtp-types.c
 * Description: Helper functions for operating with SMTP types.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <stdlib.h>
#include <string.h>
#include "smtp-types.h"

/* free_mail_object - free given mail object structure */
void free_mail_object (struct mail_object *mail)
{
    unsigned int i;

    if (NULL != mail->mail_from) {
        free(mail->mail_from);
        mail->mail_from = NULL;
    }

    if (NULL != mail->rcpt_to) {
        for (i = 0; i < mail->no_rcpt; ++i) {
            if (NULL != mail->rcpt_to[i])
                free(mail->rcpt_to[i]);
        }
        free(mail->rcpt_to);
        mail->rcpt_to = NULL;
    }

    if (NULL != mail->data)
        free(mail->data);

    memset(mail, 0, sizeof(struct mail_object));
}

