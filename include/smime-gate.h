/**
 * File:        include/smime-gate.h
 * Description: S/MIME Gate client service.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __SMIME_GATE_H
#define __SMIME_GATE_H

void smime_gate_service (int sockfd);
void unsent_service (void);
void* child_process_guard (void* arg);


#endif  /* __SMIME_GATE_H */

