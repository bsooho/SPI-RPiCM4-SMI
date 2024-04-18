#include "rpspi/v1/stream.h"
#include "rpspi/v1/export.h"
#include "rpspi/v1/utils.h"

RS_CODE PSTAT;
GENERAL_VARIABLE gv;

int monitor_ptr = 0;
int monitor_interval_ms[100] = {0};
int monitor_print = 0;
int monitor_csum_fail= 0;
int monitor_ge40ms = 0;
int monitor_ge35ms = 0;

int total_monitor_sample = 0;
int total_ge40 = 0;
int total_ge35 = 0;
int total_csum_fail = 0;
int total_okay = 0;

RS_CODE RS_stream_main(){




    RS_CODE rs_result;


    gv.CMD_TYPE = GV_CMD_TYPE;
    gv.PKT_BYTE_LEN_MAX = GV_PKT_BYTE_LEN_MAX;
    
    gv.IS_SET_BPF = GV_IS_SET_BPF;
    gv.IS_IIR_COEF_20_30 = GV_IS_IIR_COEF_20_30;

    gv.MIC_SEL = GV_MIC_SEL;
    gv.TEST_ENABLE = GV_TEST_ENABLE;
    gv.TEST_FREQ = GV_TEST_FREQ;
    gv.IS_MIC_AFE = GV_IS_MIC_AFE;

    gv.GAIN = GV_GAIN;


    gv.spi0.device = spidev0;

    gv.spi1.device = spidev1;




    if (gpioInitialise() < 0){

        RS_log_println("failed to initialize");

        return RS_FAIL;

    }



    rs_result = RS_init_spi(&gv.spi0, SPI_MODE, SPI_BPW, SPI0_SPEED, SPI_DELAY);

    if (rs_result != RS_OKAY){

        RS_log_println("failed to init spi0");

        return RS_FAIL;
    }

    rs_result = RS_init_spi(&gv.spi1, SPI_MODE, SPI_BPW, SPI1_SPEED, SPI_DELAY);

    if (rs_result != RS_OKAY){

        RS_log_println("failed to init spi1");

        return RS_FAIL;
    }


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


    // 이 부분에서 
    // 원래 코드와 동일하게
    // spi1 관련 통신 진행함

#if VERSION_MINOR == 1

    rs_result = RS_register_spi1();

#elif VERSION_MINOR == 2

    rs_result = RS_register_spi1_2();

#endif


    if (rs_result != RS_OKAY){

        RS_log_println("failed to register spi1");

        return RS_FAIL;

    }


    RS_msleep(100);

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

    PSTAT = RS_OKAY;

    gv.cnt = 0;

    gpioSetMode(GPIO_NEW_BF_DATA, PI_INPUT);

// minor version 1은 
// dynamic allocation 활용하므로
// 약간이나마 성능 저하 있을 수 있음

// minor version 2는
// dynamic allocation 사용 하지 않음

#if VERSION_MINOR == 1

    gpioSetAlertFunc(GPIO_NEW_BF_DATA, RS_gpio_interrupt_handler);



#elif VERSION_MINOR == 2

    gpioSetAlertFunc(GPIO_NEW_BF_DATA, RS_gpio_interrupt_handler2);

#endif



// 원래 코드에서는 여기서 플래그에 따라
// 렌더링을 처리 하였으나
// 변경된 현재 구조에서는 단순히 상태 체크만 하고 루프

    while(1){

        if(PSTAT == RS_FAIL || PSTAT == RS_ESOCK){
            
            RS_exit(-1);

            return RS_FAIL;

        } else if (PSTAT == RS_DONE){

            RS_exit(1);

            return RS_OKAY;
        }

        RS_msleep(100);



    }
    

    return RS_OKAY;
}




// spidev 을 사용하기 위해
// fd 확보 및 설정 적용함

