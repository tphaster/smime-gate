/**
 * File:        include/smtp-types.h
 * Description: Data types used by SMTP protocol implementation
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __MAIL_TYPES_H
#define __MAIL_TYPES_H

#include <sys/types.h>

/* Mail object */
struct mail_object {
    char *mail_from;    /* mail sender (MAIL FROM:) */
    char **rcpt_to;     /* mail recipient(s) (RCPT TO:) */
    size_t no_rcpt;     /* number of recipients */
    char *data;         /* mail body */
    size_t data_size;   /* mail body size */
};

void free_mail_object (struct mail_object *mail);
void print_mail_object (struct mail_object *mail);

#endif  /* __MAIL_TYPES_H */

