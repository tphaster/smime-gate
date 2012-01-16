/**
 * File:         src/smime.c
 * Description:  S/MIME mail transformations.
 * Author:       Tomasz Pieczerak (tphaster)
 */

#include "smtp-types.h"

int encrypt_mail (struct mail_object *mail __attribute__ ((__unused__)))
{
    return 0;
}

int decrypt_mail (struct mail_object *mail __attribute__ ((__unused__)))
{
    return 0;
}

int sign_mail (struct mail_object *mail __attribute__ ((__unused__)))
{
    return 0;
}

int verify_mail (struct mail_object *mail __attribute__ ((__unused__)))
{
    return 0;
}

