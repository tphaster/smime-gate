#!/bin/sh

CERT_REPO=../files/cert_repo.tar.gz
CONFIG=./config
RULES=./rules

PORT=5555
SERVER_ADDR=192.168.0.101
SERVER_PORT=5780
WORKING_DIR=./var_run_smime-gate

set -e

if [ ! -r $CERT_REPO ]; then
	echo "`basename $0`: cannot read CA archive ($CERT_REPO)" 1<&2
	exit 1
fi

if [ -f $CONFIG ]; then
	read -p "Config file $CONFIG exists. Overwrite? (y/n) " REPLY
	if [ "x$REPLY" = "xy" ]; then
		IFCONFIG=1
	fi
else
	IFCONFIG=1
fi

if [ -f $RULES ]; then
	read -p "Rules file $RULES exists. Overwrite? (y/n) " REPLY
	if [ "x$REPLY" = "xy" ]; then
		IFRULES=1
	fi
else
	IFCONFIG=1
fi

echo "Installing CA..."
rm -rf cert_repo
tar xzf $CERT_REPO

if [ $IFCONFIG ]; then
	echo "Creating config file..."
	cat <<EOT > $CONFIG
# S/MIME Gate configuration file

# SMTP Port, smime-gate will listen on it
smtp_port = $PORT

# rules file location
rules = $RULES

# Mail server address and port
mail_srv_addr = $SERVER_ADDR
mail_srv_port = $SERVER_PORT

EOT
fi

if [ $IFRULES ]; then
	echo "Creating rules file..."
	cat <<EOT > $RULES
# S/MIME Gate rules file

# Encryption rules
# ENCR user@far.com /path/to/user_cert.pem

ENCR userA_e@faar.com ./cert_userA_e.pem
ENCR userB_e@faar.com ./cert_userB_e.pem
ENCR encrypt@example.org ./cert_repo/ca/newcerts/02.pem

# Signing rules
# SIGN user@home.com /path/to/user_cert.pem /path/to/user_key.pem keypassword

SIGN userA_s@home.com ./cert_userA_s.pem ./key_userA_s.pem keypassword
SIGN userB_s@home.com ./cert_userB_s.pem ./key_userB_s.pem keypassword
SIGN sign@example.org ./cert_repo/ca/newcerts/01.pem ./cert_repo/newkey01.pem qwerty

# Decryption rules
# DECR user@home.com /path/to/user_cert.pem /path/to/user_key.pem keypassword

DECR userA_d@home.com ./cert_userA_d.pem ./key_userA_d.pem keypassword
DECR decrypt@example.org ./cert_repo/ca/newcerts/01.pem ./cert_repo/newkey01.pem qwerty
DECR userC_d@home.com ./cert_userC_d.pem ./key_userC_d.pem keypassword

# Verification rules
# VRFY user@far.com /path/to/user_cert.pem /path/to/ca_cert.pem

VRFY verify@example.org ./cert_repo/ca/newcerts/02.pem ./cert_repo/ca/cacert.pem
VRFY userB_v@faar.com ./cert_userB_v.pem ./ca_cert.pem
VRFY userC_v@faar.com ./cert_userA_v.pem ./ca_cert.pem

EOT
fi

echo "Creating working dir and linking to /var/run/smime-gate..."
if [ ! -d $WORKING_DIR ]; then
	mkdir -p $WORKING_DIR
fi

export WORKING_DIR=`readlink -e $WORKING_DIR`
sudo -E bash -c 'rm -f /var/run/smime-gate && ln -s $WORKING_DIR /var/run/smime-gate'

echo "Done."

