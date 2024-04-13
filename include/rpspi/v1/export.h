#ifndef _RPSPI_V1_EXPORT_H_
#define _RPSPI_V1_EXPORT_H_



/*

    @INCLUDE
*/



#include "rpspi/v1/stream.h"




/*

    @DATA
*/

typedef struct EXPORT_CONTROL {

    int SOCK_ALIVE;

    int sock_fd;
    char* sock_name;
    int client_fd;


    uint32_t rdata_len;
    uint8_t rdata[EC_MAX_RDATA_LEN];
    uint32_t rdata_ex_len;
    uint64_t rdata_ex[EC_MAX_RDATA_LEN];
    double bf_data_ex[BF_DATA_X][BF_DATA_Y];
    double rms_data_ex[RMS_DATA];
    double mic_data_ex[MIC_DATA];
    double bf_mic_data_ex[MIC_DATA];
    int flag_bf_ex;
    int flag_mic_ex;


} EXPORT_CONTROL;



/*
    @VARIABLE
*/

// 0: bf
// 1: mic
// ...

extern uint8_t flag_set[FLAG_SET];

extern EXPORT_CONTROL ec;


/*

    @FUNCTION
*/

RS_CODE RS_export_main(pthread_t *tid);

RS_CODE RS_rdata_export();

RS_CODE RS_rdata2_export();

RS_CODE RS_interpret_rdata_export();

RS_CODE RS_interpret_rdata2_export();

void* RS_export_controller(void* targ);

RS_CODE RS_process_request();

RS_CODE RS_response_rdata_ex_raw(uint32_t* resp_len, uint64_t* response);



RS_CODE RS_response_rdata_raw(uint32_t* resp_len, uint8_t* response);


RS_CODE RS_response_by_flag_set(uint32_t* resp_len, double* response);


#endif