RS_CODE RS_init_spi(RS_SPI* spi, uint8_t mode, uint8_t bpw, uint32_t speed, uint16_t delay){
    
    spi->config.mode = mode;

    spi->config.bits_per_word = bpw;

    spi->config.speed = speed;

    spi->config.delay = delay;


    int fd = spi_open(spi->device, spi->config);

    if (fd < 0){

        return RS_FAIL;

    }


    spi->spi_fd = fd;


    return RS_OKAY;
}




// 40ms 마다 들어오는 RISING 신호 처리를 위한 인터럽트 로직
// minor version 1
// dynamic allocation 사용
// minor version 2 대비 성능 저하 약간 있을 수 있음

void RS_gpio_interrupt_handler(int gpio, int level, uint32_t tick){

    if(level != 1){

        return;
    }

    RS_CODE rs_res;

    gv.cnt += 1;
    
    gv.data0_len = 0;

    // gv.rdata = []
    gv.rdata_len = 0;

    struct timespec h_start, h_end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &h_start);
    
    rs_res = RS_set_spi_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

    
    char tmp_gvcnd[24] = {0};
    sprintf(tmp_gvcnd, "gv.cnt: %d", gv.cnt);

    RS_log_txtln(tmp_gvcnd);
    RS_log_txtln("sent tx data");

    if (rs_res != RS_OKAY){

        RS_log_println("error send read cmd");

        PSTAT = RS_FAIL;

        return;

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

            PSTAT = RS_FAIL;

            return;

        }

        gv.data0_len = new_data_len;

        rs_res = RS_get_data0_from_spi_rx(&gv, (uint32_t)gv.data0_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed read data from rx");

            PSTAT = RS_FAIL;

            return;

        }

        int new_rdata_len = gv.rdata_len + gv.data0_len;

        
        rs_res = RS_buffmerge(&(gv.rdata), gv.rdata_len, gv.data0, gv.data0_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed RS_buffmerge: rdata");

            PSTAT = RS_FAIL;

            return;

        }   

        gv.rdata_len = new_rdata_len;


    }


    RS_log_txtln("recvd rx data");

    // 25 * 40ms == 1s

    if (gv.cnt >= 25){
#if PRINTOUT
        printf("gv.cnt: %d\n", gv.cnt);
        printf("gpio: %d, level: %d\n", gpio, level);
#endif
        gv.cnt = 0;


        // 여기서 비트 연산으로 데이터를 정리하고
        // 렌더링 플래그를 표시

        //rs_res = RS_interpret_rdata_general();

        rs_res = RS_interpret_rdata_export();

        if (rs_res != RS_OKAY){

            RS_log_println("failed RS_rdata_interpreter");

            PSTAT = RS_FAIL;

            return;
        }


        rs_res = RS_buffalloc(&(gv.data0), gv.data0_len, 0);

       
        if (rs_res != RS_OKAY){

            RS_log_println("failed to dealloc gv.data0");

            PSTAT = RS_FAIL;

            return;
        }
 


        rs_res = RS_buffalloc(&(gv.rdata), gv.rdata_len, 0);
    

        if (rs_res != RS_OKAY){

            RS_log_println("failed to dealloc gv.rdata");

            PSTAT = RS_FAIL;

            return;
        }


        gv.data0_len = 0;
        gv.rdata_len = 0;

    }

    
    clock_gettime(CLOCK_MONOTONIC_RAW, &h_end);

    int delta_ms = (h_end.tv_sec - h_start.tv_sec) * 1000 + (h_end.tv_nsec - h_start.tv_nsec) / 1000000;


    monitor_interval_ms[monitor_ptr] = delta_ms;

    monitor_ptr += 1;

    if (monitor_ptr >= 100){
        monitor_ptr = 0;
        monitor_print = 1;
    }

    int monitor_sum = 0;

    int monitor_max = 0;
    for(int i = 0 ; i < 100; i ++){

        monitor_sum += monitor_interval_ms[i];

        if (monitor_interval_ms[i] > monitor_max){
            monitor_max = monitor_interval_ms[i];
        }
    }

    int monitor_avg = monitor_sum / 100;
    
    if(monitor_print == 1){

        char took_ms[150] = {0};

        sprintf(took_ms, "last 100 max: %d, avg: %d", monitor_max, monitor_avg);

        RS_log_txtln("event successfully handled");

        RS_log_txtln(took_ms);

        monitor_print  = 0;


    } else {

        RS_log_txtln("event successfully handled");
    }


    return;

}



