
#include "client_core.h"



RENDER_VARIABLE rv;


int main (int argc, char **argv){


    rv.IS_SET_BPF = GV_IS_SET_BPF;
    rv.IS_IIR_COEF_20_30 = GV_IS_IIR_COEF_20_30;
    rv.MIC_SEL = GV_MIC_SEL;
    rv.TEST_ENABLE = GV_TEST_ENABLE;
    rv.TEST_FREQ = GV_TEST_FREQ;
    rv.IS_MIC_AFE = GV_IS_MIC_AFE;
    rv.GAIN = GV_GAIN;


    
    int export_fd;
    int command_fd;



    export_fd = RPCL_init_connection(export_sock);


    if (export_fd < 1){

        printf("failed to connect to export\n");

        return -1;

    }



    command_fd = RPCL_init_connection(command_sock);


    if(command_fd < 1){

        printf("failed to connect to command");

        return -1;

    }
    

    EXPORT_FD = export_fd;

    COMMAND_FD = command_fd;

    char test[10] = {0};


    while(1){

        int cmd_len = 0;

        memset(test, 0, 10 * sizeof(char));

        printf("$ test command: ");

        fgets(test, 10, stdin);

        cmd_len = strlen(test);

        for(int i = 0 ; i < cmd_len; i ++){

            if(test[i] == '\n'){

                test[i] = '\0';
            }

        }

        int read_len = 0;

        if(strcmp(test, "1") == 0){

            RPCL_example_cmd_type_1(test);


        } else if (strcmp(test, "2") == 0){

            RPCL_example_cmd_type_2(test);


        } else if (strcmp(test, "3") == 0){


            RPCL_example_cmd_type_3(test);


        } else if (strcmp(test, "XYZ") == 0){


            RPCL_example_read_xyz(test);


        } else if (strcmp(test, "IIR") == 0){


            RPCL_example_read_iircoef(test);


        } else if (strcmp(test, "COM") == 0){


            RPCL_example_read_common(test);


        } else if (strcmp(test, "XYZUP") == 0){


            RPCL_example_update_xyz(test);


        } else if (strcmp(test, "IIRUP") == 0){


            RPCL_example_update_iircoef(test);


        } else if (strcmp(test, "COMUP") == 0){


            RPCL_example_update_common(test);


        } else if (strcmp(test, "3LOAD") == 0){

            char load[10] = {0};

            strcpy(load, "3");

            for(;;){

                char timestamp[1024] = {0};

                struct timespec h_start, h_end;
                clock_gettime(CLOCK_MONOTONIC_RAW, &h_start);


                RPCL_get_current_time_string(timestamp);

                RPCL_example_cmd_type_3(load);

                clock_gettime(CLOCK_MONOTONIC_RAW, &h_end);

                int delta_ms = (h_end.tv_sec - h_start.tv_sec) * 1000 + (h_end.tv_nsec - h_start.tv_nsec) / 1000000;

                printf("%s : load testing: took %dms\n", timestamp, delta_ms);


            }


        } else {

            printf("invalid cmd: %s\n", test);

        }



    }



    return 0;
}




