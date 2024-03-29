#include "rpspi/v1/stream.h"
#include "rpspi/v1/export.h"
#include "rpspi/v1/utils.h"

EXPORT_CONTROL ec;

uint8_t flag_set[FLAG_SET] = {0};



// 시작할 때 생성되는 
// exporter 스레드


RS_CODE RS_export_main(pthread_t *tid, pthread_mutex_t *tmtx){


    int status = pthread_create(tid, NULL, RS_export_controller, NULL);

    if(status != 0){

        return RS_FAIL;

    }


    return RS_OKAY;
}


// minor version 1
// copy

RS_CODE RS_rdata_export(){


    ec.rdata_ex_len = 0;

    ec.rdata_len = 0;

    memset(ec.rdata_ex, 0, EC_MAX_RDATA_LEN * sizeof(uint64_t));

    memset(ec.rdata, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));

    for (int i = 0; i < gv.rdata_len; i ++){

        ec.rdata_ex[i] = (uint64_t)gv.rdata[i];

        ec.rdata[i] = gv.rdata[i];

        ec.rdata_ex_len += 1;

        ec.rdata_len += 1;

    }


    return RS_OKAY;
}


// minor version 2
// copy

RS_CODE RS_rdata2_export(){


    ec.rdata_ex_len = 0;

    ec.rdata_len = 0;

    memset(ec.rdata_ex, 0, EC_MAX_RDATA_LEN * sizeof(uint64_t));

    memset(ec.rdata, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));

    for (int i = 0; i < gv.rdata2_len; i ++){

        ec.rdata_ex[i] = (uint64_t)gv.rdata2[i];

        ec.rdata[i] = gv.rdata2[i];

        ec.rdata_ex_len += 1;

        ec.rdata_len += 1;

    }


    return RS_OKAY;
}


// 이 부분을 남겨둘 필요가 없음
// 리팩터링시 제거 대상 1 순위
// 처음 copy 하는 부분만
// 매크로로 처리 하면 될 듯

