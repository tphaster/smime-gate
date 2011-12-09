/**
 * File:        include/filesystem.h
 * Description: Header file with declaration of function for various
 *              filesystem operations.
 * Author:      Tomasz Pieczerak (tphaster)
 */

int save_mail(struct mail_object mail, const char *path);
int load_mail(struct mail_object *mail, const char *path);

