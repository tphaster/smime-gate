/**
 * File:        include/smtp.h
 * Description: Header file for high-level SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include "smtp-lib.h"

#define SERV_PORT   5555

int smtp_recv_mail (int sockfd, struct mail_object *mail);
int smtp_send_mail (int sockfd, struct mail_object *mail);

