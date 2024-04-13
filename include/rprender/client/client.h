
#ifndef _RPRENDER_CLIENT_H_
#define _RPRENDER_CLIENT_H_



#include "rprender/rprender.h"



extern int EXPORT_FD;
extern int COMMAND_FD;




int RPCL_init_connection(char* addr);


int RPCL_get_export(char* cmd, uint8_t* read_bytes);


int RPCL_send_command(char* cmd, uint8_t* req, uint8_t* response);


void RPCL_log_clientln(char* log);

void RPCL_get_current_time_string(char* tstr);

#endif