// 인터럽트 핸들
// minor version 2
// 아마도 최대 성능

void RS_gpio_interrupt_handler2(int gpio, int level, uint32_t tick){

    if(level != 1){

        return;
    }

    RS_CODE rs_res;

    total_monitor_sample += 1;

    gv.cnt += 1;
    
    gv.data02_len = 0;
    
    // gv.rdata = []
    gv.rdata2_len = 0;

    memset(gv.rdata2, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));
    memset(gv.data02, 0 ,GV_PKT_BYTE_LEN_MAX * sizeof(uint8_t)); 

    struct timespec h_start, h_end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &h_start);
    
    rs_res = RS_set_spi_tx_from_read_cmd(&gv, WL_TX_CMD_READ);

    
    char tmp_gvcnd[24] = {0};
    sprintf(tmp_gvcnd, "gv.cnt: %d", gv.cnt);
#if DEBUG_LEVEL == 2
    RS_log_txtln(tmp_gvcnd);
    RS_log_txtln("sent tx data 2");
#endif

    if (rs_res != RS_OKAY){

        RS_log_println("error send read cmd 2");

        PSTAT = RS_FAIL;

        return;

    }

    for (int n = 0; n < gv.num_repeat; n++){

        int new_data_len;

        if( n == ( gv.num_repeat - 1) ){
            
            new_data_len = gv.num_final;

        } else {

            new_data_len = gv.PKT_BYTE_LEN_MAX;


        }

        memset(gv.data02, 0 ,GV_PKT_BYTE_LEN_MAX * sizeof(uint8_t)); 

        gv.data02_len = new_data_len;



        rs_res = RS_get_data02_from_spi_rx(&gv, (uint32_t)gv.data02_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed read data from rx 2");

            PSTAT = RS_FAIL;

            return;

        }

        int new_rdata_len = gv.rdata2_len + gv.data02_len;

        
        rs_res = RS_buffmerge2(gv.rdata2, gv.rdata2_len, gv.data02, gv.data02_len);

        if (rs_res != RS_OKAY){


            RS_log_println("failed RS_buffmerge: rdata");

            PSTAT = RS_FAIL;

            return;

        }   

        gv.rdata2_len = new_rdata_len;


    }

#if DEBUG_LEVEL == 2
    RS_log_txtln("recvd rx data 2");
#endif
    // 25 * 40ms == 1s

#if EXPORT_ALL

#if PRINTOUT
    printf("gv.cnt: %d\n", gv.cnt);
    printf("gpio: %d, level: %d\n", gpio, level);
#endif
    if(gv.cnt == 25){

        gv.cnt = 0;

    }


    // 여기서 비트 연산으로 데이터를 정리하고
    // 렌더링 플래그를 표시

    //rs_res = RS_interpret_rdata_general();

    rs_res = RS_interpret_rdata2_export();

    if (rs_res != RS_OKAY){

        RS_log_println("failed RS_rdata_interpreter");

        PSTAT = RS_FAIL;

        return;
    }


    gv.data02_len = 0;
    gv.rdata2_len = 0;

    memset(gv.rdata2, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));
    memset(gv.data02, 0 ,GV_PKT_BYTE_LEN_MAX * sizeof(uint8_t)); 



