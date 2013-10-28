/**
 * File:        src/config.c
 * Description: Functions for parsing command-line arguments and loading
 *              configuration from file.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"
#include "system.h"

/* version - print program version and some other information */
static void version (void)
{
    printf("S/MIME Gateway %s\n\n", conf.version);

    printf("Copyright (C) 2011-2012 Tomasz Pieczerak\n"
           "License GPLv3+: GNU GPL version 3 or later\n"
           "<http://www.gnu.org/licenses/gpl.html>.\n"
           "This is free software: you are free to change and redistribute it.\n"
           "There is NO WARRANTY, to the extent permitted by law.\n\n");

    printf("Written and maintained by Tomasz Pieczerak <tphaster AT gmail.com>.\n"
           "Please send bug reports and questions to <tphaster AT gmail.com>.\n");
}

/* usage - print information about program usage */
static void usage (void)
{
    fprintf(stderr, "Usage: %s [OPTION]...\n\n"
           "Try '%s --help' for more options.\n",
            conf.prog_name, conf.prog_name);
}

/* help - print help information */
static void help (void)
{
    printf("S/MIME Gateway %s, automated mail objects signing/encrypting.\n"
           "Usage: %s [OPTION]...\n\n",
            conf.version, conf.prog_name);

    printf("Mandatory arguments to long options are mandatory for short options too.\n\n");

    printf("Startup:\n"
           "  -V,  --version        display the version of smime-gate and exit.\n"
           "  -h,  --help           print this help.\n"
           "  -d,  --daemon         smime-gate will detach and become a daemon\n");

    printf("Configuration files:\n"
           "  -c FILE,  --config=FILE   get config from FILE.\n"
           "  -r FILE,  --rules=FILE    get encryption/signing rules from FILE\n");

    printf("\nMail bug reports and suggestions to <tphaster AT gmail.com>.\n");
}

