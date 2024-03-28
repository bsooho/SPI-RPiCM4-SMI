#ifndef _RPSPI_V1_STREAM_H_
#define _RPSPI_V1_STREAM_H_






/*

    @INCLUDE
*/


#include "rpspi/rpspi.h"

#include <pigpio.h>


/*
    @DEFINITION

*/



/*
    @DATA
*/



typedef enum RS_CODE {

    RS_OKAY,
    RS_FAIL,
    RS_DONE,
    RS_ESOCK,

} RS_CODE;



typedef struct RS_SPI{

    char *device;
    spi_config_t config;
    int spi_fd;
    uint32_t tx_len;
    uint32_t rx_len;
    uint8_t tx_buffer[MAX_TX_BUFF_LEN];
    uint8_t rx_buffer[MAX_RX_BUFF_LEN];

} RS_SPI;


typedef struct GENERAL_VARIABLE {


    int CMD_TYPE;
    int PKT_BYTE_LEN_MAX;
    int IS_SET_BPF;
    int IS_IIR_COEF_20_30;
    
    int MIC_SEL;
    int TEST_ENABLE;
    int TEST_FREQ;
    int IS_MIC_AFE;

    int GAIN;

    RS_SPI spi0;

    RS_SPI spi1;

    uint8_t read_cmd[WL_TX_CMD_READ];
    int read_word_len;
    uint8_t write_cmd[WL_TX_CMD_WRITE_MAX];

    int cnt_pre;
    int read_byte_len;
    int cnt_repeat;
    int num_repeat;
    int num_final;
    
    double bf_data[BF_DATA_X][BF_DATA_Y];
    double rms_data[RMS_DATA];
    int flag_bf;
    double mic_data[MIC_DATA];
    double bf_mic_data[MIC_DATA];
    int flag_mic;
    int cnt;



    uint8_t read_data[WL_TX_CMD_WRITE_MAX];

    uint8_t* rdata;
    int rdata_len;
    uint8_t* data0;
    int data0_len;

    uint8_t rdata2[EC_MAX_RDATA_LEN];
    int rdata2_len;
    uint8_t data02[GV_PKT_BYTE_LEN_MAX];
    int data02_len;

} GENERAL_VARIABLE;




/*

    @VARIABLE
*/

extern RS_CODE PSTAT;
extern GENERAL_VARIABLE gv;

extern int monitor_ptr;
extern int monitor_interval_ms[100];
extern int monitor_print;
extern int monitor_csum_fail;
extern int monitor_ge40ms;
extern int monitor_ge35ms;

extern int total_monitor_sample;
extern int total_ge40;
extern int total_ge35;
extern int total_csum_fail;
extern int total_okay;

/*

    @FUNCTION

*/


// main

RS_CODE RS_stream_main();




RS_CODE RS_init_spi(RS_SPI* spi, uint8_t mode, uint8_t bpw, uint32_t speed, uint16_t delay);

void RS_gpio_interrupt_handler(int gpio, int level, uint32_t tick);

void RS_gpio_interrupt_handler2(int gpio, int level, uint32_t tick);


RS_CODE RS_interpret_rdata_general();


RS_CODE RS_spi_xfer(RS_SPI* spi, uint32_t tx_len, uint32_t rx_len);


RS_CODE RS_set_spi_tx_from_read_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len);

RS_CODE RS_get_data0_from_spi_rx(GENERAL_VARIABLE* gv, uint32_t rx_len);

RS_CODE RS_get_data02_from_spi_rx(GENERAL_VARIABLE* gv, uint32_t rx_len);


RS_CODE RS_set_aux_tx_from_read_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len);

RS_CODE RS_set_aux_tx_from_write_cmd(GENERAL_VARIABLE* gv, uint32_t tx_len);

RS_CODE RS_get_read_data_from_aux_rx(GENERAL_VARIABLE* gv, uint32_t rx_len);


RS_CODE RS_spi_xfer_write(RS_SPI* spi);

RS_CODE RS_spi_xfer_read(RS_SPI* spi);


RS_CODE RS_buffalloc(uint8_t** buff, uint32_t old_len, uint32_t new_len);


RS_CODE RS_buffmerge(uint8_t** buff, uint32_t buff_len, uint8_t* new_buff, uint32_t new_buff_len);

RS_CODE RS_buffmerge2(uint8_t* buff, uint32_t buff_len, uint8_t* new_buff, uint32_t new_buff_len);



void RS_exit(int ex_code);


// reg




RS_CODE RS_register_spi1();





// test

RS_CODE RS_stream_test();

void RS_gpio_interrupt_handler_test(int gpio, int level, uint32_t tick);






#endif