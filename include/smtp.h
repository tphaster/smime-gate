/**
 * File:        include/smtp.h
 * Description: Header file for high-level SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include "smtp-lib.h"

#define SERV_PORT   5555

int smtp_recv_mail (int sockfd, struct mail_object *mail, char *filename);
int smtp_send_mail (int sockfd, struct mail_object *mail);
int save_mail_to_disk (struct mail_object *mail, const char *filename);
int load_mail_from_disk (const char *filename, struct mail_object *mail);