/* parse_args - parse command-line arguments, exits on failure */
void parse_args (int argc, char **argv)
{
    size_t len;     /* argument length */
    char *arg;      /* current argument */

    bzero(&conf, sizeof(conf));

    /* get program name */
    arg = basename(argv[0]);
    len = strlen(arg)+1;
    conf.prog_name = Malloc(len);
    strncpy(conf.prog_name, arg, len);

    /* get release version */
    len = strlen(VERSION)+1;
    conf.version = Malloc(len);
    strncpy(conf.version, VERSION, len);

    /* check whether 'smime-tool' is available in PATH */
    if (system("smime-tool --version 1>/dev/null 2>&1")) {
        fprintf(stderr, "There is no 'smime-tool' available in PATH.\n");
        exit(1);
    }

    /* parse command-line arguments */
    while (--argc > 0 && (*++argv)[0] == '-') {
        arg = argv[0];

        /* long options */
        if ('-' == arg[1]) {
            /* --version */
            if (0 == strncmp(arg+2, "version", 7) && '\0' == arg[9]) {
                version();
                exit(0);
            }
            /* --help */
            else if (0 == strncmp(arg+2, "help", 4) && '\0' == arg[6]) {
                help();
                exit(0);
            }
            /* --daemon */
            else if (0 == strncmp(arg+2, "daemon", 6) && '\0' == arg[8]) {
                conf.daemon = 1;
            }
            /* --config=FILE */
            else if (0 == strncmp(arg+2, "config", 6)) {
                if ('=' != arg[8] || '\0' == arg[9]) {
                    fprintf(stderr, "No FILE given in 'config' option.\n");
                    usage();
                    exit(1);
                }

                len = strlen(arg+9)+1;
                conf.config_file = Malloc(len);
                strncpy(conf.config_file, arg+9, len);
            }
            /* --rules=FILE */
            else if (0 == strncmp(arg+2, "rules", 5)) {
                if ('=' != arg[7] || '\0' == arg[8]) {
                    fprintf(stderr, "No FILE given in 'rules' option.\n");
                    usage();
                    exit(1);
                }

                len = strlen(arg+8)+1;
                conf.rules_file = Malloc(len);
                strncpy(conf.rules_file, arg+8, len);
            }
            else {
                fprintf(stderr, "Unknown option.\n");
                usage();
                exit(1);
            }
        }
        /* short options */
        else if ('\0' != arg[1] && '\0' == arg[2]) {
            switch (arg[1]) {
                case 'V':       /* -V */
                    version();
                    exit(0);

                case 'h':       /* -h*/
                    help();
                    exit(0);

                case 'd':       /* -d */
                    conf.daemon = 1;
                    break;

                case 'c':       /* -c FILE */
                    /* FILE should be in the next argument */
                    if (--argc <= 0) {
                        fprintf(stderr, "No FILE given in 'config' option.\n");
                        usage();
                        exit(1);
                    }

                    arg = *++argv;
                    len = strlen(arg)+1;
                    if (NULL != conf.config_file)
                        free(conf.config_file);
                    conf.config_file = Malloc(len);
                    strncpy(conf.config_file, arg, len);
                    break;

                case 'r':       /* -r FILE */
                    /* FILE should be in the next argument */
                    if (--argc <= 0) {
                        fprintf(stderr, "No FILE given in 'rules' option.\n");
                        usage();
                        exit(1);
                    }

                    arg = *++argv;
                    len = strlen(arg)+1;
                    if (NULL != conf.rules_file)
                        free(conf.rules_file);
                    conf.rules_file = Malloc(len);
                    strncpy(conf.rules_file, arg, len);
                    break;

                default:        /* unknown argument */
                    fprintf(stderr, "Unknown option.\n");
                    usage();
                    exit(1);
            }
        }
        /* bad argument format */
        else {
            fprintf(stderr, "Bad argument(s).\n");
            usage();
            exit(1);
        }
    }

    if (0 != argc) {
        fprintf(stderr, "Bad arguments.\n");
        usage();
        exit(1);
    }

    /* load default config file, if none was set */
    if (NULL == conf.config_file) {
        len = strlen(DEFAULT_CONFIG_FILE)+1;
        conf.config_file = Malloc(len);
        strncpy(conf.config_file, DEFAULT_CONFIG_FILE, len);
    }
    /* check if set config file is readable */
    if (0 != access(conf.config_file, R_OK)) {
        fprintf(stderr, "Can't read '%s' configuration file.\n",
                conf.config_file);
        usage();
        exit(1);
    }

    /* check if set rules file is readable (if it was set) */
    else if (NULL != conf.rules_file && 0 != access(conf.rules_file, R_OK)) {
        fprintf(stderr, "Can't read %s file (given in 'rules' option).\n",
                conf.rules_file);
        usage();
        exit(1);
    }

    /* create working directory */
    if (0 != mkdir(DEFAULT_WORKING_DIR, 0700) && EEXIST != errno) {
        fprintf(stderr, "Can't create working directory '%s'.\n",
                DEFAULT_WORKING_DIR);
        exit(1);
    }

    /* create unsent directory */
    if (0 != mkdir(DEFAULT_UNSENT_DIR, 0700) && EEXIST != errno) {
        fprintf(stderr, "Can't create unsent directory '%s'.\n",
                DEFAULT_UNSENT_DIR);
        exit(1);
    }
}

/* Rules processing states */
#define R_FAIL     -1       /* failure */
#define R_DONE      0       /* done */
#define R_ADDR      1       /* searching for mail address */
#define R_CERT      2       /* searching for certificate */
#define R_PKEY      3       /* searching for private key */
#define R_PASS      4       /* searching for key's password */
#define R_CACR      5       /* searching for CA's certificate */

