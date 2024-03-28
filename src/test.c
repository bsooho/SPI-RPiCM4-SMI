#include "rpspi/v1/stream.h"

#include "rpspi/v1/utils.h"





int main (int argc, char **argv){

    RS_CODE rs_res;

    rs_res = RS_stream_test();

    if(rs_res != RS_OKAY){

        RS_log_println("EXIT FAIL.");

        RS_exit(EXIT_FAILURE);

        return -1;

    }

    RS_log_println("EXIT SUCCESS.");

    return 0;
}


