#ifndef _RPSPI_SERVER_H_
#define _RPSPI_SERVER_H_


#define _GNU_SOURCE
/*
    @INCLUDES
*/

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#include <linux/spi/spidev.h>
#include <pthread.h>


#include <sched.h>


#include "glob.h"


/*

    @DEFINITION


*/


/*

    @DATA

*/

typedef struct {
    uint8_t mode;
    uint8_t bits_per_word;
    uint32_t speed;
    uint16_t delay;
} spi_config_t;

/*

    @VARIABLE

*/

extern pthread_t tid;
extern pthread_t tid_cmd;
extern pthread_mutex_t tmtx; 

/*

    @FUNCTION

*/


// 아래에서
// spi_open
// spi_close
// spi_xfer2
// 말고는 현재 안씀

int spi_open(char *device, spi_config_t config);
int spi_close(int fd);
int spi_xfer(int fd, uint8_t *tx_buffer, uint8_t tx_len, uint8_t *rx_buffer, uint8_t rx_len);
int spi_xfer2(int fd, uint8_t *tx_buffer, uint32_t tx_len, uint8_t *rx_buffer, uint32_t rx_len);
int spi_xfer_read(int fd, uint8_t *tx_buffer, uint32_t tx_len, uint8_t *rx_buffer, uint32_t rx_len);
int spi_xfer_write(int fd, uint8_t *tx_buffer, uint32_t tx_len, uint8_t *rx_buffer, uint32_t rx_len);
int spi_read(int fd, uint8_t *rx_buffer, uint8_t rx_len);
int spi_write(int fd, uint8_t *tx_buffer, uint8_t tx_len);

#endif