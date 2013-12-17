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

/** Constants **/

#define VERSION         "1.0"   /* release version */

#define DEFAULT_CONFIG_FILE     "/etc/smime-gate/config"
#define DEFAULT_RULES_FILE      "/etc/smime-gate/rules"
#define DEFAULT_WORKING_DIR     "/var/run/smime-gate"
#define DEFAULT_UNSENT_DIR      "/var/run/smime-gate/unsent"
#define DEFAULT_SMTP_PORT       587

#define DPREF       "smime-gate-debug: "    /* debug prefix */
#define LPREF       "smime-gate: "          /* log prefix   */

#define CONF_MAXLEN     256     /* maximum line length of config/rules files */


/** Typedefs **/

/* struct config - holds program configuration */
struct config {
    char *prog_name;        /* program name */
    char *version;          /* program version */

    int daemon;             /* is program a daemon? */
    char *config_file;      /* configuration file location */
    char *rules_file;       /* encryption/signing rules file location */

    struct encr_rule* encr_rules;   /* encryption rules */
    size_t encr_rules_size;         /* encryption array size */
    struct sign_rule* sign_rules;   /* signing rules */
    size_t sign_rules_size;         /* signing array size */
    struct decr_rule* decr_rules;   /* decryption rules */
    size_t decr_rules_size;         /* decryption array size */
    struct vrfy_rule* vrfy_rules;   /* verification rules */
    size_t vrfy_rules_size;         /* verification array size */

    struct sockaddr_in mail_srv;    /* mail server address */
    uint16_t smtp_port;             /* listening port */
};

/* struct encr_rule - encryption rule */
struct encr_rule {
    char *rcpt;         /* mail recipient */
    char *cert_path;    /* recipient's certificate location */
};

/* struct sign_rule - signing rule */
struct sign_rule {
    char *sndr;         /* mail sender */
    char *cert_path;    /* sender's certificate location */
    char *key_path;     /* sender's private key location */
    char *key_pass;     /* sender's private key password */
};

/* struct decr_rule - decryption rule */
struct decr_rule {
    char *rcpt;         /* mail recipient */
    char *cert_path;    /* recipient's certificate location */
    char *key_path;     /* recipient's private key location */
    char *key_pass;     /* recipient's private key password */
};

/* struct vrfy_rule - verification rule */
struct vrfy_rule {
    char *sndr;         /* mail sender */
    char *cert_path;    /* sender's certificate location */
    char *cacert_path;  /* CA's certificate location */
};


/** Externs **/
extern struct config conf;  /* global configuration */


/** Functions **/
void parse_args (int argc, char **argv);
void load_config (void);
void print_config (void);
void free_config (void);

#endif  /* __CONFIG_H */

