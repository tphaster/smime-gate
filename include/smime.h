/**
 * File:        include/smime.h
 * Description: Header file for S/MIME mail processing.
 * Author:      Tomasz Pieczerak (tphaster)
 */

int encrypt_mail (struct mail_object *mail);
int decrypt_mail (struct mail_object *mail);
int sign_mail (struct mail_object *mail);
int verify_mail (struct mail_object *mail);

