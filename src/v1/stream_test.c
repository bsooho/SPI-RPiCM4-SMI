#include "rpspi/v1/stream.h"
#include "rpspi/v1/utils.h"


RS_CODE RS_stream_test(){




    RS_CODE rs_result;


    char* dev0 = "/dev/spidev0.0";
    char* dev1 = "/dev/spidev1.0";    


    gv.CMD_TYPE = GV_CMD_TYPE;
    gv.PKT_BYTE_LEN_MAX = GV_PKT_BYTE_LEN_MAX;
    
    gv.IS_SET_BPF = GV_IS_SET_BPF;
    gv.IS_IIR_COEF_20_30 = GV_IS_IIR_COEF_20_30;

    gv.MIC_SEL = GV_MIC_SEL;
    gv.TEST_ENABLE = GV_TEST_ENABLE;
    gv.TEST_FREQ = GV_TEST_FREQ;
    gv.IS_MIC_AFE = GV_IS_MIC_AFE;

    gv.GAIN = GV_GAIN;


    gv.spi0.device = dev0;

    gv.spi1.device = dev1;



    if (gpioInitialise() < 0){

        RS_log_println("failed to initialize");

        return RS_FAIL;

    }



    rs_result = RS_init_spi(&gv.spi0, 0, 8, 20000000, 0);

    if (rs_result != RS_OKAY){

        RS_log_println("failed to init spi0");

        return RS_FAIL;
    }

    RS_log_println("initiated spi0");

    rs_result = RS_init_spi(&gv.spi1, 0, 8, 6000000, 0);

    if (rs_result != RS_OKAY){

        RS_log_println("failed to init spi1");

        return RS_FAIL;
    }

    RS_log_println("initiated spi1");

    gpioSetMode(GPIO_SLEEP_ENABLE, PI_OUTPUT);
    gpioSetMode(GPIO_ROM_UPDATE_ENABLE, PI_OUTPUT);
    gpioSetMode(GPIO_NEW_BF_DATA, PI_INPUT);
    gpioSetMode(GPIO_SPI_RESET_N, PI_OUTPUT);


    gpioWrite(GPIO_SLEEP_ENABLE, PI_LOW);
    gpioWrite(GPIO_ROM_UPDATE_ENABLE, PI_LOW);


    gpioWrite(GPIO_SPI_RESET_N, PI_LOW);

    RS_msleep(100);

    gpioWrite(GPIO_SPI_RESET_N, PI_HIGH);

    RS_msleep(100);

    RS_log_println("gpio set completed");

    // TODO:
    //  set spi1 reg

    RS_log_println("!!! SET SPI1 !!!!");

    if (gv.CMD_TYPE == 0){

        gv.read_cmd[0] = 0xFC;
        gv.read_cmd[1] = 0x55;
        gv.read_cmd[2] = 0x00;

        gv.read_word_len = 3;


    } else if (gv.CMD_TYPE == 1){

        gv.read_cmd[0] = 0xFC;
        gv.read_cmd[1] = 0x55;
        gv.read_cmd[2] = 0x01;

        gv.read_word_len = 2627;
        

    } else if (gv.CMD_TYPE == 2){

        gv.read_cmd[0] = 0xFC;
        gv.read_cmd[1] = 0x55;
        gv.read_cmd[2] = 0x02;   

        gv.read_word_len = 16003;  


    } else if (gv.CMD_TYPE == 3){

        gv.read_cmd[0] = 0xFC;
        gv.read_cmd[1] = 0x55;
        gv.read_cmd[2] = 0x77;

        gv.read_word_len = 18627;
    }

    gv.cnt_pre = 0;

    gv.read_byte_len = gv.read_word_len * RS_WORD;
    gv.cnt_repeat = 0;

    gv.num_repeat = (gv.read_byte_len / gv.PKT_BYTE_LEN_MAX) + 1;

    gv.num_final = gv.read_byte_len % gv.PKT_BYTE_LEN_MAX;

    // WARN:
    //  gv.bf_data should all be set to zero
    //  gv.rms_data should all be set to zero
    gv.flag_bf = 0;

    // WARN:
    //  gv.mic_data should all be set to zero
    //  gv.bf_mic_data should all be set to zero
    gv.flag_mic = 0;

    // WARN:
    //  already defined FS, TS


    // WARN:
    //  x = np.linspace(0,TS * (VLEN-1),VLEN)
    //  is for plotting

    /*

        WARN:

            vendor/py/sbf_test_main.py
            line 149 - 175
            all plotting logic

    */

    gv.cnt = 0;

    gv.EV_HANDLE_EC = 0;

    gpioSetMode(GPIO_NEW_BF_DATA, PI_INPUT);
    gpioSetAlertFunc(GPIO_NEW_BF_DATA, RS_gpio_interrupt_handler_test);

    RS_log_println("added gpio interrupt handler");



    while(1){

        if(gv.EV_HANDLE_EC < 0){
            
            return RS_FAIL;

        } else if (gv.EV_HANDLE_EC > 0){

            return RS_OKAY;
        }

        RS_msleep(100);


        if(gv.flag_bf == 1){
            gv.flag_bf = 0;
        }

        if(gv.flag_mic == 1){
            gv.flag_mic = 0;
        }

    }
    

}




