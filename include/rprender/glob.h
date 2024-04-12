#ifndef _RPRENDER_GLOB_H_
#define _RPRENDER_GLOB_H_



/*

    @SECTION: GENERAL VARIABLE

*/


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

// 무조건 필요한 경우가 아니면 
// 아래 정의는 바꾸지 않음

#define TRUE 1
#define FALSE 0
#define RS_WORD 3

#define WAIT_TIMEOUT 10000
#define WAIT_INTERVAL_MS 100
#define SOCK_MAX_CONN 5
#define DOUBLE_T      8

// export 할 때
// CMD_TYPE 0, 1, 2, 3
// 데이터 준비 여부를 표시하기 위해 클라이언트에 보내는 데이터 길이가
// 4 byte 를 나타내기 위해 정의
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

// cmd type 3 최대 138496 byte
// ( 1200 + 112 + 8000 + 8000 ) * 8

#define MAX_EXPORT_BYTE_LEN 140000 

#define CMD_TYPE_1_LEN         BF_DATA_LEN + RMS_DATA
#define CMD_TYPE_1_BYTE_LEN    CMD_TYPE_1_LEN * DOUBLE_T

#define CMD_TYPE_2_LEN         MIC_DATA + BF_MIC_DATA
#define CMD_TYPE_2_BYTE_LEN    CMD_TYPE_2_LEN * DOUBLE_T

#define CMD_TYPE_3_LEN         BF_DATA_LEN + RMS_DATA + MIC_DATA + BF_MIC_DATA
#define CMD_TYPE_3_BYTE_LEN    CMD_TYPE_3_LEN * DOUBLE_T


/*

STATIC

*/


static char* export_sock = "/tmp/fpga_stream_export.sock";






#endif