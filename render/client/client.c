#include "rprender/client/client.h"


int EXPORT_FD = 0;
int COMMAND_FD = 0;


int RPCL_init_connection(char* addr){


    int status, valread, client_fd;
    int connection_result;

    struct sockaddr_un servaddr;


    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {

        RPCL_log_clientln("socket creation error");

        return -1;
    }


    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, addr);

    connection_result = connect(client_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    if (connection_result == -1) {
        
        RPCL_log_clientln("conenction failed");
        
        return -2;
    }

    RPCL_log_clientln("connection established to socket");

    return client_fd;
}


int RPCL_get_export(char* cmd, uint8_t* read_bytes){

    int bytes_total_len = 0;

    int val_write = 0;

    int val_read = 0;

    uint8_t flag_set[FLAG_SET] = {0};

    int cmd_buff_len = strlen(cmd);

    if (strcmp(cmd, "1") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_1_BYTE_LEN] = {0};

        val_write = write(EXPORT_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd 1");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        readb = read(EXPORT_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("cmd 1 flag not set");
            return -1;

        }
        

        
        readb = 0;

        while(val_read < CMD_TYPE_1_BYTE_LEN){

            memset(read_buff, 0,  CMD_TYPE_1_BYTE_LEN * sizeof(uint8_t));

            readb = read(EXPORT_FD, read_buff, CMD_TYPE_1_BYTE_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd 1");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                read_bytes[idx] = read_buff[i];


            }

            val_read += readb;

        }
        


    } else if (strcmp(cmd, "2") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_2_BYTE_LEN] = {0};

        val_write = write(EXPORT_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd 2");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        readb = read(EXPORT_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[1] != 1){

            RPCL_log_clientln("cmd 2 flag not set");
            return -1;

        }
        


        readb = 0;

        while(val_read < CMD_TYPE_2_BYTE_LEN){

            memset(read_buff, 0, CMD_TYPE_2_BYTE_LEN * sizeof(uint8_t));

            readb = read(EXPORT_FD, read_buff, CMD_TYPE_2_BYTE_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd 2");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                read_bytes[idx] = read_buff[i];


            }

            val_read += readb;

        }


    } else if (strcmp(cmd, "3") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_3_BYTE_LEN] = {0};

        val_write = write(EXPORT_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd 3");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        readb = read(EXPORT_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[2] != 1){
            RPCL_log_clientln("cmd 3 flag not set");
            return -1;

        }
        


        readb = 0;


        while(val_read < CMD_TYPE_3_BYTE_LEN){

            memset(read_buff, 0, CMD_TYPE_3_BYTE_LEN * sizeof(uint8_t));

            readb = read(EXPORT_FD, read_buff, CMD_TYPE_3_BYTE_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd 3");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                read_bytes[idx] = read_buff[i];


            }

            val_read += readb;

        }


        
    } else {

        RPCL_log_clientln("invalid cmd for export: ");
        RPCL_log_clientln(cmd);

        return -10;
    }



    bytes_total_len = val_read;


    return bytes_total_len;
}



