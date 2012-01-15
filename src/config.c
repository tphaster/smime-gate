/**
 * File:        src/config.c
 * Description: Functions for parsing command-line arguments and loading
 *              configuration from file.
 * Author:      Tomasz Pieczerak (tphaster)
 */

/* TODO
 *  load_config();
 */

#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "config.h"
#include "system.h"


/* version - print program version and some other information */
static void version (void)
{
    printf("S/MIME Gateway %s\n", conf.version);

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
    printf("Usage: %s [OPTION]...\n\n"
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
           "  -c,  --config=FILE    get config from FILE.\n"
           "  -r,  --rules=FILE     get encryption/signing rules from FILE\n");

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
                    printf("No FILE given in 'config' option.\n");
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
                    printf("No FILE given in 'rules' option.\n");
                    usage();
                    exit(1);
                }

                len = strlen(arg+8)+1;
                conf.rules_file = Malloc(len);
                strncpy(conf.rules_file, arg+8, len);
            }
            else {
                printf("Unknown option.\n");
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
                        printf("No FILE given in 'config' option.\n");
                        usage();
                        exit(1);
                    }

                    arg = *++argv;
                    len = strlen(arg)+1;
                    conf.config_file = Malloc(len);
                    strncpy(conf.config_file, arg, len);
                    break;

                case 'r':       /* -r FILE */
                    /* FILE should be in the next argument */
                    if (--argc <= 0) {
                        printf("No FILE given in 'rules' option.\n");
                        usage();
                        exit(1);
                    }

                    arg = *++argv;
                    len = strlen(arg)+1;
                    conf.rules_file = Malloc(len);
                    strncpy(conf.rules_file, arg, len);
                    break;

                default:        /* unknown argument */
                    printf("Unknown option.\n");
                    usage();
                    exit(1);
            }
        }
        /* bad argument format */
        else {
            printf("Bad argument(s).\n");
            usage();
            exit(1);
        }
    }

    if (0 != argc) {
        printf("Bad arguments.\n");
        usage();
        exit(1);
    }

    /* load default config file, if none was set */
    if (NULL == conf.config_file) {
        len = strlen(DEFAULT_CONFIG_FILE)+1;
        conf.config_file = Malloc(len);
        strncpy(conf.config_file, DEFAULT_CONFIG_FILE, len);
        printf("Using default configuration file '%s'.\n", conf.config_file);
    }
    /* check if set config file is readable */
    if (0 != access(conf.config_file, R_OK)) {
        printf("Can't read '%s' configuration file.\n", conf.config_file);
        usage();
        exit(1);
    }

    /* load default rules file, if none was set */
    if (NULL == conf.rules_file) {
        len = strlen(DEFAULT_RULES_FILE)+1;
        conf.rules_file = Malloc(len);
        strncpy(conf.rules_file, DEFAULT_RULES_FILE, len);
    }
    /* check if set rules file is readable */
    else if (0 != access(conf.rules_file, R_OK)) {
        printf("Can't read %s file (given in 'rules' option).\n", conf.rules_file);
        usage();
        exit(1);
    }
}

/* Rules processing states */
#define FAIL   -1       /* failure */
#define DONE    0       /* done */
#define ADDR    1       /* searching for mail address */
#define DOM     2       /* searching for domain */
#define ACTN    3       /* searching for action */
#define EKEY    4       /* searching for encryption key */
#define SKEY    5       /* searching for sign key */