/* load_config - load configuration from config and rules file */
void load_config (void)
{
    FILE *config, *rules;
    char buf[CONF_MAXLEN], *tok, *beg;
    size_t len, line_cnt, erule_cnt, drule_cnt, srule_cnt, vrule_cnt;
    int state;
    int port;

    /*** load program configuration ***/

    if (NULL == (config = fopen(conf.config_file, "r"))) {
        fprintf(stderr, "Can't read '%s' configuration file.\n",
                conf.config_file);
        usage();
        exit(1);
    }

    line_cnt = 0;

    while (fgets(buf, CONF_MAXLEN, config) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        /* SMTP Port */
        else if (0 == strncmp("smtp_port = ", buf, 12)) {
            if ((port = atoi(buf+12)) > 0)
                conf.smtp_port = htons(port);
            else
                fprintf(stderr, "Syntax error in config file on line %d"
                       "-- bad SMTP port (smtp_port).\n", line_cnt);
        }
        /* rules file location */
        else if (0 == strncmp("rules = ", buf, 8)) {
            if (NULL != conf.rules_file)
                continue;

            len = strlen(buf+8)+1;
            conf.rules_file = Malloc(len);
            strncpy(conf.rules_file, buf+8, len);
            conf.rules_file[len-2] = '\0';
        }
        /* mail server address */
        else if (0 == strncmp("mail_srv_addr = ", buf, 16)) {
            (buf+16)[strlen(buf+16)-1] = '\0';
            if (1 != inet_pton(AF_INET, buf+16, &(conf.mail_srv.sin_addr))) {
                fprintf(stderr, "Syntax error in config file on line %d"
                       " - not valid mail server address (mail_srv).\n",
                       line_cnt);
                break;
            }
            else
                conf.mail_srv.sin_family = AF_INET;
        }
        /* mail server port */
        else if (0 == strncmp("mail_srv_port = ", buf, 16)) {
            if ((port = atoi(buf+16)) > 0)
                conf.mail_srv.sin_port = htons(port);
            else
                fprintf(stderr, "Syntax error in config file on line %d"
                       "-- bad mail server port (mail_srv_port).\n", line_cnt);
        }

        else
            fprintf(stderr, "Syntax error in config file on line %d.\n",
                    line_cnt);
    }

    fclose(config);

    /* check whether configuration is complete */
    if (0 == conf.mail_srv.sin_port || 0 == conf.mail_srv.sin_family) {
        fprintf(stderr, "Configuration error, mail server address or port "
               "was not set\n");
        exit(1);
    }
    if (0 == conf.smtp_port) {
        conf.smtp_port = htons(DEFAULT_SMTP_PORT);
        fprintf(stderr, "Loaded default SMTP port as none was set.\n");
    }
    /* load default rules file, if none was set */
    if (NULL == conf.rules_file) {
        len = strlen(DEFAULT_RULES_FILE)+1;
        conf.rules_file = Malloc(len);
        strncpy(conf.rules_file, DEFAULT_RULES_FILE, len);
        fprintf(stderr, "Loaded default SMTP port as none was set.\n");
    }


    /*** load encryption/signing rules ***/

    if (NULL == (rules = fopen(conf.rules_file, "r"))) {
        fprintf(stderr, "Can't read '%s' rules file.\n", conf.rules_file);
        exit(1);
    }

    line_cnt = 0;
    conf.encr_rules_size = 0;
    conf.sign_rules_size = 0;
    conf.decr_rules_size = 0;
    conf.vrfy_rules_size = 0;

    /* count encryption/decryption/signing/verification rules */
    while (fgets(buf, CONF_MAXLEN, rules) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        else if  (0 == strncmp("ENCR ", buf, 5)) /* encryption rule */
            conf.encr_rules_size += 1;
        else if  (0 == strncmp("SIGN ", buf, 5)) /* signing rule */
            conf.sign_rules_size += 1;
        else if  (0 == strncmp("DECR ", buf, 5)) /* decryption rule */
            conf.decr_rules_size += 1;
        else if  (0 == strncmp("VRFY ", buf, 5)) /* verify rule */
            conf.vrfy_rules_size += 1;
        else
            fprintf(stderr, "Syntax error in rules file on line %d.\n",
                    line_cnt);
    }

    /* allocate rule arrays */
    conf.encr_rules = Calloc(conf.encr_rules_size, sizeof(struct encr_rule));
    conf.sign_rules = Calloc(conf.sign_rules_size, sizeof(struct sign_rule));
    conf.decr_rules = Calloc(conf.decr_rules_size, sizeof(struct decr_rule));
    conf.vrfy_rules = Calloc(conf.vrfy_rules_size, sizeof(struct vrfy_rule));

    rewind(rules);
    line_cnt = 0;
    erule_cnt = 0;
    srule_cnt = 0;
    drule_cnt = 0;
    vrule_cnt = 0;

    /* load rules */
    while (fgets(buf, CONF_MAXLEN, rules) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;

        /** encryption rule **/
        else if (0 == strncmp("ENCR ", buf, 5)) {
            size_t i;

            beg = buf+5;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = R_ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state == R_ADDR && ' ' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    /* fetching recipient address */
                    conf.encr_rules[erule_cnt].rcpt = Malloc(strlen(beg)+1);
                    strcpy(conf.encr_rules[erule_cnt].rcpt, beg);
                    state = R_CERT;     /* now look for certificate key */
                    beg = tok+1;
                    continue;
                }
                /* fetching certificate */
                else if (R_CERT == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    conf.encr_rules[erule_cnt].cert_path =
                        Malloc(strlen(beg)+1);
                    strcpy(conf.encr_rules[erule_cnt].cert_path, beg);

                    /* all needed info fetched, we're done */
                    state = R_DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = R_FAIL;
                    break;
                }
            }

            if (R_DONE == state)
                ++erule_cnt;
            else {
                fprintf(stderr, "Syntax error in rules file on line %d.\n",
                        line_cnt);

                if (NULL != conf.encr_rules[erule_cnt].rcpt)
                    free(conf.encr_rules[erule_cnt].rcpt);
                if (NULL != conf.encr_rules[erule_cnt].cert_path)
                    free(conf.encr_rules[erule_cnt].cert_path);
                bzero(conf.encr_rules+erule_cnt, sizeof(struct encr_rule));
            }
        }
        /** signing rule **/
        else if (0 == strncmp("SIGN ", buf, 5)) {
            size_t i;

            beg = buf+5;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = R_ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state != R_PASS && ' ' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    /* fetching sender address */
                    if (R_ADDR == state) {
                        conf.sign_rules[srule_cnt].sndr =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.sign_rules[srule_cnt].sndr, beg);

                        state = R_CERT;     /* now look for certificate */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching sender's certificate */
                    else if (R_CERT == state) {
                        conf.sign_rules[srule_cnt].cert_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.sign_rules[srule_cnt].cert_path, beg);

                        state = R_PKEY;     /* now look for private key */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching private key */
                    else if (R_PKEY == state) {
                        conf.sign_rules[srule_cnt].key_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.sign_rules[srule_cnt].key_path, beg);

                        state = R_PASS; /* now look for private key password */
                        beg = tok+1;
                        continue;
                    }
                }
                /* fetching private key password */
                else if (R_PASS == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    conf.sign_rules[srule_cnt].key_pass =
                        Malloc(strlen(beg)+1);
                    strcpy(conf.sign_rules[srule_cnt].key_pass, beg);

                    /* all needed info fetched, we're done */
                    state = R_DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = R_FAIL;
                    break;
                }
            }

            if (R_DONE == state)
                ++srule_cnt;
            else {
                fprintf(stderr, "Syntax error in rules file on line %d.\n",
                        line_cnt);

                if (NULL != conf.sign_rules[srule_cnt].sndr)
                    free(conf.sign_rules[srule_cnt].sndr);
                if (NULL != conf.sign_rules[srule_cnt].cert_path)
                    free(conf.sign_rules[srule_cnt].cert_path);
                if (NULL != conf.sign_rules[srule_cnt].key_path)
                    free(conf.sign_rules[srule_cnt].key_path);
                if (NULL != conf.sign_rules[srule_cnt].key_pass)
                    free(conf.sign_rules[srule_cnt].key_pass);
                bzero(conf.sign_rules+srule_cnt, sizeof(struct sign_rule));
            }
        }
        /** decryption rule **/
        else if (0 == strncmp("DECR ", buf, 5)) {
            size_t i;

            beg = buf+5;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = R_ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state != R_PASS && ' ' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    /* fetching recipient address */
                    if (R_ADDR == state) {
                        conf.decr_rules[drule_cnt].rcpt =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.decr_rules[drule_cnt].rcpt, beg);

                        state = R_CERT;     /* now look for certificate */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching sender's certificate */
                    else if (R_CERT == state) {
                        conf.decr_rules[drule_cnt].cert_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.decr_rules[drule_cnt].cert_path, beg);

                        state = R_PKEY;     /* now look for private key */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching private key */
                    else if (R_PKEY == state) {
                        conf.decr_rules[drule_cnt].key_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.decr_rules[drule_cnt].key_path, beg);

                        state = R_PASS; /* now look for private key password */
                        beg = tok+1;
                        continue;
                    }
                }
                /* fetching private key password */
                else if (R_PASS == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    conf.decr_rules[drule_cnt].key_pass =
                        Malloc(strlen(beg)+1);
                    strcpy(conf.decr_rules[drule_cnt].key_pass, beg);

                    /* all needed info fetched, we're done */
                    state = R_DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = R_FAIL;
                    break;
                }
            }

            if (R_DONE == state)
                ++drule_cnt;
            else {
                fprintf(stderr, "Syntax error in rules file on line %d.\n",
                        line_cnt);

                if (NULL != conf.decr_rules[drule_cnt].rcpt)
                    free(conf.decr_rules[drule_cnt].rcpt);
                if (NULL != conf.decr_rules[drule_cnt].cert_path)
                    free(conf.decr_rules[drule_cnt].cert_path);
                if (NULL != conf.decr_rules[drule_cnt].key_path)
                    free(conf.decr_rules[drule_cnt].key_path);
                if (NULL != conf.decr_rules[drule_cnt].key_pass)
                    free(conf.decr_rules[drule_cnt].key_pass);
                bzero(conf.decr_rules+drule_cnt, sizeof(struct decr_rule));
            }
        }
        /** verification rule **/
        else if (0 == strncmp("VRFY ", buf, 5)) {
            size_t i;

            beg = buf+5;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = R_ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state != R_CACR && ' ' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    /* fetching sender address */
                    if (R_ADDR == state) {
                        conf.vrfy_rules[vrule_cnt].sndr =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.vrfy_rules[vrule_cnt].sndr, beg);

                        state = R_CERT;     /* now look for certificate */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching sender's certificate */
                    else if (R_CERT == state) {
                        conf.vrfy_rules[vrule_cnt].cert_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.vrfy_rules[vrule_cnt].cert_path, beg);

                        state = R_CACR;     /* now look for CA certificate */
                        beg = tok+1;
                        continue;
                    }
                }
                /* fetching CA certificate */
                else if (R_CACR == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = R_FAIL;
                        break;
                    }

                    conf.vrfy_rules[vrule_cnt].cacert_path =
                        Malloc(strlen(beg)+1);
                    strcpy(conf.vrfy_rules[vrule_cnt].cacert_path, beg);

                    /* all needed info fetched, we're done */
                    state = R_DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = R_FAIL;
                    break;
                }
            }

            if (R_DONE == state)
                ++vrule_cnt;
            else {
                fprintf(stderr, "Syntax error in rules file on line %d.\n",
                        line_cnt);

                if (NULL != conf.vrfy_rules[vrule_cnt].sndr)
                    free(conf.vrfy_rules[vrule_cnt].sndr);
                if (NULL != conf.vrfy_rules[vrule_cnt].cert_path)
                    free(conf.vrfy_rules[vrule_cnt].cert_path);
                if (NULL != conf.vrfy_rules[vrule_cnt].cacert_path)
                    free(conf.vrfy_rules[vrule_cnt].cacert_path);
                bzero(conf.vrfy_rules+vrule_cnt, sizeof(struct vrfy_rule));
            }
        }
        else
            fprintf(stderr, "Syntax error in rules file on line %d.\n",
                    line_cnt);
    }

    fclose(rules);
}

