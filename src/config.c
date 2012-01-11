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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
        printf("Using default configuration file '%s'.", conf.config_data)
    }
    /* check if set config file is readable */
    if (0 != access(conf.config_file, R_OK)) {
        printf("Can't read '%s' configuration file.\n", conf.config_file);
        free(conf.config_file);
        conf.config_file = NULL;
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

/* load_config - load configuration from config and rules file */
void load_config (void)
{
    FILE *config, *rules;
    char buf[CONF_MAXLEN];
    size_t len, line_cnt;

    /* load program configuration */
    if (NULL == conf.config_file ||
        NULL == (config = fopen(conf.config_file, "r")))
    {
        /* load default config */
        conf.smtp_port = DEFAULT_SMTP_PORT;
        printf("Loaded default configuration.\n");
    }
    else {
        line_cnt = 0;

        while (fgets(buf, CONF_MAXLEN, config) != NULL) {
            ++line_cnt;

            if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
                continue;
            else if (0 == strncmp("smtp_port = ", buf, 12))   /* SMTP Port */
                conf.smtp_port = atoi(buf+12);
            else if (0 == strncmp("rules = ", buf, 8)) { /* rules location */
                if (NULL != conf.rules_file)
                    free(conf.rules_file);
                len = strlen(buf+8)+1;
                conf.rules_file = Malloc(len);
                strncpy(conf.rules_file, buf+8, len);
            }
            else
                printf("Syntax error in config file on line %d.\n", line_cnt);
        }

        fclose(config);
    }

    /* load encryption/signing rules */
    if (NULL == (rules = fopen(conf.rules_file, "r"))) {
        printf("Can't read '%s' rules file.\n", conf.rules_file);
        exit(1);
    }

    line_cnt = 0;

    while (fgets(buf, CONF_MAXLEN, config) != NULL) {
        ++line_cnt;

        if ('#' == buf[0] || '\n' == buf[0])  /* comment or empty line */
            continue;
        else if (0 == strncmp("IN ", buf, 3)) {     /* incoming rule */

        }
        else if  (0 == strncmp("OUT ", buf, 4)) {   /* outgoing rule */

        }
        else
            printf("Syntax error in rules file on line %d.\n", line_cnt);
    }

    fclose(rules);
}