void RPCL_example_cmd_type_1(char* test){


    printf("CMD_TYPE_1.\n");

    int read_len = 0;

    uint8_t read_bytes[CMD_TYPE_1_BYTE_LEN] = {0};

    read_len = RPCL_get_export(test, read_bytes);

    if(read_len != CMD_TYPE_1_BYTE_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 

    printf("conversion start\n");

    double data_raw[CMD_TYPE_1_LEN];


    for(int i = 0 ; i < CMD_TYPE_1_LEN; i++){

        uint8_t tmp[DOUBLE_T] = {0};

        int start_index = i * DOUBLE_T;

        int end_index = start_index + DOUBLE_T;

        int idx = 0;

        for(int j = start_index; j < end_index; j++){

            
            tmp[idx] = read_bytes[j];

            idx += 1;

        }

        memcpy(&(data_raw[i]), tmp, DOUBLE_T * sizeof(uint8_t));


    }


    int i = 0;

    for (int x = 0; x < BF_DATA_X; x++){

        for(int y = 0; y < BF_DATA_Y; y++){


            rv.bf_data[x][y] = data_raw[i];

            i += 1;

        }

    }


    for(int x = 0; x < RMS_DATA; x++){


        rv.rms_data[x] = data_raw[i];

        i += 1;

    }


    printf("conversion success\n");


}

void RPCL_example_cmd_type_2(char* test){


    printf("CMD_TYPE_2.\n");

    int read_len = 0;

    uint8_t read_bytes[CMD_TYPE_2_BYTE_LEN] = {0};

    read_len = RPCL_get_export(test, read_bytes);

    if(read_len != CMD_TYPE_2_BYTE_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 


    printf("conversion start\n");

    double data_raw[CMD_TYPE_2_LEN];


    for(int i = 0 ; i < CMD_TYPE_2_LEN; i++){

        uint8_t tmp[DOUBLE_T] = {0};

        int start_index = i * DOUBLE_T;

        int end_index = start_index + DOUBLE_T;

        int idx = 0;

        for(int j = start_index; j < end_index; j++){

            
            tmp[idx] = read_bytes[j];

            idx += 1;

        }

        memcpy(&(data_raw[i]), tmp, DOUBLE_T * sizeof(uint8_t));


    }


    int i = 0;


    for(int x = 0; x < MIC_DATA; x++){


        rv.mic_data[x] = data_raw[i];

        i += 1;

    }            

    for(int x = 0; x < BF_MIC_DATA; x++){


        rv.bf_mic_data[x] = data_raw[i];

        i += 1;

    }


    printf("conversion success\n");


}

void RPCL_example_cmd_type_3(char* test){


    printf("CMD_TYPE_3.\n");

    int read_len = 0;

    uint8_t read_bytes[CMD_TYPE_3_BYTE_LEN] = {0};

    read_len = RPCL_get_export(test, read_bytes);

    if(read_len != CMD_TYPE_3_BYTE_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 

    printf("conversion start\n");

    double data_raw[CMD_TYPE_3_LEN];


    for(int i = 0 ; i < CMD_TYPE_3_LEN; i++){

        uint8_t tmp[DOUBLE_T] = {0};

        int start_index = i * DOUBLE_T;

        int end_index = start_index + DOUBLE_T;

        int idx = 0;

        for(int j = start_index; j < end_index; j++){

            
            tmp[idx] = read_bytes[j];

            idx += 1;

        }

        memcpy(&(data_raw[i]), tmp, DOUBLE_T * sizeof(uint8_t));


    }


    int i = 0;

    for (int x = 0; x < BF_DATA_X; x++){

        for(int y = 0; y < BF_DATA_Y; y++){


            rv.bf_data[x][y] = data_raw[i];

            i += 1;

        }

    }


    for(int x = 0; x < RMS_DATA; x++){


        rv.rms_data[x] = data_raw[i];

        i += 1;

    }



    for(int x = 0; x < MIC_DATA; x++){


        rv.mic_data[x] = data_raw[i];

        i += 1;

    }            

    for(int x = 0; x < BF_MIC_DATA; x++){


        rv.bf_mic_data[x] = data_raw[i];

        i += 1;

    }

    printf("conversion success\n");



}

void RPCL_example_update_xyz(char* test){


    printf("CMD_TYPE_XYZUP.\n");

    int read_len = 0;

    uint8_t req[CMD_TYPE_XYZ_LEN] = {0};

    uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

    /*

        TODO:
            conversion
    
    */

    // =======================================================

    uint8_t write_cmd[EC_MAX_WRITE_CMD_LEN] = {0};

    int head;

    int csum;

    int wdata = 0;



    uint8_t byte0;

    uint8_t byte1;

    uint8_t byte2;


    int grid_x[40] = {
        -10041, -9526, -9011, -8496, -7981, 
        -7466, -6951, -6436, -5922, -5407, 
        -4892, -4377, -3862, -3347, -2832, 
        -2317, -1803, -1288, -773, -258, 
        257, 772, 1287, 1802, 2316, 
        2831, 3346, 3861, 4376, 4891, 
        5406, 5921, 6435, 6950, 7465, 
        7980, 8495, 9010, 9525, 10040
    };

    int grid_y[30] = {
        -7467, -6952, -6437, -5922, -5407, 
        -4892, -4377, -3863, -3348, -2833, 
        -2318, -1803, -1288, -773, -258, 
        257, 772, 1287, 1802, 2317, 
        2832, 3347, 3862, 4376, 4891, 
        5406, 5921, 6436, 6951, 7466

    };

    int distance_z = 131072;


    memset(write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

    write_cmd[0] = 0xFC;
    write_cmd[1] = 0x00;
    write_cmd[2] = 0x01;

    head = 0xFC0001;

    csum = head;

    // ----- gridx

    for (int n = 0 ; n < 40; n ++){


        wdata = grid_x[n] & 0x3FFFF;
        csum = (csum + wdata) & 0xFFFFFF;

        byte0 = (uint8_t)(wdata >> 16) & 0x3;
        byte1 = (uint8_t)(wdata >> 8) & 0xFF;
        byte2 = wdata & 0xFF;

        write_cmd[(n+1)*3] = byte0;
        write_cmd[(n+1)*3+1] = byte1;
        write_cmd[(n+1)*3+2] = byte2;

    }

    // ----- gridy

    for(int n = 0 ; n < 30; n ++){

        wdata = grid_y[n] & 0x3FFFF;
        csum = (csum + wdata) & 0xFFFFFF;
        byte0 = (wdata >> 16) & 0x3;
        byte1 = (wdata >> 8) & 0xFF;
        byte2 = wdata & 0xFF;

	    write_cmd[(n+41)*3] = byte0;
	    write_cmd[(n+41)*3+1] = byte1;
	    write_cmd[(n+41)*3+2] = byte2;


    }

    // ----- distance z

    wdata = distance_z & 0xFFFFFF;
    csum = (csum + wdata) & 0xFFFFFF;
    byte0 = (wdata >> 16) & 0xFF;
    byte1 = (wdata >> 8) & 0xFF;
    byte2 = wdata & 0xFF;

	write_cmd[(71)*3] = byte0;
	write_cmd[(71)*3+1] = byte1;
	write_cmd[(71)*3+2] = byte2;

    // ----- check sum

    write_cmd[72*3] = (uint8_t)((csum >> 16) & 0xFF);
    write_cmd[72*3+1] = (uint8_t)((csum >> 8) & 0xFF);
    write_cmd[72*3+2] = (uint8_t)(csum & 0xFF);


    printf("-----------------------------\n");
    printf("cmd xyz write data");

    char arr_str[1024] = {0};

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_XYZ_LEN, write_cmd);

    printf("%s\n", arr_str);

    // =======================================================

    read_len = RPCL_send_command(test, req, response);

    if(read_len < 0){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 




}

void RPCL_example_read_xyz(char* test){



    printf("CMD_TYPE_XYZ.\n");


    int read_len = 0;

    uint8_t req[COMMAND_READ_LEN] = {0};

    uint8_t response[CMD_TYPE_XYZ_LEN] = {0};

    req[0] = 0xAC;
    req[1] = 0x00;
    req[2] = 0x01;

    read_len = RPCL_send_command(test, req, response);

    if(read_len != CMD_TYPE_XYZ_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 

    uint64_t read_data_u64[EC_MAX_WRITE_CMD_LEN] = {0};

    char arr_str[1024] = {0};


    for(int i = 0; i < EC_MAX_XYZ_LEN; i++){

        read_data_u64[i] = (uint64_t)response[i];

    }

    int blen = EC_MAX_XYZ_LEN;
    int wlen = EC_MAX_XYZ_LEN / RS_WORD;

    int index_m2 = EC_MAX_XYZ_LEN - 3;
    int index_m1 = EC_MAX_XYZ_LEN - 2;
    int index_m0 = EC_MAX_XYZ_LEN - 1;


    int csum_fpga = (int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

    int csum_cal = 0;


    for (int k = 0 ; k < wlen - 1; k++){

        int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

        csum_cal = csum_cal + word;
    }

    csum_cal = csum_cal % POW_2_24;

    int csum_check = FALSE;

    if(csum_fpga == csum_cal){

        csum_check = TRUE;

    } 

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_XYZ_LEN, response);

    printf("-----------------------------------------\n");
    printf("cmd xyz read data\n");
    printf("%s\n", arr_str);
    printf("rdata length = %d\n", blen);
    printf("head %x csum_fpga %x  csum cal %x (%d)\n", req, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");

}

void RPCL_example_update_iircoef(char* test){


    printf("CMD_TYPE_IIRUP.\n");

    int read_len = 0;


    uint8_t req[CMD_TYPE_IIR_LEN] = {0};

    uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

    /*

        TODO:
            conversion
    
    */


    // ==================================================


    uint8_t write_cmd[EC_MAX_WRITE_CMD_LEN] = {0};

    int head;

    int csum;

    int wdata = 0;



    uint8_t byte0;

    uint8_t byte1;

    uint8_t byte2;


    write_cmd[0] = 0xFC;
    write_cmd[1] = 0x00;
    write_cmd[2] = 0x02;

    head = 0xFC0002;
    csum = head;

    int coef_b1;
    int coef_gain;

    int coef_a1a2[8] = {0};


    if(rv.IS_IIR_COEF_20_30 == 1){


        coef_b1 = 0xA;
        coef_gain = 4;

        coef_gain = coef_gain * rv.GAIN;

        coef_a1a2[0] = -41416;
        coef_a1a2[1] = 58510;
        coef_a1a2[2] = -47335;
        coef_a1a2[3] = 59339;
        coef_a1a2[4] = -37733;
        coef_a1a2[5] = 62346;
        coef_a1a2[6] = -51992;
        coef_a1a2[7] = 63192;

    } else {
        coef_b1 = 0x6;
        coef_gain = 1563;

        coef_gain = coef_gain * rv.GAIN;

        coef_a1a2[0] = -25114;
        coef_a1a2[1] = 24378;
        coef_a1a2[2] = -434;
        coef_a1a2[3] = 50257;
        coef_a1a2[4] = -61111;
        coef_a1a2[5] = 57541;
        coef_a1a2[6] = -64863;
        coef_a1a2[7] = 64451;


    }

    for (int n = 0; n < 9; n ++){


        if (n == 0){
            
            wdata = (coef_b1 << 16 | coef_gain) & 0xFFFFF;

        } else {

            wdata = coef_a1a2[n-1] & 0x3FFFF;
        }

        byte0 = (uint8_t)((wdata >> 16) & 0xFF);
        byte1 = (uint8_t)((wdata >> 8) & 0xFF);
        byte2 = (uint8_t)(wdata & 0xFF);

        write_cmd[n*3 + 3] = byte0;
        write_cmd[n*3 + 4] = byte1;
        write_cmd[n*3 + 5] = byte2;

        csum = (csum + wdata) & 0xFFFFFF;

    }

    write_cmd[10*3] = (uint8_t)((csum >> 16) & 0xFF);
    write_cmd[10*3+1] = (uint8_t)((csum >> 8) & 0xFF);
    write_cmd[10*3+2] = (uint8_t)(csum & 0xFF);




    printf("-----------------------------\n");
    printf("iir bpf coef write data\n");

    char arr_str[1024] = {0};

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_IIR_COEF_LEN, write_cmd);    

    printf("%s\n", arr_str);

    // ==================================================

    read_len = RPCL_send_command(test, req, response);

    if(read_len < 0){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 


}

void RPCL_example_read_iircoef(char* test){

    printf("CMD_TYPE_IIR.\n");

    int read_len = 0;

    uint8_t req[COMMAND_READ_LEN] = {0};

    uint8_t response[CMD_TYPE_IIR_LEN] = {0};


    req[0] = 0xAC;
    req[1] = 0x00;
    req[2] = 0x02;

    read_len = RPCL_send_command(test, req, response);

    if(read_len != CMD_TYPE_IIR_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 


    uint64_t read_data_u64[EC_MAX_WRITE_CMD_LEN] = {0};

    char arr_str[1024] = {0};



    for(int i = 0; i < EC_MAX_IIR_COEF_LEN; i++){

        read_data_u64[i] = (uint64_t)response[i];

    }


    int blen = EC_MAX_IIR_COEF_LEN;
    int wlen = EC_MAX_IIR_COEF_LEN / RS_WORD;

    int index_m2 = EC_MAX_IIR_COEF_LEN - 3;
    int index_m1 = EC_MAX_IIR_COEF_LEN - 2;
    int index_m0 = EC_MAX_IIR_COEF_LEN - 1;


    int csum_fpga =(int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

    int csum_cal = 0;


    for (int k = 0 ; k < wlen - 1; k++){

        int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

        csum_cal = csum_cal + word;
    }

    csum_cal = csum_cal % POW_2_24;

    int csum_check = FALSE;

    if(csum_fpga == csum_cal){

        csum_check = TRUE;

    } 

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_IIR_COEF_LEN, response);

    printf("-----------------------------------------\n");
    printf("iir bpf read data\n");
    printf("%s\n", arr_str);
    printf("rdata length = %d\n", blen);
    printf("head %x csum_fpga %x  csum cal %x (%d)\n", req, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");

}

void RPCL_example_update_common(char* test){


    printf("CMD_TYPE_COMUP.\n");

    int read_len = 0;

    uint8_t req[CMD_TYPE_COM_LEN] = {0};

    uint8_t response[MAX_COMMAND_BYTE_LEN] = {0};

    /*

        TODO:
            conversion
    
    */

   // =====================================================


    uint8_t write_cmd[EC_MAX_WRITE_CMD_LEN] = {0};

    write_cmd[0] = 0xFC;
    write_cmd[1] = 0x00;
    write_cmd[2] = 0x03;

    int head = 0xFC0003;

    int csum = head;

    int wdata = 0;

    csum = (csum + wdata) & 0xFFFFFF;

    uint8_t byte0 = (uint8_t)((wdata >> 16) & 0xFF);

    uint8_t byte1 = (uint8_t)((wdata >> 8) & 0xFF);

    uint8_t byte2 = (uint8_t)(wdata & 0xFF);

    write_cmd[3] = byte0;
    write_cmd[4] = byte1;
    write_cmd[5] = byte2;

    int common1 = (int)((rv.TEST_FREQ << 16) | (rv.TEST_ENABLE << 9) | (rv.IS_MIC_AFE << 8) | rv.MIC_SEL);

    wdata = common1;

    csum = (csum + wdata) & 0xFFFFFF;

    byte0 = (uint8_t)((wdata >> 16) & 0xFF);

    byte1 = (uint8_t)((wdata >> 8) & 0xFF);

    byte2 = (uint8_t)(wdata & 0xFF);

    write_cmd[6] = byte0;
    write_cmd[7] = byte1;
    write_cmd[8] = byte2;

    write_cmd[9] = (uint8_t)((csum >> 16) & 0xFF);
    write_cmd[10] = (uint8_t)((csum >> 8) & 0xFF);
    write_cmd[11] = (uint8_t)(csum & 0xFF);


    printf("---------------------------------------------\n");

    printf("cmd common write data\n");

    printf("MIC sel [%d] TEST EN[%d] TEST FREQ [%d] is mic AFE [%d]\n",rv.MIC_SEL , rv.TEST_ENABLE, rv.TEST_FREQ , rv.IS_MIC_AFE);

    char arr_str[1024] = {0};

    RPCL_stringify_array_u8(arr_str, EC_MAX_COMMON_LEN, write_cmd);

    printf("%s\n", arr_str);

   // =====================================================


    read_len = RPCL_send_command(test, req, response);

    if(read_len < 0){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 


}

void RPCL_example_read_common(char* test){



    printf("CMD_TYPE_COM.\n");

    int read_len = 0;

    uint8_t req[COMMAND_READ_LEN] = {0};

    uint8_t response[CMD_TYPE_COM_LEN] = {0};


    req[0] = 0xAC;
    req[1] = 0x00;
    req[2] = 0x03;

    read_len = RPCL_send_command(test, req, response);

    if(read_len != CMD_TYPE_COM_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 


    uint64_t read_data_u64[EC_MAX_WRITE_CMD_LEN] = {0};

    char arr_str[1024] = {0};


    for(int i = 0; i < EC_MAX_COMMON_LEN; i++){


        read_data_u64[i] = (uint64_t)response[i];

    }

    int blen = EC_MAX_COMMON_LEN;
    int wlen = EC_MAX_COMMON_LEN / RS_WORD;

    int index_m2 = EC_MAX_COMMON_LEN - 3;
    int index_m1 = EC_MAX_COMMON_LEN - 2;
    int index_m0 = EC_MAX_COMMON_LEN - 1;


    int csum_fpga = (int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

    int csum_cal = 0;

    for (int k = 0 ; k < wlen - 1; k++){

        int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

        csum_cal = csum_cal + word;
    }

    csum_cal = csum_cal & 0xFFFFFF;

    int csum_check = FALSE;

    if(csum_fpga == csum_cal){

        csum_check = TRUE;

    } 

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_COMMON_LEN, response);

    printf("---------------------------------\n");
    printf("cmd common read data\n");
    printf("%s\n", arr_str);

    printf("rdata length = %d\n", blen);

    printf("head %x csum fpga %x  csum cal %x (%d)\n", req, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");


}
