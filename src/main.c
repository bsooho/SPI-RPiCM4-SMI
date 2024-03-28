#include "rpspi/v1/stream.h"
#include "rpspi/v1/export.h"
#include "rpspi/v1/utils.h"



pthread_t tid;
pthread_mutex_t tmtx;

int main (int argc, char **argv){


    // cpu core 2 개 할당

    cpu_set_t  mask;

    
    CPU_ZERO(&mask);
    CPU_SET(2, &mask);

    int result = sched_setaffinity(0, sizeof(mask), &mask);
    // result = sched_setaffinity(1, sizeof(mask), &mask);


    // export 스레드를 생성하고
    // 지정한 unix domain 소켓에서 클라이언트가 데이터 요청 및
    // 응답 받을 수 있도록 함


    RS_CODE rs_res;

    ec.SOCK_ALIVE = FALSE;

    rs_res = RS_export_main(&tid, &tmtx);

    if (rs_res != RS_OKAY){

        RS_log_println("failed to export");

        return -1;
    }

    // 대기하다가 커넥션 실패하면 종료함

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



    // streamer 를 시작
    // FPGA 측에서 데이터 수집하여 정리


    rs_res = RS_stream_main();

    if(rs_res != RS_OKAY){


        RS_log_println("failed to stream");


        return -3;

    }

    RS_log_println("EXIT SUCCESS.");

    return 0;
}



