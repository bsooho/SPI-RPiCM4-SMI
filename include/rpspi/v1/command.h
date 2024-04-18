#ifndef _RPSPI_V1_COMMAND_H_
#define _RPSPI_V1_COMMAND_H_



/*

    @INCLUDE
*/



#include "rpspi/v1/stream.h"




/*

    @DATA
*/



typedef struct COMMAND_CONTROL {

    int SOCK_ALIVE;

    int sock_fd;
    char* sock_name;
    int client_fd;


} COMMAND_CONTROL;



/*
    @VARIABLE
*/



extern COMMAND_CONTROL cc;


/*

    @FUNCTION
*/

RS_CODE RS_command_main(pthread_t *tid);

void* RS_command_controller(void* targ);

RS_CODE RS_process_command();

RS_CODE RS_roundtrip_to_flag_set_cmd(char* cmd, uint8_t* req, uint32_t* resp_len, uint8_t* response);


#endif