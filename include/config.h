/**
 * File:        include/config.h
 * Description: Header file containing declarations of function parsing
 *              command-line arguments and loading configuration.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>
#include <netinet/in.h>


/*** Constants ***/

#define VERSION         "0.1"   /* release version */

/* Defaults */
#define DEFAULT_CONFIG_FILE     "./config"
#define DEFAULT_RULES_FILE      "./rules"
#define DEFAULT_SMTP_PORT       587

#define CONF_MAXLEN     256     /* maximum line length of config/rules files */

/* Mail actions */
#define ACTION_NONE         0   /* no action */
#define ACTION_SIGN         1   /* sign mail object */
#define ACTION_ENCRYPT      2   /* encrypt mail object */
#define ACTION_BOTH         3   /* encrypt and sign mail object */


/*** Type Definitions ***/

/* struct config - holds program configuration */
struct config {
    char *prog_name;        /* program name */
    char *version;          /* program version */

    int daemon;             /* is program a daemon? */
    char *config_file;      /* configuration file location */
    char *rules_file;       /* encryption/signing rules file location */

    struct out_rule* out_rules; /* outgoing encryption/signing rules */
    size_t out_rules_size;      /* outgoing rules array size */
    struct in_rule* in_rules;   /* incoming encryption/signing rules */
    size_t in_rules_size;       /* incoming rules array size */

    struct sockaddr_in mail_srv;    /* mail server address */
    uint16_t smtp_port;             /* listening port */
};

/* struct out_rule - outgoing encryption/signing rule */
struct out_rule {
    char *sender;           /* mail object sender */
    char *out_domain;       /* destination domain */
    int action;             /* sign, encrypt or both? */
    char *sign_key_path;    /* signing key location */
    char *encrypt_key_path; /* encryption key location */
};

/* struct in_rule - incoming encryption/signing rule */
struct in_rule {
    int ok;                 /* is rule correct? */
    char *sender;           /* mail object sender */
    char *vsign_key_path;   /* location of key used to verify sign */
    char *decrypt_key_path; /* location of key for decryption */
};


/*** Externs ***/
extern struct config conf;  /* global configuration */


/*** Function prototypes ***/
void parse_args (int argc, char **argv);
void load_config (void);
void print_config (void);

#endif  /* __CONFIG_H */