int RPCL_send_command(char* cmd, uint8_t* req, uint8_t* response){


    int response_total_len = 0;

    int val_write = 0;

    int val_read = 0;

    uint8_t flag_set[FLAG_SET] = {0};

    int cmd_buff_len = strlen(cmd);

    if (strcmp(cmd, "XYZ") == 0){


        int readb = 0;

        uint8_t read_buff[CMD_TYPE_XYZ_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd xyz");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, COMMAND_READ_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd xyz req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("cmd xyz flag failed");
            return -1;

        }
        
        readb = 0;


        while(val_read < CMD_TYPE_XYZ_LEN){

            memset(read_buff, 0, CMD_TYPE_XYZ_LEN * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, CMD_TYPE_XYZ_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd xyz");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];


            }

            val_read += readb;

        }


    } else if (strcmp(cmd, "IIR") == 0){


        int readb = 0;

        uint8_t read_buff[CMD_TYPE_IIR_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd iir");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, COMMAND_READ_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd iir req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }


        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("cmd iir flag failed");
            return -1;

        }
        
        readb = 0;


        while(val_read < CMD_TYPE_IIR_LEN){

            memset(read_buff, 0, CMD_TYPE_IIR_LEN * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, CMD_TYPE_IIR_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd iir");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];


            }

            val_read += readb;

        }

    } else if (strcmp(cmd, "COM") == 0){


        int readb = 0;

        uint8_t read_buff[CMD_TYPE_COM_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd com");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, COMMAND_READ_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd com req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }


        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("cmd com flag failed");
            return -1;

        }
        
        readb = 0;


        while(val_read < CMD_TYPE_COM_LEN){

            memset(read_buff, 0, CMD_TYPE_COM_LEN * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, CMD_TYPE_COM_LEN * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read cmd com");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];


            }

            val_read += readb;

        }


    } else if (strcmp(cmd, "XYZUP") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_COM_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd xyz update");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, CMD_TYPE_XYZ_LEN * sizeof(uint8_t));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd xyz update req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }


        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("flag failed");

        }
        
        readb = 0;

        int resp_len = flag_set[1];

        while(val_read < resp_len){

            memset(read_buff, 0, resp_len * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, resp_len * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read xyz update");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];

            }

            val_read += readb;

        }

    } else if (strcmp(cmd, "IIRUP") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_COM_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd iir update");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, CMD_TYPE_IIR_LEN * sizeof(uint8_t));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd iir update req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }


        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("flag failed");

        }
        
        readb = 0;

        int resp_len = flag_set[1];

        while(val_read < resp_len){

            memset(read_buff, 0, resp_len * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, resp_len * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read iir update");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];

            }

            val_read += readb;

        }



    } else if (strcmp(cmd, "COMUP") == 0){

        int readb = 0;

        uint8_t read_buff[CMD_TYPE_COM_LEN] = {0};

        val_write = write(COMMAND_FD, cmd, CMD_BUFF_LEN * sizeof(char));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd com update");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }

        val_write = write(COMMAND_FD, req, CMD_TYPE_COM_LEN * sizeof(uint8_t));

        if (val_write <=0){

            RPCL_log_clientln("failed to send cmd com update req");
            RPCL_log_clientln("val_write <= 0");
            return -1;

        }


        readb = read(COMMAND_FD, flag_set, FLAG_SET * sizeof(uint8_t));
        
        if (readb <= 0){

            RPCL_log_clientln("failed to read flag");
            RPCL_log_clientln("readb <= 0");
            return -1;

        }


        if(flag_set[0] != 1){

            RPCL_log_clientln("flag failed");

        }
        
        readb = 0;

        int resp_len = flag_set[1];

        while(val_read < resp_len){

            memset(read_buff, 0, resp_len * sizeof(uint8_t));

            readb = read(COMMAND_FD, read_buff, resp_len * sizeof(uint8_t));

            if (readb <=0){

                RPCL_log_clientln("failed to read com update");
                RPCL_log_clientln("readb <= 0");
                return -2;

            }


            for (int i = 0 ; i < readb; i ++){

                int idx = i + val_read;

                response[idx] = read_buff[i];

            }

            val_read += readb;

        }



    } else {

        RPCL_log_clientln("invalid cmd for command: ");
        RPCL_log_clientln(cmd);

        return -10;

    }

    response_total_len = val_read;


    return response_total_len;
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


void RPCL_stringify_array_u8(char* strarray, int arr_len , uint8_t* arr){


    strcat(strarray, "[ ");


    for (int i = 0 ; i < arr_len; i ++){


        char tmp_el[24] = {0};

        sprintf(tmp_el, "%u, ", arr[i]);

        strcat(strarray, tmp_el);

    }


    strcat(strarray, " ]");


}

void RPCL_msleep(long ms){

    struct timespec ts;
    int res;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&ts, &ts);
}