void RS_gpio_interrupt_handler_test(int gpio, int level, uint32_t tick){

    if(level != 1){

        return;
    }

    RS_CODE rs_res;

    gv.cnt += 1;
    
    gv.data0_len = 0;
    
    // gv.rdata = []
    gv.rdata_len = 0;


    rs_res = RS_set_spi_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

    //memcpy(gv.spi0.tx_buffer, gv.read_cmd, WL_TX_CMD_READ);

    // rs_res = RS_spi_xfer(&(gv.spi0), WL_TX_CMD_READ, 4095);
    
    char *tmp_gvcnd[24] = {0};
    sprintf(tmp_gvcnd, "gv.cnt: %d", gv.cnt);

    RS_log_txtln(tmp_gvcnd);
    RS_log_txtln("sent tx data");

    if (rs_res != RS_OKAY){

        RS_log_println("error send read cmd");

        RS_exit(EXIT_FAILURE);

    }


    for (int n = 0; n < gv.num_repeat; n++){

        int new_data_len;

        if( n == ( gv.num_repeat - 1) ){
            
            new_data_len = gv.num_final;

        } else {

            new_data_len = gv.PKT_BYTE_LEN_MAX;


        }

        rs_res = RS_buffalloc(&(gv.data0), gv.data0_len, new_data_len);

        if (rs_res != RS_OKAY){

            RS_log_println("failed RS_buffalloc: data0");

            RS_exit(EXIT_FAILURE);

        }

        gv.data0_len = new_data_len;

        rs_res = RS_get_data0_from_spi_rx(&gv, (uint32_t)gv.data0_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed read data from rx");

            RS_exit(EXIT_FAILURE);

        }

        int new_rdata_len = gv.rdata_len + gv.data0_len;


        
        rs_res = RS_buffmerge(&(gv.rdata), gv.rdata_len, gv.data0, gv.data0_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed RS_buffmerge: rdata");

            RS_exit(EXIT_FAILURE);

        }   

        gv.rdata_len = new_rdata_len;


    }


    RS_log_txtln("recvd rx data");

    // 25 * 40ms == 1s

    if (gv.cnt >= 25){

        printf("gv.cnt: %d\n", gv.cnt);
        printf("gpio: %d, level: %d\n", gpio, level);


        gv.cnt = 0;

        int rlen = gv.rdata_len;

        int wlen = rlen / RS_WORD;

        printf("rlen: %d\n", rlen);

        // head

        int head = (gv.rdata[0] << 16) | (gv.rdata[1] << 8) | (gv.rdata[2]);

        // count

        int cnt_now =  (gv.rdata[3] << 16) | (gv.rdata[4]<< 8) | (gv.rdata[5]);

        int cnt_diff = cnt_now - gv.cnt_pre;

        gv.cnt_pre = cnt_now;

        // csum from FPGA

        uint8_t csum_0 = gv.rdata[gv.rdata_len - 1];

        uint8_t csum_1 = gv.rdata[gv.rdata_len - 2];

        uint8_t csum_2 = gv.rdata[gv.rdata_len - 3];

        int csum = (csum_2 << 16) | (csum_1 << 8) | csum_0 ;

        // csum from received data

        int csum_cal = 0;

        for (int k = 0 ; k < wlen - 1; k++){

            int word = (gv.rdata[k * 3] << 16) | (gv.rdata[k*3+1]<< 8) | gv.rdata[k*3+2];

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

        char first_15[15] = {0};
        char last_15[15] = {0};

        for (int i = 0 ; i < 15; i ++){

            first_15[i] = gv.rdata[i];

            last_15[14 - i] = gv.rdata[gv.rdata_len - 1 - i];


        }

        printf("rdata length = %d: %s %s\n", rlen, first_15, last_15);

        printf("head %x csum %x  csum cal %x (%d)\n", head, csum, csum_cal, csum_check);
    
        // BF RMS data
        if (gv.CMD_TYPE == 1){

            int rn = 29;
            int cn = 0;

            for (int k = 0; k < BF_DATA_LEN; k++){

                float tmp = (float)(((gv.rdata[k*6+6]) << 40) | ((gv.rdata[k*6+7]) << 32) | ((gv.rdata[k*6+8]) << 24) | ((gv.rdata[k*6+9]) << 16) | ((gv.rdata[k*6+10]) << 8) | gv.rdata[k*6+11]);

                gv.bf_data[rn][cn] = tmp;

                rn = rn - 1;

                if (rn == -1){

                    rn = 29;
                    cn = cn + 1;
                }
            }


            for (int k = 0 ; k < RMS_DATA; k++){


                float tmp = (float)(((gv.rdata[k*6+7206]) << 40) | ((gv.rdata[k*6+7207]) << 32) | ((gv.rdata[k*6+7208]) << 24) | ((gv.rdata[k*6+7209]) << 16) | ((gv.rdata[k*6+7210]) << 8) | gv.rdata[k*6+7211]);


                gv.rms_data[k] = tmp;

            }

            if(csum_check == TRUE){

                gv.flag_bf = 1;
            
            }



        } else if (gv.CMD_TYPE == 2) {
            
            for (int k = 0; k < MIC_DATA; k++){

                float tmp = (float)(((gv.rdata[k*6+6]) << 16) | ((gv.rdata[k*6+7]) << 8) | gv.rdata[k*6+8]);

                if (tmp >= POW_2_23F) {
                    tmp = tmp - POW_2_24F;
                }

                gv.mic_data[k] = tmp;

                tmp = (float)(((gv.rdata[k*6+9]) << 16) | ((gv.rdata[k*6+10]) << 8) | gv.rdata[k*6+11]);

                if (tmp >= POW_2_23F) {
                    tmp = tmp - POW_2_24F;
                }

                gv.bf_mic_data[k] = tmp;


            }

            if (csum_check == TRUE){
                gv.flag_mic = 1;
            }



        } else {

            RS_log_println("CMD_TYPE not implemented");

            printf("CMD_TYPE: %d\n", gv.CMD_TYPE);

            exit(EXIT_FAILURE);


        }
    
    }






}
