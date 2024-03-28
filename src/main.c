#include "rpspi/v1/stream.h"
#include "rpspi/v1/export.h"
#include "rpspi/v1/utils.h"



pthread_t tid;
pthread_mutex_t tmtx;

int main (int argc, char **argv){


    cpu_set_t  mask;

    
    CPU_ZERO(&mask);
    CPU_SET(2, &mask);

    int result = sched_setaffinity(0, sizeof(mask), &mask);
    // result = sched_setaffinity(1, sizeof(mask), &mask);




    RS_CODE rs_res;

    ec.SOCK_ALIVE = FALSE;

    rs_res = RS_export_main(&tid, &tmtx);

    if (rs_res != RS_OKAY){

        RS_log_println("failed to export");

        return -1;
    }


    int i;
    int retry = WAIT_TIMEOUT / WAIT_INTERVAL_MS;

    for(i = 0; i < retry;i ++){


        if(ec.SOCK_ALIVE == TRUE){

            RS_log_println("sock is ready");

            break;
        }

        RS_msleep(WAIT_INTERVAL_MS);

    }


    if(i == retry){

        RS_log_println("failed to create sock, timed out");


        return -2;
    }





    rs_res = RS_stream_main();

    if(rs_res != RS_OKAY){


        RS_log_println("failed to stream");


        return -3;

    }

    RS_log_println("EXIT SUCCESS.");

    return 0;
}