#elif EXPORT_X5 


    if ((gv.cnt % 5) == 0){
#if PRINTOUT
        printf("gv.cnt: %d\n", gv.cnt);
        printf("gpio: %d, level: %d\n", gpio, level);
#endif
        if(gv.cnt == 25){

            gv.cnt = 0;

        }


        // 여기서 비트 연산으로 데이터를 정리하고
        // 렌더링 플래그를 표시

        //rs_res = RS_interpret_rdata_general();

        rs_res = RS_interpret_rdata2_export();

        if (rs_res != RS_OKAY){

            RS_log_println("failed RS_rdata_interpreter");

            PSTAT = RS_FAIL;

            return;
        }


        gv.data02_len = 0;
        gv.rdata2_len = 0;

        memset(gv.rdata2, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));
        memset(gv.data02, 0 ,GV_PKT_BYTE_LEN_MAX * sizeof(uint8_t)); 
    }


#else 


    if (gv.cnt >= 25){
#if PRINTOUT
        printf("gv.cnt: %d\n", gv.cnt);
        printf("gpio: %d, level: %d\n", gpio, level);
#endif
        gv.cnt = 0;

        // 여기서 비트 연산으로 데이터를 정리하고
        // 렌더링 플래그를 표시

        //rs_res = RS_interpret_rdata_general();

        rs_res = RS_interpret_rdata2_export();

        if (rs_res != RS_OKAY){

            RS_log_println("failed RS_rdata_interpreter");

            PSTAT = RS_FAIL;

            return;
        }


        gv.data02_len = 0;
        gv.rdata2_len = 0;

        memset(gv.rdata2, 0, EC_MAX_RDATA_LEN * sizeof(uint8_t));
        memset(gv.data02, 0 ,GV_PKT_BYTE_LEN_MAX * sizeof(uint8_t)); 
    }


#endif


    
    clock_gettime(CLOCK_MONOTONIC_RAW, &h_end);

    int delta_ms = (h_end.tv_sec - h_start.tv_sec) * 1000 + (h_end.tv_nsec - h_start.tv_nsec) / 1000000;


    monitor_interval_ms[monitor_ptr] = delta_ms;

    if(delta_ms >= 40){
        monitor_ge40ms += 1;
    } else if (delta_ms >= 35){
        monitor_ge35ms += 1;
    }

    monitor_ptr += 1;

    if (monitor_ptr >= 100){
        monitor_ptr = 0;
        monitor_print = 1;
    }

    int monitor_sum = 0;

    int monitor_max = 0;
    int monitor_min = 100;
    for(int i = 0 ; i < 100; i ++){

        monitor_sum += monitor_interval_ms[i];

        if (monitor_interval_ms[i] > monitor_max){
            monitor_max = monitor_interval_ms[i];
        }

        if(monitor_interval_ms[i] < monitor_min){

            monitor_min = monitor_interval_ms[i];
        }
    }

    int monitor_avg = monitor_sum / 100;
    
    if(monitor_print == 1){

        char report1[150] = {0};
        char report2[150] = {0};
        char report3[150] = {0};
        char report4[150] = {0};
        char report5[150] = {0};

        sprintf(report1, "max:     %d, avg: %d, min: %d", monitor_max, monitor_avg, monitor_min);
        sprintf(report2, "ge_40ms: %d, ge_35ms: %d, csum_fail: %d",monitor_ge40ms, monitor_ge35ms, monitor_csum_fail);

        total_csum_fail += monitor_csum_fail;
        total_ge40 += monitor_ge40ms;
        total_ge35 += monitor_ge35ms;


        sprintf(report3, "total ge40ms   : %d / %d ", total_ge40, total_monitor_sample);
        sprintf(report4, "total ge35ms   : %d / %d ", total_ge35, total_monitor_sample);
        sprintf(report5, "total csum_fail: %d / %d ", total_csum_fail, total_monitor_sample);


#if DEBUG_LEVEL == 2
        RS_log_txtln("event successfully handled, last 100 took average:  ");

        RS_log_txtln(took_ms);

#elif DEBUG_LEVEL == 1
        RS_log_txtln("[REPORT_LAST_100_INT]");
        RS_log_txtln(report1);
        RS_log_txtln(report2);
        RS_log_txtln(report3);
        RS_log_txtln(report4);
        RS_log_txtln(report5);

        RS_log_txtln("[END]");

#endif

        monitor_print  = 0;
        monitor_csum_fail = 0;
        monitor_ge40ms = 0;
        monitor_ge35ms = 0;


    } 


    return;

}