/* print_config - print current global configuration */
void print_config (void)
{
    size_t i;
    char addr[INET_ADDRSTRLEN];
    struct hostent *hp;

    printf("Global configuration\n\n");

    printf("Program name: %s\n"
           "Version:      %s\n\n", conf.prog_name, conf.version);

    if (0 == conf.daemon)
        printf("Daemon mode:  no\n");
    else
        printf("Daemon mode:  yes\n");

    hp = gethostbyaddr(&(conf.mail_srv.sin_addr),
            sizeof(conf.mail_srv.sin_addr), AF_INET);
    if (NULL != hp) {
        printf("Mail server:  %s (%s:%d)\n", hp->h_name, inet_ntop(AF_INET,
                &(conf.mail_srv.sin_addr), addr, INET_ADDRSTRLEN),
                ntohs(conf.mail_srv.sin_port));
    }
    else {
        printf("Mail server:  %s:%d\n", inet_ntop(AF_INET,
                &(conf.mail_srv.sin_addr), addr, INET_ADDRSTRLEN),
                ntohs(conf.mail_srv.sin_port));
    }

    printf("SMTP Port:    %d\n\n", ntohs(conf.smtp_port));

    printf("Config file:  %s\n", conf.config_file);
    printf("Rules file:   %s\n\n", conf.rules_file);

    printf("Loaded rules:\n");

    /* print rules for encryption */
    for (i = 0; i < conf.encr_rules_size; ++i) {
        if (NULL != conf.encr_rules[i].rcpt) {
            printf(" Encrypt mails to %s with certificate %s\n",
                    conf.encr_rules[i].rcpt, conf.encr_rules[i].cert_path);
        }
    }
    printf("\n");

    /* print rules for signing */
    for (i = 0; i < conf.sign_rules_size; ++i) {
        if (NULL != conf.sign_rules[i].sndr) {
            printf(" Sign mails from %s with certificate %s,"
                   " key %s (password: %s)\n", conf.sign_rules[i].sndr,
                    conf.sign_rules[i].cert_path, conf.sign_rules[i].key_path,
                    conf.sign_rules[i].key_pass);
        }
    }
    printf("\n");

    /* print rules for decryption */
    for (i = 0; i < conf.decr_rules_size; ++i) {
        if (NULL != conf.decr_rules[i].rcpt) {
            printf(" Decrypt mails from %s with certificate %s,"
                   " key %s (password: %s)\n", conf.decr_rules[i].rcpt,
                    conf.decr_rules[i].cert_path, conf.decr_rules[i].key_path,
                    conf.decr_rules[i].key_pass);
        }
    }
    printf("\n");

    /* print rules for verification */
    for (i = 0; i < conf.vrfy_rules_size; ++i) {
        if (NULL != conf.vrfy_rules[i].sndr) {
            printf(" Verify mails from %s with certificate %s (CA: %s)\n",
                    conf.vrfy_rules[i].sndr, conf.vrfy_rules[i].cert_path,
                    conf.vrfy_rules[i].cacert_path);
        }
    }
    printf("\n");
}

