#include "rpspi/v1/utils.h"



void RS_get_current_time_string(char* tstr){

    struct tm *info;
    
    int millisec;
    char msec_str[5] = {0};

    struct timeval tv;


    gettimeofday(&tv, NULL);


    millisec = tv.tv_usec / 1000;

    if (millisec >= 1000){

        millisec -= 1000;
        tv.tv_sec += 1;

    }




    info = localtime(&tv.tv_sec);

    strftime(tstr, MAX_TIMESTR_LEN, "%Y-%m-%d %H:%M:%S", info);

    sprintf(msec_str, ".%03d", millisec);

    strcat(tstr, msec_str);


}



void RS_log_println(char* log){



    char time_str[MAX_TIMESTR_LEN] = {0};

    RS_get_current_time_string(time_str);

    printf("[ %s ] ", time_str);

    printf("%s \n", log);



}


void RS_log_txtln(char* log){

#if DEBUG_LEVEL == 0

    return;

#endif 

    char log_str[MAX_LOG_TXT_LEN] = {0};

    char time_str[MAX_TIMESTR_LEN] = {0};

    RS_get_current_time_string(time_str);

    sprintf(log_str, "[ %s ] %s\n", time_str, log);

    FILE *fp = fopen("log/log.txt", "a");

    fputs(log_str, fp);

    fflush(fp);
}


void RS_log_sockln(char* log){

#if DEBUG_LEVEL == 0

    return;

#endif 

    char log_str[MAX_LOG_TXT_LEN] = {0};

    char time_str[MAX_TIMESTR_LEN] = {0};

    RS_get_current_time_string(time_str);

    sprintf(log_str, "[ %s ] %s\n", time_str, log);

    FILE *fp = fopen("log/sock.txt", "a");

    fputs(log_str, fp);

    fflush(fp);
}


void RS_stringify_array(char* strarray, int arr_len , uint64_t* arr){


    strcat(strarray, "[ ");


    for (int i = 0 ; i < arr_len; i ++){


        char tmp_el[24] = {0};

        sprintf(tmp_el, "%llu, ", arr[i]);

        strcat(strarray, tmp_el);

    }


    strcat(strarray, " ]");


}

void RS_stringify_array_u8(char* strarray, int arr_len , uint8_t* arr){


    strcat(strarray, "[ ");


    for (int i = 0 ; i < arr_len; i ++){


        char tmp_el[24] = {0};

        sprintf(tmp_el, "%u, ", arr[i]);

        strcat(strarray, tmp_el);

    }


    strcat(strarray, " ]");


}


void RS_msleep(long ms){

    struct timespec ts;
    int res;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&ts, &ts);
}