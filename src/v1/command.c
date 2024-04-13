#include "rpspi/v1/stream.h"
#include "rpspi/v1/command.h"
#include "rpspi/v1/utils.h"


COMMAND_CONTROL cc;


RS_CODE RS_command_main(pthread_t *tid){


    int status = pthread_create(tid, NULL, RS_command_controller, NULL);

    if(status != 0){

        return RS_FAIL;

    }


    return RS_OKAY;
}


void* RS_command_controller(void* targ){


    cc.sock_name = command_sock;

    int server_socket;
    int client_socket;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    server_addr.sun_family = AF_UNIX;

    strcpy(server_addr.sun_path, cc.sock_name);

    int slen = sizeof(server_addr);


    unlink(server_addr.sun_path);

    if(bind(server_socket, (struct sockaddr*)&server_addr, slen) < 0){


        RS_log_sockln("failed to bind for command");

        return;

    }

    RS_log_sockln("bind command sock");

    chmod(server_addr.sun_path, 0777);


    if(listen(server_socket, SOCK_MAX_CONN) < 0){

        RS_log_sockln("failed to listen");
    
        return;
    }

    RS_log_sockln("listening on: ");

    RS_log_sockln(cc.sock_name);

    cc.sock_fd = server_socket;

    cc.SOCK_ALIVE = TRUE;

    RS_CODE rs_res;

    while(TRUE){

        int clen = sizeof(client_addr);


        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &clen);

        RS_log_sockln("client accepted");

        cc.client_fd = client_socket;


        rs_res = RS_process_command();        


        if(rs_res == RS_FAIL){


            RS_log_sockln("failed to process request for command");

            close(cc.sock_fd);

            PSTAT = RS_ESOCK;

            return;

        } else if (rs_res == RS_DONE){


            RS_log_sockln("finishing interaction");

            close(cc.sock_fd);

            PSTAT = RS_DONE;

            return;

        }


    }


}


