/**
 * File:        include/smtp-lib.h
 * Description: Header file for basic SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

int smtp_send_command (int sockfd);
int smtp_send_reply (int sockfd);