RS_CODE RS_set_spi_tx_from_read_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len){


    RS_CODE rs_res;

    memset(gv->spi0.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi0.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi0.tx_len = tx_len;
    gv->spi0.rx_len = 0;

    memcpy(gv->spi0.tx_buffer, gv->read_cmd, tx_len);

    errno = 0;

    int status = spi_xfer2(gv->spi0.spi_fd, gv->spi0.tx_buffer, gv->spi0.tx_len, gv->spi0.rx_buffer, gv->spi0.rx_len);

#if DEBUG_LEVEL == 2

    printf("tx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }



    return RS_OKAY;

}


RS_CODE RS_get_data0_from_spi_rx(GENERAL_VARIABLE* gv, uint32_t rx_len){

    RS_CODE rs_res;

    memset(gv->spi0.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi0.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi0.tx_len = 0;
    gv->spi0.rx_len = rx_len;


    errno = 0;

    int status = spi_xfer2(gv->spi0.spi_fd, gv->spi0.tx_buffer, gv->spi0.tx_len, gv->spi0.rx_buffer, gv->spi0.rx_len);

#if DEBUG_LEVEL == 2

    printf("rx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }


    memcpy(gv->data0, gv->spi0.rx_buffer, rx_len);


    return RS_OKAY;
}



RS_CODE RS_get_data02_from_spi_rx(GENERAL_VARIABLE* gv, uint32_t rx_len){

    RS_CODE rs_res;

    memset(gv->spi0.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi0.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi0.tx_len = 0;
    gv->spi0.rx_len = rx_len;


    errno = 0;

    int status = spi_xfer2(gv->spi0.spi_fd, gv->spi0.tx_buffer, gv->spi0.tx_len, gv->spi0.rx_buffer, gv->spi0.rx_len);

#if DEBUG_LEVEL == 2

    printf("rx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }


    memcpy(gv->data02, gv->spi0.rx_buffer, rx_len);


    return RS_OKAY;
}


RS_CODE RS_set_aux_tx_from_read_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len){


    RS_CODE rs_res;

    memset(gv->spi1.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi1.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi1.tx_len = tx_len;
    gv->spi1.rx_len = 0;

    memcpy(gv->spi1.tx_buffer, gv->read_cmd, tx_len);

    errno = 0;

    int status = spi_xfer2(gv->spi1.spi_fd, gv->spi1.tx_buffer, gv->spi1.tx_len, gv->spi1.rx_buffer, gv->spi1.rx_len);

#if DEBUG_LEVEL == 2

    printf("tx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }



    return RS_OKAY;

}

RS_CODE RS_set_aux_tx_from_write_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len){


    RS_CODE rs_res;

    memset(gv->spi1.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi1.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi1.tx_len = tx_len;
    gv->spi1.rx_len = 0;

    memcpy(gv->spi1.tx_buffer, gv->write_cmd, tx_len);

    errno = 0;

    int status = spi_xfer2(gv->spi1.spi_fd, gv->spi1.tx_buffer, gv->spi1.tx_len, gv->spi1.rx_buffer, gv->spi1.rx_len);

#if DEBUG_LEVEL == 2

    printf("tx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }



    return RS_OKAY;

}

RS_CODE RS_get_read_data_from_aux_rx(GENERAL_VARIABLE* gv, uint32_t rx_len){

    RS_CODE rs_res;

    memset(gv->spi1.tx_buffer, 0, MAX_TX_BUFF_LEN);
    memset(gv->spi1.rx_buffer, 0, MAX_RX_BUFF_LEN);

    gv->spi1.tx_len = 0;
    gv->spi1.rx_len = rx_len;


    errno = 0;

    int status = spi_xfer2(gv->spi1.spi_fd, gv->spi1.tx_buffer, gv->spi1.tx_len, gv->spi1.rx_buffer, gv->spi1.rx_len);

#if DEBUG_LEVEL == 2

    printf("rx status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;
    }



    memcpy(gv->read_data, gv->spi1.rx_buffer, rx_len);


    return RS_OKAY;
}



RS_CODE RS_interpret_rdata_general(){


    int rlen = gv.rdata_len;

    int wlen = rlen / RS_WORD;
#if PRINTOUT
    printf("rlen: %d\n", rlen);
#endif
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
#if PRINTOUT
    printf("count %d (%d)\n", cnt_now, cnt_diff);
#endif
    char first_15[15] = {0};
    char last_15[15] = {0};

    for (int i = 0 ; i < 15; i ++){

        first_15[i] = gv.rdata[i];

        last_15[14 - i] = gv.rdata[gv.rdata_len - 1 - i];


    }
#if PRINTOUT
    printf("rdata length = %d: %s %s\n", rlen, first_15, last_15);

    printf("head %x csum %x  csum cal %x (%d)\n", head, csum, csum_cal, csum_check);
#endif
    // BF RMS data
    if (gv.CMD_TYPE == 1){

        int rn = 29;
        int cn = 0;

        for (int k = 0; k < BF_DATA_LEN; k++){

            double tmp = (double)(((gv.rdata[k*6+6]) << 40) | ((gv.rdata[k*6+7]) << 32) | ((gv.rdata[k*6+8]) << 24) | ((gv.rdata[k*6+9]) << 16) | ((gv.rdata[k*6+10]) << 8) | gv.rdata[k*6+11]);

            gv.bf_data[rn][cn] = tmp;

            rn = rn - 1;

            if (rn == -1){

                rn = 29;
                cn = cn + 1;
            }
        }


        for (int k = 0 ; k < RMS_DATA; k++){


            double tmp = (double)(((gv.rdata[k*6+7206]) << 40) | ((gv.rdata[k*6+7207]) << 32) | ((gv.rdata[k*6+7208]) << 24) | ((gv.rdata[k*6+7209]) << 16) | ((gv.rdata[k*6+7210]) << 8) | gv.rdata[k*6+7211]);


            gv.rms_data[k] = tmp;

        }

        if(csum_check == TRUE){

            gv.flag_bf = 1;
        
        }



    } else if (gv.CMD_TYPE == 2) {
        
        for (int k = 0; k < MIC_DATA; k++){

            double tmp = (double)(((gv.rdata[k*6+6]) << 16) | ((gv.rdata[k*6+7]) << 8) | gv.rdata[k*6+8]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            gv.mic_data[k] = tmp;

            tmp = (double)(((gv.rdata[k*6+9]) << 16) | ((gv.rdata[k*6+10]) << 8) | gv.rdata[k*6+11]);

            if (tmp >= POW_2_23D) {
                tmp = tmp - POW_2_24D;
            }

            gv.bf_mic_data[k] = tmp;


        }

        if (csum_check == TRUE){
            gv.flag_mic = 1;
        }



    } else {

        RS_log_println("CMD_TYPE not implemented");

        printf("CMD_TYPE: %d\n", gv.CMD_TYPE);

        return RS_FAIL;


    }


    return RS_OKAY;
}



RS_CODE RS_spi_xfer(RS_SPI* spi, uint32_t tx_len, uint32_t rx_len){


    errno = 0;

    // spi_xfer_write(spi->spi_fd, spi->tx_buffer, spi->tx_len, spi->rx_buffer, spi->rx_len);

    int status = spi_xfer2(spi->spi_fd, spi->tx_buffer, spi->tx_len, spi->rx_buffer, spi->rx_len);


#if DEBUG_LEVEL == 2

    printf("status: %d\n", status);

#endif

    if (errno != 0){

        return RS_FAIL;

    }


    return RS_OKAY;
}




RS_CODE RS_spi_xfer_write(RS_SPI* spi){


    errno = 0;

    spi_xfer_write(spi->spi_fd, spi->tx_buffer, spi->tx_len, spi->rx_buffer, spi->rx_len);

    if (errno != 0){

        return RS_FAIL;

    }


    return RS_OKAY;
}

RS_CODE RS_spi_xfer_read(RS_SPI* spi){



    errno = 0;

    spi_xfer_read(spi->spi_fd, spi->tx_buffer, spi->tx_len, spi->rx_buffer, spi->rx_len);

    if (errno != 0){

        return RS_FAIL;

    }


    return RS_OKAY;
}


RS_CODE RS_buffalloc(uint8_t** buff, uint32_t old_len, uint32_t new_len){



    if (old_len == 0 && new_len == 0){

        return RS_FAIL;

    }

    
    if(old_len < 0 || new_len < 0){

        return RS_FAIL;
    }

    uint8_t* new_buff;


    if (old_len == 0){

        // new alloc

        new_buff = (uint8_t*)malloc(new_len * sizeof(uint8_t));


        memset(new_buff, 0, new_len * sizeof(uint8_t));


        *buff = new_buff;


    } else if (new_len == 0){

        // free

        free(*buff);


    } else {

        // realloc

        new_buff = (uint8_t*)realloc(*buff, new_len * sizeof(uint8_t));

        memset(new_buff, 0, new_len * sizeof(uint8_t));

        *buff = new_buff;

    }



    return RS_OKAY;
}



RS_CODE RS_buffmerge(uint8_t** buff, uint32_t buff_len, uint8_t* new_buff, uint32_t new_buff_len){


    if (new_buff_len == 0){

        return RS_FAIL;

    }

    
    if(buff_len < 0 || new_buff_len < 0){

        return RS_FAIL;
    }

    uint8_t* merged_buff;

    if(buff_len == 0){

        // new alloc

        merged_buff = (uint8_t*)malloc(new_buff_len * sizeof(uint8_t));

        memset(merged_buff, 0, new_buff_len * sizeof(uint8_t));

        memcpy(merged_buff, new_buff, new_buff_len * sizeof(uint8_t));

        *buff = merged_buff;

    } else {

        // realloc copy

        int new_merged_len = buff_len + new_buff_len;

        merged_buff = (uint8_t*)realloc(*buff, new_merged_len * sizeof(uint8_t));

        int bi = 0;

        for (int i = 0; i < new_buff_len; i++){

            bi = buff_len + i;

            merged_buff[bi] = new_buff[i];

        }

        *buff = merged_buff;

    }




    return RS_OKAY;
}



RS_CODE RS_buffmerge2(uint8_t* buff, uint32_t buff_len, uint8_t* new_buff, uint32_t new_buff_len){


    if (new_buff_len == 0){

        return RS_FAIL;

    }

    
    if(buff_len < 0 || new_buff_len < 0){

        return RS_FAIL;
    }



    for(int i = 0; i < new_buff_len; i ++){

        int add_idx = buff_len + i;

        buff[add_idx] = new_buff[i];

    }



    return RS_OKAY;
}




void RS_exit(int ex_code){


    RS_log_println("deallocate buffer...");


    if(gv.rdata_len != 0 ){

        free(gv.rdata);

    }


    if(gv.data0_len != 0){

        free(gv.data0);
    }

    RS_log_println("close spi devices...");

    // spi_close(gv.spi0.spi_fd);

    // spi_close(gv.spi1.spi_fd);

    RS_log_println("terminating gpio...");

    //gpioTerminate();


    RS_log_println("clean up successful!");

    

}







