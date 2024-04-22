
#ifndef _RPCLIENT_CORE_H_
#define _RPCLIENT_CORE_H_



#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>







#include "rp_glob.h"




typedef struct RENDER_VARIABLE{

    double bf_data[BF_DATA_X][BF_DATA_Y];
    double rms_data[RMS_DATA];
    double mic_data[MIC_DATA];
    double bf_mic_data[MIC_DATA];

    int IS_SET_BPF;
    int IS_IIR_COEF_20_30;
    int MIC_SEL;
    int TEST_ENABLE;
    int TEST_FREQ;
    int IS_MIC_AFE;
    int GAIN;



} RENDER_VARIABLE;



extern RENDER_VARIABLE rv;


void RPCL_example_cmd_type_1(char* test);

void RPCL_example_cmd_type_2(char* test);

void RPCL_example_cmd_type_3(char* test);

void RPCL_example_update_xyz(char* test);

void RPCL_example_read_xyz(char* test);

void RPCL_example_update_iircoef(char* test);

void RPCL_example_read_iircoef(char* test);

void RPCL_example_update_common(char* test);

void RPCL_example_read_common(char* test);




extern int EXPORT_FD;
extern int COMMAND_FD;



int RPCL_init_connection(char* addr);


int RPCL_get_export(char* cmd, uint8_t* read_bytes);


int RPCL_send_command(char* cmd, uint8_t* req, uint8_t* response);


void RPCL_log_clientln(char* log);

void RPCL_get_current_time_string(char* tstr);

void RPCL_stringify_array_u8(char* strarray, int arr_len, uint8_t* arr);

void RPCL_msleep(long ms);



#endif