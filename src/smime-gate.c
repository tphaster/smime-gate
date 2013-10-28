/**
 * File:        src/smime-gate.c
 * Description: S/MIME Gate client service function.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/prctl.h>
#define _GNU_SOURCE
#include <libgen.h>

#include "config.h"
#include "smtp.h"
#include "system.h"

/** Local functions **/
static char *generate_filename (unsigned int nr);
int smime_process_mails (struct mail_object **mails, char **fns, int no_mails);
char *strcasestr(const char *haystack, const char *needle);


/* smime_gate_service - receive mails from client, process them and *
 *                      send to mail server <forward-path>          */
void smime_gate_service (int sockfd)
{
    int i, srvfd;
    int srv = SMTP_SRV_NEW;
    int no_mails = 0;
    char **fns = Calloc(MAILBUF, sizeof(char *));
    char *filename;
    char *unsent = Malloc(FNMAXLEN);
    struct mail_object **mails = Calloc(MAILBUF, sizeof(struct mail_object *));
    struct mail_object *mail = Malloc(sizeof(struct mail_object));

    if (NULL == (filename = generate_filename(no_mails)))
        err_sys("malloc error");

    /* receive mail objects from client */
    while (0 == smtp_recv_mail(sockfd, mail, filename, srv)) {
        mails[no_mails] = mail;
        fns[no_mails] = filename;
        ++no_mails;
#ifdef DEBUG
        err_msg("received mail, saved in %s", filename);
#endif

        if (NULL == (filename = generate_filename(no_mails))) {
            srv = SMTP_SRV_ERR;
            filename = NULL;
            mail = NULL;
            continue;
        }

        if (NULL == (mail = malloc(sizeof(struct mail_object)))) {
            srv = SMTP_SRV_ERR;
            free(filename);
            filename = NULL;
            mail = NULL;
            continue;
        }

        if (MAILBUF == no_mails)
            srv = SMTP_SRV_ERR;
        else
            srv = SMTP_SRV_NXT;
    }
    free(filename);
    free(mail);
    filename = NULL;
    mail = NULL;

    if (0 == no_mails)
        goto end_service;   /* no mails to process */

    smime_process_mails(mails, fns, no_mails);

    /* forward all received mail objects */
    srvfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (connect(srvfd, (SA *) &(conf.mail_srv), sizeof(conf.mail_srv)) < 0) {
        /* mails cannot be sent now, move it to unsent directory */
        for (i = 0; i < no_mails; ++i) {
            snprintf(unsent, FNMAXLEN, DEFAULT_UNSENT_DIR "/%s", basename(fns[i]));
            rename(fns[i], unsent);

            free_mail_object(mails[i]);
            free(mails[i]);
            free(fns[i]);
        }
        err_sys("connect error");
    }

    if (1 == no_mails)
        srv = SMTP_CLI_NEW | SMTP_CLI_LST;
    else
        srv = SMTP_CLI_NEW | SMTP_CLI_CON;

    for (i = 0; i < no_mails; ++i) {
        if (0 == smtp_send_mail(srvfd, mails[i], srv))
            remove(fns[i]);
        else {  /* mail cannot be sent now, move it to unsent directory */
            snprintf(unsent, FNMAXLEN, DEFAULT_UNSENT_DIR "/%s", basename(fns[i]));
            rename(fns[i], unsent);
        }

        free_mail_object(mails[i]);
        free(mails[i]);
        free(fns[i]);

        if (no_mails-2 == i)
            srv = SMTP_CLI_NXT | SMTP_CLI_LST;
        else
            srv = SMTP_CLI_NXT | SMTP_CLI_CON;
    }

end_service:
    free(unsent);
    free(mails);
    free(fns);
    free_config();
}

/* generate_filename - generate unique filename for mail, returns allocated *
 *                     pointer (behaves like malloc())                      */
static char *generate_filename (unsigned int nr)
{
    char *fn = malloc(FNMAXLEN);
    unsigned int t, p;

    if (NULL != fn) {
        t = time(NULL);
        p = getpid();

        snprintf(fn, FNMAXLEN, DEFAULT_WORKING_DIR "/mail%d_%d-%d", t, p, nr);
    }

    return fn;
}

