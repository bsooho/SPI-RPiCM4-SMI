#include "rprender/client/client.h"


int CLIENT_FD = 0;


int RPCL_init_connection(char* addr){


    int status, valread, client_fd;
    int connection_result;

    struct sockaddr_un servaddr;


    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {

        RPCL_log_clientln("socket creation error");

        return -1;
    }


    connection_result = connect(client_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    if (connection_result == -1) {
        
        RPCL_log_clientln("conenction failed");
        
        return -2;
    }


    CLIENT_FD = client_fd;

    RPCL_log_clientln("connection established to socket");

    return 0;
}


int RPCL_get_spi0(char* cmd, uint8_t* read_bytes){

    int bytes_total_len = 0;







    return bytes_total_len;
}



int RPCL_send_spi1(){





    return 0;
}




void RPCL_log_clientln(char* log){

#if DEBUG_LEVEL == 0

    return;

#endif 

    char log_str[MAX_LOG_TXT_LEN] = {0};

    char time_str[MAX_TIMESTR_LEN] = {0};

    RPCL_get_current_time_string(time_str);

    sprintf(log_str, "[ %s ] %s\n", time_str, log);

    FILE *fp = fopen("log/client.txt", "a");

    fputs(log_str, fp);

    fflush(fp);

    fclose(fp);


}



void RPCL_get_current_time_string(char* tstr){

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

