/**
 * File:        include/smtp.h
 * Description: Header file for high-level SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __SMTP_H
#define __SMTP_H

#include "smtp-types.h"

/** Constants **/

/* Warnings */
#define WQUITNSEND   1  /* QUIT not send, mail successfully sent */
#define WQUITRNRCV   2  /* not received 221 reply after QUIT, mail sent */

/* Errors */
#define ESENDERR    -1  /* sending error */
#define ERECVERR    -2  /* receiving error */
#define EQUITRECV   -3  /* QUIT received before mail completion */
#define EUEXEOF     -4  /* unexpected end of file */
#define EFOPEN      -5  /* can't open file */

/* SMTP Server states (for smtp_recv_mail()) */
#define SMTP_SRV_NEW        0   /* use for the first receipt */
#define SMTP_SRV_NXT        1   /* use for a next receipt */
#define SMTP_SRV_ERR       -1   /* dysfunctional server */

/* SMTP Client states (for smtp_send_mail()) */
#define SMTP_CLI_NEW    0x1     /* for first mail */
#define SMTP_CLI_NXT    0x0     /* for next mail */
#define SMTP_CLI_LST    0x2     /* for last mail (close connection) */
#define SMTP_CLI_CON    0x0     /* don't close connection after sending */


/** Functions **/
int smtp_recv_mail (int sockfd, struct mail_object *mail, char *filename,
                    int srv);
int smtp_send_mail (int sockfd, struct mail_object *mail, int cli);
int save_mail_to_file (struct mail_object *mail, const char *filename);
int load_mail_from_file (const char *filename, struct mail_object *mail);

#endif  /* __SMTP_H */