RS_CODE RS_interpret_rdata_export(){

    pthread_mutex_lock(&tmtx);

    if(RS_rdata_export() != RS_OKAY){

        return RS_FAIL;

    }

    int rlen = ec.rdata_ex_len;

    int wlen = rlen / RS_WORD;

    printf("ex rlen: %d\n", rlen);

    // head

    int head = (ec.rdata_ex[0] << 16) | (ec.rdata_ex[1] << 8) | (ec.rdata_ex[2]);

    // count


    int cnt_now =  (ec.rdata_ex[3] << 16) | (ec.rdata_ex[4]<< 8) | (ec.rdata_ex[5]);

    int cnt_diff = cnt_now - gv.cnt_pre;

    gv.cnt_pre = cnt_now;

    // csum from FPGA

    uint64_t csum_0 = ec.rdata_ex[ec.rdata_ex_len - 1];

    uint64_t csum_1 = ec.rdata_ex[ec.rdata_ex_len - 2];

    uint64_t csum_2 = ec.rdata_ex[ec.rdata_ex_len - 3];

    long long csum = (csum_2 << 16) | (csum_1 << 8) | csum_0 ;

    // csum from received data

    long long csum_cal = 0;

    for (int k = 0 ; k < wlen - 1; k++){

        long long word = (long long)((ec.rdata_ex[k * 3] << 16) | (ec.rdata_ex[k*3+1]<< 8) | ec.rdata_ex[k*3+2]);

        csum_cal = csum_cal + word;
    }



    csum_cal = csum_cal % POW_2_24;
    
    int csum_check;

    if(csum == csum_cal){
        csum_check = TRUE;
    } else {
        csum_check = FALSE;
    }


    // display result

    printf("count %d (%d)\n", cnt_now, cnt_diff);

    uint64_t first_15[15] = {0};
    uint64_t last_15[15] = {0};

    for (int i = 0 ; i < 15; i ++){

        first_15[i] = ec.rdata_ex[i];

        last_15[14 - i] = ec.rdata_ex[ec.rdata_ex_len - 1 - i];


    }

    char str_first15[5120] = {0};
    char str_last15[5120] = {0};

    RS_stringify_array(str_first15, 15, first_15);

    RS_stringify_array(str_last15, 15, last_15);

    printf("rdata length = %d: %s %s\n", rlen, str_first15, str_last15);

    printf("head %x csum %llx  csum cal %llx (%d)\n", head, csum, csum_cal, csum_check);

    // BF RMS data
    if (gv.CMD_TYPE == 1){

        int rn = 29;
        int cn = 0;

        for (int k = 0; k < BF_DATA_LEN; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 40) | ((ec.rdata_ex[k*6+7]) << 32) | ((ec.rdata_ex[k*6+8]) << 24) | ((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            ec.bf_data_ex[rn][cn] = tmp;

            rn = rn - 1;

            if (rn == -1){

                rn = 29;
                cn = cn + 1;
            }
        }


        for (int k = 0 ; k < RMS_DATA; k++){


            double tmp = (double)(((ec.rdata_ex[k*6+7206]) << 40) | ((ec.rdata_ex[k*6+7207]) << 32) | ((ec.rdata_ex[k*6+7208]) << 24) | ((ec.rdata_ex[k*6+7209]) << 16) | ((ec.rdata_ex[k*6+7210]) << 8) | ec.rdata_ex[k*6+7211]);


            ec.rms_data_ex[k] = tmp;

        }

        if(csum_check == TRUE){

            ec.flag_bf_ex = 1;

            flag_set[0] = 1;
        
        }



    } else if (gv.CMD_TYPE == 2) {
        
        for (int k = 0; k < MIC_DATA; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 16) | ((ec.rdata_ex[k*6+7]) << 8) | ec.rdata_ex[k*6+8]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.mic_data_ex[k] = tmp;

            tmp = (double)(((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.bf_mic_data_ex[k] = tmp;


        }

        if (csum_check == TRUE){
            ec.flag_mic_ex = 1;

            flag_set[1] = 1;
        }



    }

    pthread_mutex_unlock(&tmtx);

    return RS_OKAY;
}


// minor version 2

RS_CODE RS_interpret_rdata2_export(){

    pthread_mutex_lock(&tmtx);

    if(RS_rdata2_export() != RS_OKAY){

        return RS_FAIL;

    }

    int rlen = ec.rdata_ex_len;

    int wlen = rlen / RS_WORD;

    printf("ex rlen: %d\n", rlen);

    // head

    int head = (ec.rdata_ex[0] << 16) | (ec.rdata_ex[1] << 8) | (ec.rdata_ex[2]);

    // count


    int cnt_now =  (ec.rdata_ex[3] << 16) | (ec.rdata_ex[4]<< 8) | (ec.rdata_ex[5]);

    int cnt_diff = cnt_now - gv.cnt_pre;

    gv.cnt_pre = cnt_now;

    // csum from FPGA

    uint64_t csum_0 = ec.rdata_ex[ec.rdata_ex_len - 1];

    uint64_t csum_1 = ec.rdata_ex[ec.rdata_ex_len - 2];

    uint64_t csum_2 = ec.rdata_ex[ec.rdata_ex_len - 3];

    long long csum = (csum_2 << 16) | (csum_1 << 8) | csum_0 ;

    // csum from received data

    long long csum_cal = 0;

    for (int k = 0 ; k < wlen - 1; k++){

        long long word = (long long)((ec.rdata_ex[k * 3] << 16) | (ec.rdata_ex[k*3+1]<< 8) | ec.rdata_ex[k*3+2]);

        csum_cal = csum_cal + word;
    }



    csum_cal = csum_cal % POW_2_24;
    
    int csum_check;

    if(csum == csum_cal){
        csum_check = TRUE;
    } else {
        csum_check = FALSE;
    }


    // display result

    printf("count %d (%d)\n", cnt_now, cnt_diff);

    uint64_t first_15[15] = {0};
    uint64_t last_15[15] = {0};

    for (int i = 0 ; i < 15; i ++){

        first_15[i] = ec.rdata_ex[i];

        last_15[14 - i] = ec.rdata_ex[ec.rdata_ex_len - 1 - i];


    }

    char str_first15[5120] = {0};
    char str_last15[5120] = {0};

    RS_stringify_array(str_first15, 15, first_15);

    RS_stringify_array(str_last15, 15, last_15);

    printf("rdata length = %d: %s %s\n", rlen, str_first15, str_last15);

    printf("head %x csum %llx  csum cal %llx (%d)\n", head, csum, csum_cal, csum_check);

    if (csum_check != TRUE){
        monitor_csum_fail += 1;
    }

    // BF RMS data
    if (gv.CMD_TYPE == 1){

        int rn = 29;
        int cn = 0;

        for (int k = 0; k < BF_DATA_LEN; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 40) | ((ec.rdata_ex[k*6+7]) << 32) | ((ec.rdata_ex[k*6+8]) << 24) | ((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            ec.bf_data_ex[rn][cn] = tmp;

            rn = rn - 1;

            if (rn == -1){

                rn = 29;
                cn = cn + 1;
            }
        }


        for (int k = 0 ; k < RMS_DATA; k++){


            double tmp = (double)(((ec.rdata_ex[k*6+7206]) << 40) | ((ec.rdata_ex[k*6+7207]) << 32) | ((ec.rdata_ex[k*6+7208]) << 24) | ((ec.rdata_ex[k*6+7209]) << 16) | ((ec.rdata_ex[k*6+7210]) << 8) | ec.rdata_ex[k*6+7211]);


            ec.rms_data_ex[k] = tmp;

        }

        if(csum_check == TRUE){

            ec.flag_bf_ex = 1;

            flag_set[0] = 1;
        
        }



    } else if (gv.CMD_TYPE == 2) {
        
        for (int k = 0; k < MIC_DATA; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 16) | ((ec.rdata_ex[k*6+7]) << 8) | ec.rdata_ex[k*6+8]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.mic_data_ex[k] = tmp;

            tmp = (double)(((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.bf_mic_data_ex[k] = tmp;


        }

        if (csum_check == TRUE){
            ec.flag_mic_ex = 1;

            flag_set[1] = 1;
        }


    } else if (gv.CMD_TYPE == 3){

        int rn = 29;
        int cn = 0;

        for (int k = 0; k < BF_DATA_LEN; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 40) | ((ec.rdata_ex[k*6+7]) << 32) | ((ec.rdata_ex[k*6+8]) << 24) | ((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            ec.bf_data_ex[rn][cn] = tmp;

            rn = rn - 1;

            if (rn == -1){

                rn = 29;
                cn = cn + 1;
            }
        }


        for (int k = 0 ; k < RMS_DATA; k++){


            double tmp = (double)(((ec.rdata_ex[k*6+7206]) << 40) | ((ec.rdata_ex[k*6+7207]) << 32) | ((ec.rdata_ex[k*6+7208]) << 24) | ((ec.rdata_ex[k*6+7209]) << 16) | ((ec.rdata_ex[k*6+7210]) << 8) | ec.rdata_ex[k*6+7211]);


            ec.rms_data_ex[k] = tmp;

        }

        if(csum_check == TRUE){

            ec.flag_bf_ex = 1;

            flag_set[0] = 1;
        
        }


        for (int k = 0; k < MIC_DATA; k++){

            double tmp = (double)(((ec.rdata_ex[k*6+6]) << 16) | ((ec.rdata_ex[k*6+7]) << 8) | ec.rdata_ex[k*6+8]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.mic_data_ex[k] = tmp;

            tmp = (double)(((ec.rdata_ex[k*6+9]) << 16) | ((ec.rdata_ex[k*6+10]) << 8) | ec.rdata_ex[k*6+11]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            ec.bf_mic_data_ex[k] = tmp;


        }

        if (csum_check == TRUE){
            ec.flag_mic_ex = 1;

            flag_set[1] = 1;
        }


        if(flag_set[0] == 1 && flag_set[1] == 1){
            flag_set[2] = 1;
        }

    }

    pthread_mutex_unlock(&tmtx);

    return RS_OKAY;
}



// 소켓 스레드

void* RS_export_controller(void* targ){


    ec.sock_name = export_sock;

    int server_socket;
    int client_socket;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    server_addr.sun_family = AF_UNIX;

    strcpy(server_addr.sun_path, ec.sock_name);

    int slen = sizeof(server_addr);


    unlink(server_addr.sun_path);

    if(bind(server_socket, (struct sockaddr*)&server_addr, slen) < 0){


        RS_log_sockln("failed to bind");

        return;

    }

    RS_log_sockln("bind");

    chmod(server_addr.sun_path, 0777);


    if(listen(server_socket, SOCK_MAX_CONN) < 0){

        RS_log_sockln("failed to listen");
    
        return;
    }

    RS_log_sockln("listening on: ");

    RS_log_sockln(ec.sock_name);

    ec.sock_fd = server_socket;

    ec.SOCK_ALIVE = TRUE;

    RS_CODE rs_res;

    while(TRUE){

        int clen = sizeof(client_addr);


        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &clen);

        RS_log_sockln("client accepted");

        ec.client_fd = client_socket;


        rs_res = RS_process_request();        


        if(rs_res == RS_FAIL){


            RS_log_sockln("failed to process request");

            close(ec.sock_fd);

            PSTAT = RS_ESOCK;

            return;

        } else if (rs_res == RS_DONE){


            RS_log_sockln("finishing interaction");

            close(ec.sock_fd);

            PSTAT = RS_DONE;

            return;

        }


    }


}


RS_CODE RS_process_request(){


    RS_CODE rs_res;

    while(TRUE){

        int val_read ;

        int val_write;

        char CMD[CMD_BUFF_LEN] = {0};

        val_read = read(ec.client_fd, CMD, CMD_BUFF_LEN * sizeof(char));

        if(val_read <= 0){

            RS_log_sockln("val_read <= 0");

            return RS_OKAY;
        }


        if(strcmp(CMD, "1") == 0){

            RS_log_sockln("CMD: 1");

            uint32_t resp_len = 0;

            // uint8_t response[EC_MAX_RDATA_LEN] = {0};

            double response[MAX_EXPORT_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_response_by_flag_set(&resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type 1 response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(ec.client_fd, response, resp_len * sizeof(double));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                return RS_OKAY;
            }

            RS_log_sockln("resp written");

        } else if(strcmp(CMD, "2") == 0){

            RS_log_sockln("CMD: 2");

            uint32_t resp_len = 0;

            // uint8_t response[EC_MAX_RDATA_LEN] = {0};

            double response[MAX_EXPORT_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_response_by_flag_set(&resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type 2 response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(ec.client_fd, response, resp_len * sizeof(double));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                return RS_OKAY;
            }

            RS_log_sockln("resp written");

        } else if(strcmp(CMD, "3") == 0){

            RS_log_sockln("CMD: 3");

            uint32_t resp_len = 0;

            // uint8_t response[EC_MAX_RDATA_LEN] = {0};

            double response[MAX_EXPORT_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_response_by_flag_set(&resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type 3 response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(ec.client_fd, response, resp_len * sizeof(double));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                return RS_OKAY;
            }

            RS_log_sockln("resp written");

        } else {

            RS_log_sockln("invalid cmd");
            RS_log_sockln(CMD);

            close(ec.client_fd);

            return RS_OKAY;

        }

    }


    return RS_OKAY;
}


RS_CODE RS_response_rdata_ex_raw(uint32_t* resp_len, uint64_t* response){

    pthread_mutex_lock(&tmtx);

    *resp_len = ec.rdata_ex_len;

    memcpy(response, ec.rdata_ex, (*resp_len) * sizeof(uint64_t));


    pthread_mutex_unlock(&tmtx);

    return RS_OKAY;
}


RS_CODE RS_response_rdata_raw(uint32_t* resp_len, uint8_t* response){

    pthread_mutex_lock(&tmtx);


    *resp_len = ec.rdata_len;

    memcpy(response, ec.rdata, (*resp_len) * sizeof(uint8_t));


    pthread_mutex_unlock(&tmtx);

    return RS_OKAY;
}

RS_CODE RS_response_by_flag_set(uint32_t* resp_len, double* response){


    pthread_mutex_lock(&tmtx);


    *resp_len = 0;

    uint8_t flag_set_local[FLAG_SET] = {0};

    memcpy(flag_set_local, flag_set, FLAG_SET * sizeof(uint8_t));



    if(flag_set_local[0] == 1){

        // bf data


        for(int x = 0 ; x < BF_DATA_X; x ++){

            for(int y = 0 ; y < BF_DATA_Y; y ++){

                response[*resp_len] = ec.bf_data_ex[x][y];


                *resp_len += 1;

            }          

        }

        // rms data

        for (int x = 0; x < RMS_DATA; x++){

            response[*resp_len] = ec.rms_data_ex[x];
            
            *resp_len += 1;

        }

        
    } 

    if(flag_set_local[1] == 1){

        // mic data

        for(int x = 0 ; x < MIC_DATA; x++){

            response[*resp_len] = ec.mic_data_ex[x];

            *resp_len += 1;
        }


        // bf mic data

        for(int x = 0 ; x < MIC_DATA; x++){

            response[*resp_len] = ec.bf_mic_data_ex[x];

            *resp_len += 1;
        }


    }




    int val_write = write(ec.client_fd, flag_set_local, FLAG_SET * sizeof(uint8_t));

    if(val_write <= 0){

        RS_log_sockln("failed to send flag set");

        pthread_mutex_unlock(&tmtx);

        return RS_FAIL;

    }

    char flag_str[24] = {0};
    char flag_log[64] = {0};
    RS_stringify_array_u8(flag_str, 4, flag_set_local);

    sprintf(flag_log, "flag: %s", flag_str);

    RS_log_sockln(flag_log);

    pthread_mutex_unlock(&tmtx);



    return RS_OKAY;
}
