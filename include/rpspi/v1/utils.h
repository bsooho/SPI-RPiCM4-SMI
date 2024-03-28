#ifndef _RPSPI_V1_UTILS_H_
#define _RPSPI_V1_UTILS_H_


#include "rpspi/v1/stream.h"







void RS_get_current_time_string(char* tstr);


void RS_log_println(char* log);

void RS_log_txtln(char* log);

void RS_log_sockln(char* log);

void RS_stringify_array(char* strarray, int arr_len, uint64_t* arr);

void RS_stringify_array_u8(char* strarray, int arr_len, uint8_t* arr);


void RS_msleep(long ms);



#endif