RS_CODE RS_process_command(){


    RS_CODE rs_res;

    while(TRUE){

        int val_read ;

        int val_write;

        char CMD[CMD_BUFF_LEN] = {0};

        val_read = read(cc.client_fd, CMD, CMD_BUFF_LEN * sizeof(char));

        if(val_read <= 0){

            RS_log_sockln("val_read <= 0");

            close(cc.client_fd);

            return RS_OKAY;
        }


        if(strcmp(CMD, "XYZ") == 0){

            RS_log_sockln("CMD: XYZ");

            uint8_t READ_CMD[COMMAND_READ_LEN] = {0};

            val_read = read(cc.client_fd, READ_CMD, COMMAND_READ_LEN * sizeof(uint8_t));

            if(val_read <= 0){

                RS_log_sockln("val_read <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }
    

            uint32_t resp_len = 0;

            uint8_t response[CMD_TYPE_XYZ_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, READ_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type xyz response");

                return RS_OKAY;

            }

            if(resp_len != CMD_TYPE_XYZ_LEN){
                RS_log_sockln("data not ready, continue");
                continue;
            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");

        } else if(strcmp(CMD, "IIR") == 0){

            RS_log_sockln("CMD: IIR");

            uint8_t READ_CMD[COMMAND_READ_LEN] = {0};

            val_read = read(cc.client_fd, READ_CMD, COMMAND_READ_LEN * sizeof(uint8_t));

            if(val_read <= 0){

                RS_log_sockln("val_read <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }
    

            uint32_t resp_len = 0;

            uint8_t response[CMD_TYPE_IIR_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, READ_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type iir response");

                return RS_OKAY;

            }

            if(resp_len != CMD_TYPE_IIR_LEN){
                RS_log_sockln("data not ready, continue");
                continue;
            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");

        } else if(strcmp(CMD, "COM") == 0){

            RS_log_sockln("CMD: COM");

            uint8_t READ_CMD[COMMAND_READ_LEN] = {0};

            val_read = read(cc.client_fd, READ_CMD, COMMAND_READ_LEN * sizeof(uint8_t));

            if(val_read <= 0){

                RS_log_sockln("val_read <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }
    

            uint32_t resp_len = 0;

            uint8_t response[CMD_TYPE_COM_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, READ_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type com response");

                return RS_OKAY;

            }

            if(resp_len != CMD_TYPE_COM_LEN){
                RS_log_sockln("data not ready, continue");
                continue;
            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");


        } else if (strcmp(CMD, "XYZUP") == 0){


            RS_log_sockln("CMD: XYZUP");

            uint8_t UPDATE_CMD[CMD_TYPE_XYZ_LEN] = {0};

            uint8_t read_buff[CMD_TYPE_XYZ_LEN] = {0};

            int read_len = 0;


            while(read_len < CMD_TYPE_XYZ_LEN){
                
                memset(read_buff, 0 , CMD_TYPE_XYZ_LEN * sizeof(uint8_t));

                val_read = read(cc.client_fd, read_buff, CMD_TYPE_XYZ_LEN * sizeof(uint8_t));

                if(val_read <= 0){

                    RS_log_sockln("val_read <= 0");

                    close(cc.client_fd);

                    return RS_OKAY;
                }

                for(int i = 0; i < val_read; i++){

                    int idx = i + read_len;

                    UPDATE_CMD[idx] = read_buff[i];


                }

                read_len += val_read;

            }

    

            uint32_t resp_len = 0;

            uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, UPDATE_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type xyzup response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");



        } else if (strcmp(CMD, "IIRUP") == 0){

            RS_log_sockln("CMD: IIRUP");

            uint8_t UPDATE_CMD[CMD_TYPE_IIR_LEN] = {0};

            uint8_t read_buff[CMD_TYPE_IIR_LEN] = {0};

            int read_len = 0;


            while(read_len < CMD_TYPE_IIR_LEN){
                
                memset(read_buff, 0 , CMD_TYPE_IIR_LEN * sizeof(uint8_t));

                val_read = read(cc.client_fd, read_buff, CMD_TYPE_IIR_LEN * sizeof(uint8_t));

                if(val_read <= 0){

                    RS_log_sockln("val_read <= 0");

                    close(cc.client_fd);

                    return RS_OKAY;
                }

                for(int i = 0; i < val_read; i++){

                    int idx = i + read_len;

                    UPDATE_CMD[idx] = read_buff[i];


                }

                read_len += val_read;

            }

    

            uint32_t resp_len = 0;

            uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, UPDATE_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type iirup response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");


        } else if (strcmp(CMD, "COMUP") == 0){

            RS_log_sockln("CMD: COMUP");

            uint8_t UPDATE_CMD[CMD_TYPE_COM_LEN] = {0};

            uint8_t read_buff[CMD_TYPE_COM_LEN] = {0};

            int read_len = 0;

            while(read_len < CMD_TYPE_COM_LEN){
                
                memset(read_buff, 0 , CMD_TYPE_COM_LEN * sizeof(uint8_t));

                val_read = read(cc.client_fd, read_buff, CMD_TYPE_COM_LEN * sizeof(uint8_t));

                if(val_read <= 0){

                    RS_log_sockln("val_read <= 0");

                    close(cc.client_fd);

                    return RS_OKAY;
                }

                for(int i = 0; i < val_read; i++){

                    int idx = i + read_len;

                    UPDATE_CMD[idx] = read_buff[i];


                }

                read_len += val_read;

            }

    

            uint32_t resp_len = 0;

            uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

            // rs_res = RS_response_rdata_raw(&resp_len, response);

            rs_res = RS_roundtrip_to_flag_set_cmd(CMD, UPDATE_CMD, &resp_len, response);

            if(rs_res != RS_OKAY){

                RS_log_sockln("cmd type comup response");

                return RS_OKAY;

            }

            char len_str[64] = {0};
            sprintf(len_str, "resp_len: %d", resp_len);

            RS_log_sockln(len_str);

            val_write = write(cc.client_fd, response, resp_len * sizeof(uint8_t));

            memset(len_str, 0, 64 * sizeof(char));

            sprintf(len_str, "val_write: %d", val_write);

            RS_log_sockln(len_str);            

            if(val_write <=0){

                RS_log_sockln("val_write <= 0");

                close(cc.client_fd);

                return RS_OKAY;
            }

            RS_log_sockln("resp written");


        } else {

            RS_log_sockln("invalid cmd");
            RS_log_sockln(CMD);

            close(cc.client_fd);

            return RS_OKAY;

        }

    }


    return RS_OKAY;
}



RS_CODE RS_roundtrip_to_flag_set_cmd(char* cmd, uint8_t* req, uint32_t* resp_len, uint8_t* response){


    *resp_len = 0;

    uint8_t flag_set_local[FLAG_SET] = {0};

    int val_write = 0;

    RS_CODE rs_res;

    if(strcmp(cmd, "XYZ") == 0){


        memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

        memcpy(gv.read_cmd, req, WL_TX_CMD_READ * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

        if (rs_res != RS_OKAY){

            RS_log_sockln("failed to send read cmd xyz");
            

        } else {


            RS_msleep(100);

            memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

            rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_XYZ_LEN);


            if (rs_res != RS_OKAY){


                RS_log_sockln("failed to read xyz");
                


            } else {

                *resp_len = EC_MAX_XYZ_LEN;

                memcpy(response, gv.read_data, EC_MAX_XYZ_LEN * sizeof(uint8_t));

                flag_set_local[0] = 1;

                flag_set_local[1] = EC_MAX_XYZ_LEN;



            }


        }




    } else if (strcmp(cmd, "IIR") == 0) {


        memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

        memcpy(gv.read_cmd, req, WL_TX_CMD_READ * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

        if (rs_res != RS_OKAY){

            RS_log_sockln("failed to send read cmd iir");
            

        } else {


            RS_msleep(100);

            memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

            rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_IIR_COEF_LEN);


            if (rs_res != RS_OKAY){


                RS_log_sockln("failed to read iir");
                

            } else {

                *resp_len = EC_MAX_IIR_COEF_LEN;

                memcpy(response, gv.read_data, EC_MAX_IIR_COEF_LEN * sizeof(uint8_t));

                flag_set_local[0] = 1;

                flag_set_local[1] = EC_MAX_IIR_COEF_LEN;


            }


        }



    } else if (strcmp(cmd, "COM") == 0) {




        memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

        memcpy(gv.read_cmd, req, WL_TX_CMD_READ * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

        if (rs_res != RS_OKAY){

            RS_log_sockln("failed to send read cmd common");
            

        } else {


            RS_msleep(100);

            memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

            rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_COMMON_LEN);


            if (rs_res != RS_OKAY){


                RS_log_println("failed to read common");
                

            } else {

                *resp_len = EC_MAX_COMMON_LEN;

                memcpy(response, gv.read_data, EC_MAX_COMMON_LEN * sizeof(uint8_t));

                flag_set_local[0] = 1;

                flag_set_local[1] = EC_MAX_COMMON_LEN;


            }


        }


    } else if (strcmp(cmd, "XYZUP") == 0) {

        memset(gv.write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        memcpy(gv.write_cmd, req, EC_MAX_XYZ_LEN * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_XYZ_LEN);

        if (rs_res != RS_OKAY){

            char* failed = "failed to update xyz";

            RS_log_sockln(failed);

            int failed_len = strlen(failed);

            flag_set_local[1] = failed_len;

            *resp_len = failed_len;

            memcpy(response, failed, failed_len * sizeof(uint8_t));


        } else {

            char* success = "successfully updated xyz";

            int success_len = strlen(success);

            flag_set_local[0] = 1;

            flag_set_local[1] = success_len;

            *resp_len = success_len;

            memcpy(response, success, success_len * sizeof(uint8_t));

        }

    } else if (strcmp(cmd, "IIRUP") == 0) {

        memset(gv.write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        memcpy(gv.write_cmd, req, EC_MAX_IIR_COEF_LEN * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_IIR_COEF_LEN);

        if (rs_res != RS_OKAY){

            char* failed = "failed to update iir";

            RS_log_sockln(failed);

            int failed_len = strlen(failed);

            flag_set_local[1] = failed_len;

            *resp_len = failed_len;

            memcpy(response, failed, failed_len * sizeof(uint8_t));


        } else {

            char* success = "successfully updated iir";

            int success_len = strlen(success);

            flag_set_local[0] = 1;

            flag_set_local[1] = success_len;

            *resp_len = success_len;

            memcpy(response, success, success_len * sizeof(uint8_t));

        }


    } else if (strcmp(cmd, "COMUP") == 0) {


        memset(gv.write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        memcpy(gv.write_cmd, req, EC_MAX_COMMON_LEN * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_COMMON_LEN);

        if (rs_res != RS_OKAY){

            char* failed = "failed to update com";

            RS_log_sockln(failed);

            int failed_len = strlen(failed);

            flag_set_local[1] = failed_len;

            *resp_len = failed_len;

            memcpy(response, failed, failed_len * sizeof(uint8_t));


        } else {

            char* success = "successfully updated com";

            int success_len = strlen(success);

            flag_set_local[0] = 1;

            flag_set_local[1] = success_len;

            *resp_len = success_len;

            memcpy(response, success, success_len * sizeof(uint8_t));

        }



    } 



    val_write = write(cc.client_fd, flag_set_local, FLAG_SET * sizeof(uint8_t));

    if(val_write <= 0){

        RS_log_sockln("failed to send flag set cmd");

        close(cc.client_fd);

        return RS_FAIL;

    }


    char flag_str[24] = {0};
    char flag_log[64] = {0};
    RS_stringify_array_u8(flag_str, 4, flag_set_local);

    sprintf(flag_log, "flag: %s", flag_str);

    RS_log_sockln(flag_log);


    return RS_OKAY;
}
