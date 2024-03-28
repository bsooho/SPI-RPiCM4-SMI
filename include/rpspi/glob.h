#ifndef _RPSPI_GLOB_H_
#define _RPSPI_GLOB_H_

/*

    @SECTION: GENERAL VARIABLE


*/

#define VERSION_MINOR             2
#define EXPORT_X5                 1

#define GV_CMD_TYPE               1
#define GV_PKT_BYTE_LEN_MAX       4095
#define GV_IS_SET_BPF             1
#define GV_IS_IIR_COEF_20_30      0
#define GV_MIC_SEL                0
#define GV_TEST_ENABLE            0
#define GV_TEST_FREQ              20
#define GV_IS_MIC_AFE             0
#define GV_GAIN                   1

#define SPI_MODE 0
//#define SPI_BPW 8
#define SPI_BPW 32
#define SPI0_SPEED 24995000 
#define SPI1_SPEED 6000000
#define SPI_DELAY 0



#define GPIO_SLEEP_ENABLE         7
#define GPIO_ROM_UPDATE_ENABLE    14
#define GPIO_NEW_BF_DATA          16
#define GPIO_SPI_RESET_N          17

/*
    DEBUG_LEVEL:
        0 = no debug
        1 = write to log
        2 = 1 + verbose print out

*/

#define DEBUG_LEVEL 1


/*

    @SECTION: GENERAL CONSTANT

*/

#define TRUE 1
#define FALSE 0
#define RS_WORD 3

#define WAIT_TIMEOUT 10000
#define WAIT_INTERVAL_MS 100
#define SOCK_MAX_CONN 5
#define FLAG_SET 4

#define CMD_BUFF_LEN 10

#define MAX_TIMESTR_LEN 80
#define MAX_LOG_TXT_LEN 10240

#define MAX_TX_BUFF_LEN 4095
#define MAX_RX_BUFF_LEN 4095
#define POW_2_24 16777216
#define POW_2_35 34359738368
#define POW_2_24D 16777216.0
#define POW_2_23D 8388608.0



/*

    SPI0 DATA

*/

// spi0 read cmd
#define WL_TX_CMD_READ 1 * RS_WORD

// spi0 read

#define EC_MAX_RDATA_LEN 18627 * RS_WORD

// spi0 interpret
#define BF_DATA_X 30
#define BF_DATA_Y 40
#define BF_DATA_LEN BF_DATA_X * BF_DATA_Y
#define RMS_DATA 112
#define MIC_DATA 8000
#define BF_MIC_DATA 8000


/*

    SPI1 DATA

*/

// spi1 read cmd

#define WL_TX_CMD_WRITE_MAX 73 * RS_WORD


// spi1 read

#define EC_MAX_WRITE_CMD_LEN 73 * RS_WORD

#define EC_MAX_XYZ_LEN 73 * RS_WORD
#define EC_MAX_IIR_COEF_LEN 11 * RS_WORD
#define EC_MAX_COMMON_LEN 4 * RS_WORD


/*
    EXPORT DATA

*/

#define MAX_EXPORT_BYTE_LEN 131072

/*

    STATIC 

*/

static char* spidev0 = "/dev/spidev0.0";
static char* spidev1 = "/dev/spidev1.0";    

static char* export_sock = "/tmp/fpga_stream_export.sock";


#endif