/* smime_process_mails - process mail objects, according to rules in config */
int smime_process_mails (struct mail_object **mails, char **fns, int no_mails)
{
    int m, toprcs, sign_encr, ret;
    unsigned int r;
    char cmd[CMDMAXLEN];

    for (m = 0; m < no_mails; ++m) {

        /** signing rules **/
        toprcs = 0;
        sign_encr = 0;
        for (r = 0; r < conf.sign_rules_size; ++r) {
            if (NULL != conf.sign_rules[r].sndr) {
                if (strcasestr(mails[m]->mail_from, conf.sign_rules[r].sndr)) {
                    toprcs = 1;
                    break;
                }
            }
        }
        /* did we find matching rule? */
        if (toprcs) {
            snprintf(cmd, CMDMAXLEN,
                "smime-tool -sign -cert %s -key %s -pass %s %s > %s.prcs",
                conf.sign_rules[r].cert_path, conf.sign_rules[r].key_path,
                conf.sign_rules[r].key_pass, fns[m], fns[m]);

            ret = system(cmd);
            snprintf(cmd, CMDMAXLEN, "%s.prcs", fns[m]);

            if (0 == ret) {
                if (0 == rename(cmd, fns[m])) {
                    free_mail_object(mails[m]);
                    load_mail_from_file(fns[m], mails[m]);
                    sign_encr = 1;
                    /* signing successful */
                }
            }
            remove(cmd);
        }
        /** end of signing rules **/

        /** encryption rules **/
        toprcs = 0;
        for (r = 0; r < conf.encr_rules_size; ++r) {
            if (NULL != conf.encr_rules[r].rcpt) {
                /* when there is only one recipient, encryption makes sense */
                if (1 == mails[m]->no_rcpt) {
                    if (strcasestr(mails[m]->rcpt_to[0],
                                conf.encr_rules[r].rcpt))
                    {
                        toprcs = 1;
                        break;
                    }
                }
            }
        }
        /* did we find matching rule? */
        if (toprcs) {
            snprintf(cmd, CMDMAXLEN,
                    "smime-tool -encrypt -cert %s %s > %s.prcs",
                    conf.encr_rules[r].cert_path, fns[m], fns[m]);

            ret = system(cmd);
            snprintf(cmd, CMDMAXLEN, "%s.prcs", fns[m]);

            if (0 == ret) {
                if (0 == rename(cmd, fns[m])) {
                    free_mail_object(mails[m]);
                    load_mail_from_file(fns[m], mails[m]);
                    sign_encr = 1;
                    /* encryption successful */
                }
            }
            remove(cmd);
        }
        /** end of encryption rules **/

        if (sign_encr)  /* there is no sense in decrypting/verifying mails, */
            continue;   /* which has just been encrypted/signed.            */

        /** decryption rules **/
        toprcs = 0;
        for (r = 0; r < conf.decr_rules_size; ++r) {
            if (NULL != conf.decr_rules[r].rcpt) {
                /* when there is only one recipient, decryption makes sense */
                if (1 == mails[m]->no_rcpt) {
                    if (strcasestr(mails[m]->rcpt_to[0],
                                conf.decr_rules[r].rcpt))
                    {
                        toprcs = 1;
                        break;
                    }
                }
            }
        }
        /* did we find matching rule? */
        if (toprcs) {
            snprintf(cmd, CMDMAXLEN,
                "smime-tool -decrypt -cert %s -key %s -pass %s %s > %s.prcs",
                conf.decr_rules[r].cert_path, conf.decr_rules[r].key_path,
                conf.decr_rules[r].key_pass, fns[m], fns[m]);

            ret = system(cmd);
            snprintf(cmd, CMDMAXLEN, "%s.prcs", fns[m]);
            if (0 == ret) {
                if (0 == rename(cmd, fns[m])) {
                    free_mail_object(mails[m]);
                    load_mail_from_file(fns[m], mails[m]);
                    /* decryption successful */
                }
            }
            remove(cmd);
        }
        /** end of decryption rules **/

        /** verification rules **/
        toprcs = 0;
        for (r = 0; r < conf.vrfy_rules_size; ++r) {
            if (NULL != conf.vrfy_rules[r].sndr) {
                if (strcasestr(mails[m]->mail_from, conf.vrfy_rules[r].sndr)) {
                    toprcs = 1;
                    break;
                }
            }
        }
        /* did we find matching rule? */
        if (toprcs) {
            snprintf(cmd, CMDMAXLEN,
                "smime-tool -verify -cert %s -ca %s %s > %s.prcs",
                conf.vrfy_rules[r].cert_path, conf.vrfy_rules[r].cacert_path,
                fns[m], fns[m]);

            ret = system(cmd);
            snprintf(cmd, CMDMAXLEN, "%s.prcs", fns[m]);

            if (0 == ret) {
                if (0 == rename(cmd, fns[m])) {
                    free_mail_object(mails[m]);
                    load_mail_from_file(fns[m], mails[m]);
                    /* verification successful */
                }
            }
            remove(cmd);
        }
        /** end of verification rules **/
    }

    return 0;
}

void unsent_service (void)
{
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    for (;;) {
        if (-1 == send_mails_from_dir(DEFAULT_UNSENT_DIR, &(conf.mail_srv)) )
            err_sys("failed to open unsent directory");

        sleep(UNSENT_SLEEP);
    }
}

