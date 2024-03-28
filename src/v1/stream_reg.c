#include "rpspi/v1/stream.h"
#include "rpspi/v1/utils.h"



// spi1 통신하는 부분
// 지금은 처음 시작할 때만
// 고정 로직으로 한 번 동작

RS_CODE RS_register_spi1(){

    RS_CODE rs_res;

    // common register

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

    int common1 = (int)((gv.TEST_FREQ << 16) | (gv.TEST_ENABLE << 9) | (gv.IS_MIC_AFE << 8) | gv.MIC_SEL);

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

    printf("MIC sel [%d] TEST EN[%d] TEST FREQ [%d] is mic AFE [%d]\n",gv.MIC_SEL , gv.TEST_ENABLE, gv.TEST_FREQ , gv.IS_MIC_AFE);

    char arr_str[1024] = {0};

    RS_stringify_array_u8(arr_str, EC_MAX_COMMON_LEN, write_cmd);

    printf("%s\n", arr_str);


    memset(gv.write_cmd, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

    memcpy(gv.write_cmd, write_cmd, EC_MAX_COMMON_LEN * sizeof(uint8_t));

    rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_COMMON_LEN);

    if (rs_res != RS_OKAY){


        RS_log_println("failed to write common");
        
        return RS_FAIL;


    }

    RS_msleep(100);




    uint8_t read_cmd[WL_TX_CMD_READ] = {0};

    read_cmd[0] = 0xAC;
    read_cmd[1] = 0x00;
    read_cmd[2] = 0x03;

    memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

    memcpy(gv.read_cmd, read_cmd, WL_TX_CMD_READ * sizeof(uint8_t));

    rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

    if (rs_res != RS_OKAY){


        RS_log_println("failed to send read cmd common");
        
        return RS_FAIL;


    }

    RS_msleep(100);

    uint8_t data_0[EC_MAX_WRITE_CMD_LEN] = {0};

    uint8_t read_data[EC_MAX_WRITE_CMD_LEN] = {0};

    uint64_t read_data_u64[EC_MAX_WRITE_CMD_LEN] = {0};

    memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

    rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_COMMON_LEN);


    if (rs_res != RS_OKAY){


        RS_log_println("failed to send read common");
        
        return RS_FAIL;


    }

    memcpy(read_data, gv.read_data, EC_MAX_COMMON_LEN * sizeof(uint8_t));


    for(int i = 0; i < EC_MAX_COMMON_LEN; i++){


        read_data_u64[i] = (uint64_t)read_data[i];

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

    RS_stringify_array_u8(arr_str, EC_MAX_COMMON_LEN, read_data);

    printf("---------------------------------\n");
    printf("cmd common read data\n");
    printf("%s\n", arr_str);

    printf("rdata length = %d\n", blen);

    printf("head %x csum fpga %x  csum cal %x (%d)\n", head, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");

    // grid distance 

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

    memset(arr_str, 0, 1024 * sizeof(char));

    RS_stringify_array_u8(arr_str, EC_MAX_XYZ_LEN, write_cmd);

    printf("%s\n", arr_str);

    memset(gv.write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

    memcpy(gv.write_cmd, write_cmd, EC_MAX_XYZ_LEN * sizeof(uint8_t));

    rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_XYZ_LEN);

    if (rs_res != RS_OKAY){


        RS_log_println("failed to update xyz");
        
        return RS_FAIL;


    }

    read_cmd[0] = 0xAC;
    read_cmd[1] = 0x00;
    read_cmd[2] = 0x01;



    memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

    memcpy(gv.read_cmd, read_cmd, WL_TX_CMD_READ * sizeof(uint8_t));

    rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

    if (rs_res != RS_OKAY){


        RS_log_println("failed to send read cmd xyz");
        
        return RS_FAIL;


    }

    RS_msleep(100);

    memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

    rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_XYZ_LEN);


    if (rs_res != RS_OKAY){


        RS_log_println("failed to read xyz");
        
        return RS_FAIL;


    }

    memset(read_data_u64, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint64_t));
    memset(read_data, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

    memcpy(read_data, gv.read_data, EC_MAX_XYZ_LEN * sizeof(uint8_t));


    for(int i = 0; i < EC_MAX_XYZ_LEN; i++){

        read_data_u64[i] = (uint64_t)read_data[i];

    }

    blen = EC_MAX_XYZ_LEN;
    wlen = EC_MAX_XYZ_LEN / RS_WORD;

    index_m2 = EC_MAX_XYZ_LEN - 3;
    index_m1 = EC_MAX_XYZ_LEN - 2;
    index_m0 = EC_MAX_XYZ_LEN - 1;


    csum_fpga = (int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

    csum_cal = 0;


    for (int k = 0 ; k < wlen - 1; k++){

        int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

        csum_cal = csum_cal + word;
    }

    csum_cal = csum_cal % POW_2_24;

    csum_check = FALSE;

    if(csum_fpga == csum_cal){

        csum_check = TRUE;

    } 

    memset(arr_str, 0, 1024 * sizeof(char));

    RS_stringify_array_u8(arr_str, EC_MAX_XYZ_LEN, read_data);

    printf("-----------------------------------------\n");
    printf("cmd xyz read data\n");
    printf("%s\n", arr_str);
    printf("rdata length = %d\n", blen);
    printf("head %x csum_fpga %x  csum cal %x (%d)\n", head, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");

    if(gv.IS_SET_BPF == 1){

        memset(write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        write_cmd[0] = 0xFC;
        write_cmd[1] = 0x00;
        write_cmd[2] = 0x02;

        head = 0xFC0002;
        csum = head;

        int coef_b1;
        int coef_gain;

        int coef_a1a2[8] = {0};


        if(gv.IS_IIR_COEF_20_30 == 1){


            coef_b1 = 0xA;
            coef_gain = 4;

            coef_gain = coef_gain * gv.GAIN;

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

            coef_gain = coef_gain * gv.GAIN;

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

        memset(arr_str, 0, 1024 * sizeof(char));

        RS_stringify_array_u8(arr_str, EC_MAX_IIR_COEF_LEN, write_cmd);    

        printf("%s\n", arr_str);

        memset(gv.write_cmd, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        memcpy(gv.write_cmd, write_cmd, EC_MAX_IIR_COEF_LEN * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_write_cmd(&gv, EC_MAX_IIR_COEF_LEN);

        if (rs_res != RS_OKAY){


            RS_log_println("failed to update iir coef");
            
            return RS_FAIL;


        }


        RS_msleep(100);


        read_cmd[0] = 0xAC;
        read_cmd[1] = 0x00;
        read_cmd[2] = 0x02;



        memset(gv.read_cmd, 0, WL_TX_CMD_READ * sizeof(uint8_t));

        memcpy(gv.read_cmd, read_cmd, WL_TX_CMD_READ * sizeof(uint8_t));

        rs_res = RS_set_aux_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

        if (rs_res != RS_OKAY){


            RS_log_println("failed to send read cmd iir coef");
            
            return RS_FAIL;


        }

        RS_msleep(100);


        memset(gv.read_data, 0, WL_TX_CMD_WRITE_MAX * sizeof(uint8_t));

        rs_res = RS_get_read_data_from_aux_rx(&gv, EC_MAX_IIR_COEF_LEN);


        if (rs_res != RS_OKAY){


            RS_log_println("failed to read iir coef");
            
            return RS_FAIL;


        }

        memset(read_data_u64, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint64_t));
        memset(read_data, 0, EC_MAX_WRITE_CMD_LEN * sizeof(uint8_t));

        memcpy(read_data, gv.read_data, EC_MAX_IIR_COEF_LEN * sizeof(uint8_t));


        for(int i = 0; i < EC_MAX_IIR_COEF_LEN; i++){

            read_data_u64[i] = (uint64_t)read_data[i];

        }


        blen = EC_MAX_IIR_COEF_LEN;
        wlen = EC_MAX_IIR_COEF_LEN / RS_WORD;

        index_m2 = EC_MAX_IIR_COEF_LEN - 3;
        index_m1 = EC_MAX_IIR_COEF_LEN - 2;
        index_m0 = EC_MAX_IIR_COEF_LEN - 1;


        csum_fpga =(int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

        csum_cal = 0;


        for (int k = 0 ; k < wlen - 1; k++){

            int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

            csum_cal = csum_cal + word;
        }

        csum_cal = csum_cal % POW_2_24;

        csum_check = FALSE;

        if(csum_fpga == csum_cal){

            csum_check = TRUE;

        } 

        memset(arr_str, 0, 1024 * sizeof(char));

        RS_stringify_array_u8(arr_str, EC_MAX_IIR_COEF_LEN, read_data);

        printf("-----------------------------------------\n");
        printf("iir bpf read data\n");
        printf("%s\n", arr_str);
        printf("rdata length = %d\n", blen);
        printf("head %x csum_fpga %x  csum cal %x (%d)\n", head, csum_fpga, csum_cal, csum_check);
        printf("-----------------------------------------\n");
    }


    return RS_OKAY;
}