
#include "rprender/client/client.h"





int main (int argc, char **argv){


    
    int export_fd;
    int command_fd;



    export_fd = RPCL_init_connection(export_sock);


    if (export_fd < 1){

        printf("failed to connect to export\n");

        return -1;

    }



    command_fd = RPCL_init_connection(command_sock);


    if(command_fd < 1){

        printf("failed to connect to command");

        return -1;

    }
    

    char test[10] = {0};


    while(1){

        int cmd_len = 0;

        memset(test, 0, 10 * sizeof(char));

        printf("$ test command: ");

        fgets(test, 10, stdin);

        cmd_len = strlen(test);

        for(int i = 0 ; i < cmd_len; i ++){

            if(test[i] == '\n'){

                test[i] = '\0';
            }

        }

        int read_len = 0;

        if(strcmp(test, "1") == 0){

            printf("CMD_TYPE_1.\n");

            uint8_t read_bytes[CMD_TYPE_1_BYTE_LEN] = {0};

            read_len = RPCL_get_export(test, read_bytes);

            if(read_len != CMD_TYPE_1_BYTE_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 


        } else if (strcmp(test, "2") == 0){

            printf("CMD_TYPE_2.\n");

            uint8_t read_bytes[CMD_TYPE_2_BYTE_LEN] = {0};

            read_len = RPCL_get_export(test, read_bytes);

            if(read_len != CMD_TYPE_2_BYTE_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 


        } else if (strcmp(test, "3") == 0){

            printf("CMD_TYPE_3.\n");

            uint8_t read_bytes[CMD_TYPE_3_BYTE_LEN] = {0};

            read_len = RPCL_get_export(test, read_bytes);

            if(read_len != CMD_TYPE_3_BYTE_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 



        } else if (strcmp(test, "XYZ") == 0){


            printf("CMD_TYPE_XYZ.\n");

            uint8_t req[COMMAND_READ_LEN] = {0};

            uint8_t response[CMD_TYPE_XYZ_LEN] = {0};

            req[0] = 0xAC;
            req[1] = 0x00;
            req[2] = 0x01;

            read_len = RPCL_send_command(test, req, response);

            if(read_len != CMD_TYPE_XYZ_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 


        } else if (strcmp(test, "IIR") == 0){

            printf("CMD_TYPE_IIR.\n");

            uint8_t req[COMMAND_READ_LEN] = {0};

            uint8_t response[CMD_TYPE_IIR_LEN] = {0};


            req[0] = 0xAC;
            req[1] = 0x00;
            req[2] = 0x02;

            read_len = RPCL_send_command(test, req, response);

            if(read_len != CMD_TYPE_IIR_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 


        } else if (strcmp(test, "COM") == 0){

            printf("CMD_TYPE_COM.\n");

            uint8_t req[COMMAND_READ_LEN] = {0};

            uint8_t response[CMD_TYPE_COM_LEN] = {0};


            req[0] = 0xAC;
            req[1] = 0x00;
            req[2] = 0x03;

            read_len = RPCL_send_command(test, req, response);

            if(read_len != CMD_TYPE_COM_LEN){

                printf("failed: %d\n", read_len);
            
            } else {

                printf("success!\n");
            } 



        } else if (strcmp(test, "XYZUP") == 0){

            printf("CMD_TYPE_XYZUP.\n");


        } else if (strcmp(test, "IIRUP") == 0){

            printf("CMD_TYPE_IIRUP.\n");



        } else if (strcmp(test, "COMUP") == 0){

            printf("CMD_TYPE_COMUP.\n");


        } else {

            printf("invalid cmd: %s\n", test);

        }



    }




    return 0;
}