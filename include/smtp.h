/**
 * File:        include/smtp.h
 * Description: Header file for high-level SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __SMTP_H
#define __SMTP_H

#include "smtp-types.h"

/*** Constants ***/

/** Warnings **/
#define WQUITNSEND   1  /* QUIT not send, mail successfully sent */
#define WQUITRNRCV   2  /* not received 221 reply after QUIT, mail sent */

/** Errors **/
#define ESENDERR    -1  /* sending error */
#define ERECVERR    -2  /* receiving error */
#define EQUITRECV   -3  /* QUIT received before mail completion */
#define EUEXEOF     -4  /* unexpected end of file */
#define EFOPEN      -5  /* can't open file */

/** SMTP Server states (for smtp_recv_mail()) **/
#define SMTP_SRV_NEW        0   /* use for the first receipt */
#define SMTP_SRV_NXT        1   /* use for a next receipt */


/*** Functions ***/
int smtp_recv_mail (int sockfd, struct mail_object *mail, char *filename,
                    int srv);
int smtp_send_mail (int sockfd, struct mail_object *mail);
int save_mail_to_file (struct mail_object *mail, const char *filename);
int load_mail_from_file (const char *filename, struct mail_object *mail);

#endif  /* __SMTP_H */

