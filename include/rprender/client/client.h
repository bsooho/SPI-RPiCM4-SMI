
#ifndef _RPRENDER_CLIENT_H_
#define _RPRENDER_CLIENT_H_



#include "rprender/rprender.h"




extern int CLIENT_FD;




int RPCL_init_connection(char* addr);


int RPCL_get_spi0(char* cmd, uint8_t* read_bytes);


int RPCL_send_spi1();



void RPCL_log_clientln(char* log);

void RPCL_get_current_time_string(char* tstr);

#endif