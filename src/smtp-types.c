/**
 * File:        src/smtp-types.c
 * Description: Helper functions for operating with SMTP types.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <stdlib.h>
#include <stdio.h>
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

/* print_mail_object - print mail object's data to stderr */
void print_mail_object (struct mail_object *mail)
{
    unsigned int i;

    fprintf(stderr, "=== MAIL OBJECT ===\n");

    /* mail's SMTP envelope */
    fprintf(stderr, "FROM: %s\n", mail->mail_from);
    for (i = 0; i < mail->no_rcpt; ++i)
        fprintf(stderr, "TO:   %s\n", mail->rcpt_to[i]);

    /* mail's SMTP content */
    fprintf(stderr, "\nDATA (size: %d octets)\n%s=== END OF MAIL ===\n",
            mail->data_size, mail->data);
}

