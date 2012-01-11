/**
 * File:        include/config.h
 * Description: Header file containing declarations of function parsing
 *              command-line arguments and loading configuration.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <stdint.h>


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

    uint16_t smtp_port;     /* SMTP port */
};

/* struct out_rule - outgoing encryption/signing rule */
struct out_rule {
    char *sender;       /* mail object sender */
    char *out_domain;   /* destination domain */
    int action;         /* sign, encrypt or both? */
    char *key_path;     /* cryptographic key location */
};

/* struct out_rule - outgoing encryption/signing rule */
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