/* load_config - load configuration from config and rules file */
void load_config (void)
{
    FILE *config, *rules;
    char buf[CONF_MAXLEN], *tok, *beg;
    size_t len, line_cnt, irule_cnt, orule_cnt;
    int state;
    int port;

    /* load program configuration */
    if (NULL == (config = fopen(conf.config_file, "r"))) {
        printf("Can't read '%s' configuration file.\n", conf.config_file);
        usage();
        exit(1);
    }

    line_cnt = 0;

    while (fgets(buf, CONF_MAXLEN, config) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        else if (0 == strncmp("smtp_port = ", buf, 12)) { /* SMTP Port */
            if ((port = atoi(buf+12)) > 0)
                conf.mail_srv.sin_port = htons(port);
            else
                printf("Syntax error in config file on line %d"
                       "-- bad SMTP port (smtp_port).\n", line_cnt);
        }
        else if (0 == strncmp("rules = ", buf, 8)) { /* rules location */
            if (NULL != conf.rules_file)
                free(conf.rules_file);

            len = strlen(buf+8)+1;
            conf.rules_file = Malloc(len);
            strncpy(conf.rules_file, buf+8, len);
            conf.rules_file[len-2] = '\0';
        }
        else if (0 == strncmp("mail_srv = ", buf, 11)) { /* mail server */
            (buf+11)[strlen(buf+11)-1] = '\0';
            if (1 != inet_pton(AF_INET, buf+11, &(conf.mail_srv.sin_addr))) {
                printf("Syntax error in config file on line %d"
                       " - not valid mail server address (mail_srv).\n",
                       line_cnt);
                break;
            }
            else
                conf.mail_srv.sin_family = AF_INET;
        }
        else
            printf("Syntax error in config file on line %d.\n", line_cnt);
    }

    fclose(config);

    /* check whether configuration is complete */
    if (0 == conf.mail_srv.sin_port || 0 == conf.mail_srv.sin_family) {
        printf("Configuration error, mail server address or SMTP port "
               "was not set\n");
        exit(1);
    }

    /* load encryption/signing rules */
    if (NULL == (rules = fopen(conf.rules_file, "r"))) {
        printf("Can't read '%s' rules file.\n", conf.rules_file);
        exit(1);
    }

    line_cnt = 0;
    conf.out_rules_size = 0;
    conf.in_rules_size = 0;

    /* count incoming/outgoing rules */
    while (fgets(buf, CONF_MAXLEN, rules) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        else if (0 == strncmp("IN ", buf, 3))   /* incoming rule */
            conf.in_rules_size += 1;
        else if  (0 == strncmp("OUT ", buf, 4)) /* outgoing rule */
            conf.out_rules_size += 1;
        else
            printf("Syntax error in rules file on line %d.\n", line_cnt);
    }

    /* allocate rule arrays */
    conf.out_rules = Calloc(conf.out_rules_size, sizeof(struct out_rule));
    conf.in_rules = Calloc(conf.in_rules_size, sizeof(struct in_rule));

    rewind(rules);
    line_cnt = 0;
    irule_cnt = 0;
    orule_cnt = 0;

    /* load rules */
    while (fgets(buf, CONF_MAXLEN, rules) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        else if (0 == strncmp("IN ", buf, 3)) {     /* incoming rule */
            size_t i;

            beg = buf+3;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state != SKEY && ' ' == *tok) {
                    *tok = '\0';
                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = FAIL;
                        break;
                    }

                    /* fetching sender address */
                    if (ADDR == state) {
                        conf.in_rules[irule_cnt].sender =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.in_rules[irule_cnt].sender, beg);
                        state = EKEY;   /* now look for encryption key */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching decryption key */
                    else if (EKEY == state) {
                        conf.in_rules[irule_cnt].decrypt_key_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.in_rules[irule_cnt].decrypt_key_path, beg);
                        state = SKEY;   /* now look for sign key */
                        beg = tok+1;
                        continue;
                    }
                }
                /* fetching key for verifying signs */
                else if (SKEY == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = FAIL;
                        break;
                    }

                    conf.in_rules[irule_cnt].vsign_key_path =
                        Malloc(strlen(beg)+1);
                    strcpy(conf.in_rules[irule_cnt].vsign_key_path, beg);

                    /* all needed info fetched, we're done */
                    conf.in_rules[irule_cnt].ok = 1;
                    state = DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = FAIL;
                    break;
                }
            }

            if (DONE == state)
                ++irule_cnt;
            else {
                printf("Syntax error in rules file on line %d.\n", line_cnt);

                if (NULL != conf.in_rules[irule_cnt].sender)
                    free(conf.in_rules[irule_cnt].sender);
                if (NULL != conf.in_rules[irule_cnt].vsign_key_path)
                    free(conf.in_rules[irule_cnt].vsign_key_path);
                if (NULL != conf.in_rules[irule_cnt].decrypt_key_path)
                    free(conf.in_rules[irule_cnt].decrypt_key_path);
                bzero(conf.in_rules+irule_cnt, sizeof(struct in_rule));
            }
        }
        else if (0 == strncmp("OUT ", buf, 4)) {   /* outgoing rule */
            size_t i;

            beg = buf+4;
            tok = beg;
            len = strlen(tok);

            for (i = 0, state = ADDR; i < len; ++i, ++tok) {

                /* space is a separator */
                if (state != SKEY && ' ' == *tok) {
                    *tok = '\0';
                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = FAIL;
                        break;
                    }

                    /* fetching sender address */
                    if (ADDR == state) {
                        conf.out_rules[orule_cnt].sender =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.out_rules[orule_cnt].sender, beg);
                        state = DOM;    /* now look for remote domain */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching remote domain name */
                    else if (DOM == state) {
                        conf.out_rules[orule_cnt].out_domain =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.out_rules[orule_cnt].out_domain, beg);
                        state = ACTN;   /* now look for action type */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching action type */
                    else if (ACTN == state) {
                        if (0 == strncmp("ENCRYPT", beg, strlen(beg)))
                            conf.out_rules[orule_cnt].action = ACTION_ENCRYPT;
                        else if (0 == strncmp("SIGN", beg, strlen(beg)))
                            conf.out_rules[orule_cnt].action = ACTION_SIGN;
                        else if (0 == strncmp("BOTH", beg, strlen(beg)))
                            conf.out_rules[orule_cnt].action = ACTION_BOTH;
                        else {
                            state = FAIL;
                            break;
                        }

                        state = EKEY;    /* now look for encryption key */
                        beg = tok+1;
                        continue;
                    }
                    /* fetching encryption key */
                    else if (EKEY == state) {
                        if (ACTION_SIGN != conf.out_rules[orule_cnt].action) {
                            conf.out_rules[orule_cnt].encrypt_key_path =
                                Malloc(strlen(beg)+1);
                            strcpy(
                              conf.out_rules[orule_cnt].encrypt_key_path, beg);
                        }

                        state = SKEY;   /* now look for sign key */
                        beg = tok+1;
                        continue;
                    }
                }
                else if (SKEY == state && '\n' == *tok) {
                    *tok = '\0';

                    /* field should contain some data... */
                    if (0 == strlen(beg)) {
                        state = FAIL;
                        break;
                    }

                    if (ACTION_ENCRYPT != conf.out_rules[orule_cnt].action) {
                        conf.out_rules[orule_cnt].sign_key_path =
                            Malloc(strlen(beg)+1);
                        strcpy(conf.out_rules[orule_cnt].sign_key_path, beg);
                    }

                    /* all needed info fetched, we're done */
                    state = DONE;
                    break;
                }
                /* reached end of line before fetching information */
                else if ('\0' == *tok) {
                    state = FAIL;
                    break;
                }
            }

            if (DONE == state)
                ++orule_cnt;
            else {
                printf("Syntax error in rules file on line %d.\n", line_cnt);

                if (NULL != conf.out_rules[orule_cnt].sender)
                    free(conf.out_rules[orule_cnt].sender);
                if (NULL != conf.out_rules[orule_cnt].out_domain)
                    free(conf.out_rules[orule_cnt].out_domain);
                if (NULL != conf.out_rules[orule_cnt].encrypt_key_path)
                    free(conf.out_rules[orule_cnt].encrypt_key_path);
                if (NULL != conf.out_rules[orule_cnt].sign_key_path)
                    free(conf.out_rules[orule_cnt].sign_key_path);
                bzero(conf.out_rules+orule_cnt, sizeof(struct out_rule));
            }

        }
        else
            printf("Syntax error in rules file on line %d.\n", line_cnt);
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
        printf("Mail server:  %s (%s)\n", hp->h_name, inet_ntop(AF_INET,
                &(conf.mail_srv.sin_addr), addr, INET_ADDRSTRLEN));
    }
    else {
        printf("Mail server:  %s\n", inet_ntop(AF_INET,
                &(conf.mail_srv.sin_addr), addr, INET_ADDRSTRLEN));
    }

    printf("SMTP Port:    %d\n\n", ntohs(conf.mail_srv.sin_port));

    printf("Config file:  %s\n", conf.config_file);
    printf("Rules file:   %s\n\n", conf.rules_file);

    printf("Loaded rules:\n");

    /* print rules for outgoing traffic */
    for (i = 0; i < conf.out_rules_size; ++i) {
        if (ACTION_SIGN == conf.out_rules[i].action)
            printf("  Sign mails from %s to %s domain with key %s\n",
                    conf.out_rules[i].sender, conf.out_rules[i].out_domain,
                    conf.out_rules[i].sign_key_path);
        else if (ACTION_ENCRYPT == conf.out_rules[i].action)
            printf("  Encrypt mails from %s to %s domain with key %s\n",
                    conf.out_rules[i].sender, conf.out_rules[i].out_domain,
                    conf.out_rules[i].encrypt_key_path);
        else if (ACTION_BOTH == conf.out_rules[i].action) {
            printf("/ Encrypt mails from %s to %s domain with key %s\n",
                    conf.out_rules[i].sender, conf.out_rules[i].out_domain,
                    conf.out_rules[i].encrypt_key_path);
            printf("\\ Sign mails from %s to %s domain with key %s\n",
                    conf.out_rules[i].sender, conf.out_rules[i].out_domain,
                    conf.out_rules[i].sign_key_path);
        }
    }
    printf("\n");

    /* print rules for incoming traffic */
    for (i = 0; i < conf.in_rules_size; ++i) {
        if (conf.in_rules[i].ok) {
            printf("/ Decrypt mails from %s with key %s\n",
                    conf.in_rules[i].sender,
                    conf.in_rules[i].decrypt_key_path);
            printf("\\ Verify mail signs from %s with key %s\n",
                    conf.in_rules[i].sender,
                    conf.in_rules[i].vsign_key_path);
        }
    }
}