/* free_config - free global configuration structure */
void free_config (void)
{
    size_t i;

    if (NULL != conf.prog_name)
        free(conf.prog_name);
    if (NULL != conf.version)
        free(conf.version);
    if (NULL != conf.config_file)
        free(conf.config_file);
    if (NULL != conf.rules_file)
        free(conf.rules_file);

    for (i = 0; i < conf.encr_rules_size; ++i) {
        if (NULL != conf.encr_rules[i].rcpt)
            free(conf.encr_rules[i].rcpt);
        if (NULL != conf.encr_rules[i].cert_path)
            free(conf.encr_rules[i].cert_path);
    }

    for (i = 0; i < conf.sign_rules_size; ++i) {
        if (NULL != conf.sign_rules[i].sndr)
            free(conf.sign_rules[i].sndr);
        if (NULL != conf.sign_rules[i].cert_path)
            free(conf.sign_rules[i].cert_path);
        if (NULL != conf.sign_rules[i].key_path)
            free(conf.sign_rules[i].key_path);
        if (NULL != conf.sign_rules[i].key_pass)
            free(conf.sign_rules[i].key_pass);
    }

    for (i = 0; i < conf.decr_rules_size; ++i) {
        if (NULL != conf.decr_rules[i].rcpt)
            free(conf.decr_rules[i].rcpt);
        if (NULL != conf.decr_rules[i].cert_path)
            free(conf.decr_rules[i].cert_path);
        if (NULL != conf.decr_rules[i].key_path)
            free(conf.decr_rules[i].key_path);
        if (NULL != conf.decr_rules[i].key_pass)
            free(conf.decr_rules[i].key_pass);
    }

    for (i = 0; i < conf.vrfy_rules_size; ++i) {
        if (NULL != conf.vrfy_rules[i].sndr)
            free(conf.vrfy_rules[i].sndr);
        if (NULL != conf.vrfy_rules[i].cert_path)
            free(conf.vrfy_rules[i].cert_path);
        if (NULL != conf.vrfy_rules[i].cacert_path)
            free(conf.vrfy_rules[i].cacert_path);
    }

    if (NULL != conf.encr_rules)
        free(conf.encr_rules);
    if (NULL != conf.sign_rules)
        free(conf.sign_rules);
    if (NULL != conf.decr_rules)
        free(conf.decr_rules);
    if (NULL != conf.vrfy_rules)
        free(conf.vrfy_rules);
}

