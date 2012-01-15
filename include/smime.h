/**
 * File:        include/smime.h
 * Description: Header file for S/MIME mail processing.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __SMIME_H
#define __SMIME_H

int encrypt_mail (struct mail_object *mail);
int decrypt_mail (struct mail_object *mail);
int sign_mail (struct mail_object *mail);
int verify_mail (struct mail_object *mail);

#endif  /* __SMIME_H */

