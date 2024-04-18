# SPI-RPiCM4-SMI


[Architecture](#architecture)\
[Requirements](#requirements)\
[How to run](#how-to-run)\
[Socket Export Protocol](#socket-export-protocol)\
[Client Include](#client-include)\
[Client Example (CMD)](#client-example-cmd)\
[Client Example (C)](#client-example-c)\
[Notes](#notes)\
[Test](#test)

## Architecture

최초 개발은 아래 내용 전체가 python으로 작성됨.\
1.FPGA와 SPI 통신\
2.데이터 검증/정리 (CSUM 확인)\
3.이미지 및 소리데이터 보여주기 위한 렌더링

상기를 아래와 같이 변경하였음.



- dma-spi streamer (C)

FPGA와 상호작용하여 필요한 데이터를 읽어온 뒤 exporter 가 외부 소켓 클라이언트에게 즉각 제공할 수 있는 형태로 변환하여 저장하는 역할.

- data exporter (C)

streamer 가 수집, 가공한 데이터를 외부 소켓 클라이언트에게 제공하는 역할.
향후 실제 장비 SW (SMI Player)에서는 이 소켓을 통해 데이터 주고받음.

- renderer (python)

이 부분은 실제 장비 SW와 관련없고, 상기 exporter 소켓에서 읽어들인 데이터의 자체 검증을 위하여 CMD_TYPE에 따라 렌더링하는 작업에 해당함.
참고용 코드로 해석하면 됨.


## Requirements

- Raspberry Pi CM4 (Compute Module 4)
- Raspberry Pi OS (Legacy, 32-bit)
- git
- build-essential, make
- python3 3.9.2, python3-venv


## How to run


```shell

# enable force turbo

sudo nano /boot/config.txt 또는 /bbot/firmware/config.txt

# 맨 아래에 다음 추가
...
[all]

force_turbo=1

sudo reboot

# git clone


git clone https://github.com/bsooho/SPI-RPiCM4-SMI.git


# build


cd SPI-RPiCM4-SMI

./dep.sh

make v1

# 이때 컴파일러 warning 이 뜨지만 
# 크게 중요한 것은 아님

# run



sudo ./v1.run



# render


cd render/py

python3 -m venv myenv

source myenv/bin/activate

pip install -r requirements.txt

python main.py

# python game_main.py <- pygame


```


## Socket Export Protocol


클라이언트는(ex: render/py/game_main.py)\
기본적으로 unix domain socket 에 데이터를 요청하고\
데이터가 준비 되었는지 확인한 뒤\
해당 데이터 길이만큼을 읽어 들이도록 함

디폴트 socket 위치는 다음과 같음

```shell

/tmp/fpga_stream_export.sock

```

이는

- include/rpspi/rp_glob.h (서버 측)
- render/py/GLOB.py (클라이언트 측)
  
에서 각각 수정할 수 있음


### Definition

```shell

# SPI0

REQUEST:
    << [1] : 1 byte, char
            [1, 2, 3] 중 하나의 값
            CMD_TYPE == 1 은 1
            CMD_TYPE == 2 는 2
            CMD_TYPE == 3 은 3

RESPONSE:

    >> [4] : 4 byte, uint8_t
            예를 들어 CMD_TYPE == 1 요청을 보내면
            [1, 0, 0, 0]
            이 와야 다음 CMD_TYPE_1_BYTE_LEN 만큼 읽을 수 있음

            CMD_TYPE_1
            [1, 0, 0, 0]
            CMD_TYPE_2
            [0, 1, 0, 0]
            CMD_TYPE_3
            [1, 1, 1, 0]

    >> [CMD_TYPE_${X}_BYTE_LEN]: CMD_TYPE_${X}_LEN byte, double 

            해당 CMD_TYPE_${X}_BYTE_LEN 만큼 읽고
            8 byte 씩 double 타입으로 변환하면 됨

# SPI1


REQUEST:
    
    << [10] : 10 byte, char
            XYZ   == read xyz
            IIR   == read iir
            COM   == read common
            XYZUP == update xyz
            IIRUP == update iir
            COMUP == update common
    
    << [req] : ${req} byte
            각 요청 길이에 맞는 만큼
            XYZ   == read xyz     은 73 * 3
            IIR   == read iir     은 11 * 3
            COM   == read common  은 4  * 3
            XYZUP == update xyz   은 가변 길이 (flag[1])
            IIRUP == update iir   은 가변 길이 (flag[1])
            COMUP == update common은 가변 길이 (flag[1])

RESPONSE:

    >> [4] : 4 byte, uint8_t
            첫번째 자리 성공 여부:
                0 == 실패
                1 == 성공
            두번째 자리 응답 길이:
                최대 255
            예를 들어,
            [1, 10, 0, 0]
            이 오면
            성공했고, 응답길이가 10 바이트라는 뜻

    >> [resp]: ${resp} byte, uint8_t 

            각 응답별 response 길이

```

### Client Include 

아래는 타 클라이언트 활용법임

```shell

필요한 파일:
    include/rp_glob.h
    include/rprender/rprender.h
    include/rprender/client/client.h
    render/client/client.c


```

```c

// 사용을 원할 시 아래 파일만 include 하면 됨
// 빌드 예시는 make client 참조
#include "rprender/client/client.h"



```


### Client Example (CMD)

아래는 디버깅용 클라이언트 프로그램 사용법임

```shell

make client

./client

# CMD TYPE 3 데이터 테스트

$ test command: 3
CMD_TYPE_3.
success!
conversion start
conversion success

# read common 테스트 

$ test command: COM
CMD_TYPE_COM.
success!
---------------------------------
cmd common read data
[ 250, 0, 3, 0, 0, 0, 20, 0, 0, 14, 0, 3,  ]
rdata length = 12
head ffb5fe10 csum fpga e0003  csum cal e0003 (1)
-----------------------------------------
```


### Client Example (C)

아래는 render/client/main.c 에서 발췌한 것임


```c


    
    int export_fd;
    int command_fd;

    // SPI0 소켓에 연결

    export_fd = RPCL_init_connection(export_sock);


    if (export_fd < 1){

        printf("failed to connect to export\n");

        return -1;

    }


    // SPI1 소켓에 연결

    command_fd = RPCL_init_connection(command_sock);


    if(command_fd < 1){

        printf("failed to connect to command");

        return -1;

    }
    

    EXPORT_FD = export_fd;

    COMMAND_FD = command_fd;

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

        // 각 명령어에 따라 예제 처리 코드로 분기


        if(strcmp(test, "1") == 0){

            RPCL_example_cmd_type_1(test);


        } else if (strcmp(test, "2") == 0){

            RPCL_example_cmd_type_2(test);


        } else if (strcmp(test, "3") == 0){


            RPCL_example_cmd_type_3(test);


        } else if (strcmp(test, "XYZ") == 0){


            RPCL_example_read_xyz(test);


        } else if (strcmp(test, "IIR") == 0){


            RPCL_example_read_iircoef(test);


        } else if (strcmp(test, "COM") == 0){


            RPCL_example_read_common(test);


        } else if (strcmp(test, "XYZUP") == 0){


            RPCL_example_update_xyz(test);


        } else if (strcmp(test, "IIRUP") == 0){


            RPCL_example_update_iircoef(test);


        } else if (strcmp(test, "COMUP") == 0){


            RPCL_example_update_common(test);


        } else {

            printf("invalid cmd: %s\n", test);

        }



    }



    return 0;

```

- CMD_TYPE_3 예제
```c


void RPCL_example_cmd_type_3(char* test){


    printf("CMD_TYPE_3.\n");

    int read_len = 0;

    uint8_t read_bytes[CMD_TYPE_3_BYTE_LEN] = {0};

    // SPI0 에서는 RPCL_get_export 에
    // 1. 명령코드
    // 2. 해당 명령코드로 읽어들일 만큼의 uint8_t 배열
    // 전달하면 됨

    read_len = RPCL_get_export(test, read_bytes);

    if(read_len != CMD_TYPE_3_BYTE_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 

    printf("conversion start\n");

    // CMD_TYPE_3 의 경우
    // 전처리 계산된 모든 데이터를 읽어오므로
    // 아래와 같이 해석함
    // 같은 호스트이므로 엔디안은 무시

    double data_raw[CMD_TYPE_3_LEN];


    for(int i = 0 ; i < CMD_TYPE_3_LEN; i++){

        uint8_t tmp[DOUBLE_T] = {0};

        int start_index = i * DOUBLE_T;

        int end_index = start_index + DOUBLE_T;

        int idx = 0;

        for(int j = start_index; j < end_index; j++){

            
            tmp[idx] = read_bytes[j];

            idx += 1;

        }

        memcpy(&(data_raw[i]), tmp, DOUBLE_T * sizeof(uint8_t));


    }


    int i = 0;

    for (int x = 0; x < BF_DATA_X; x++){

        for(int y = 0; y < BF_DATA_Y; y++){


            rv.bf_data[x][y] = data_raw[i];

            i += 1;

        }

    }


    for(int x = 0; x < RMS_DATA; x++){


        rv.rms_data[x] = data_raw[i];

        i += 1;

    }



    for(int x = 0; x < MIC_DATA; x++){


        rv.mic_data[x] = data_raw[i];

        i += 1;

    }            

    for(int x = 0; x < BF_MIC_DATA; x++){


        rv.bf_mic_data[x] = data_raw[i];

        i += 1;

    }

    printf("conversion success\n");



}


```

- read XYZ 커맨드 예제
```c


void RPCL_example_read_xyz(char* test){



    printf("CMD_TYPE_XYZ.\n");


    int read_len = 0;

    uint8_t req[COMMAND_READ_LEN] = {0};

    uint8_t response[CMD_TYPE_XYZ_LEN] = {0};

    req[0] = 0xAC;
    req[1] = 0x00;
    req[2] = 0x01;

    // SPI1 에서는 RPCL_send_command 에
    // 1. 명령코드
    // 2. 커맨드 바이트 배열
    // 3. 해당 커맨드로 읽어들일 unint8_t 배열
    // 전달하면 됨

    read_len = RPCL_send_command(test, req, response);

    if(read_len != CMD_TYPE_XYZ_LEN){

        printf("failed: %d\n", read_len);
    
    } else {

        printf("success!\n");
    } 

    uint64_t read_data_u64[EC_MAX_WRITE_CMD_LEN] = {0};

    char arr_str[1024] = {0};


    for(int i = 0; i < EC_MAX_XYZ_LEN; i++){

        read_data_u64[i] = (uint64_t)response[i];

    }

    int blen = EC_MAX_XYZ_LEN;
    int wlen = EC_MAX_XYZ_LEN / RS_WORD;

    int index_m2 = EC_MAX_XYZ_LEN - 3;
    int index_m1 = EC_MAX_XYZ_LEN - 2;
    int index_m0 = EC_MAX_XYZ_LEN - 1;


    int csum_fpga = (int)((read_data_u64[index_m2] << 16) | (read_data_u64[index_m1] << 8) | read_data_u64[index_m0]);

    int csum_cal = 0;


    for (int k = 0 ; k < wlen - 1; k++){

        int word = (int)((read_data_u64[k*3] << 16) | (read_data_u64[k*3+1] << 8) | read_data_u64[k*3+2]);

        csum_cal = csum_cal + word;
    }

    csum_cal = csum_cal % POW_2_24;

    int csum_check = FALSE;

    if(csum_fpga == csum_cal){

        csum_check = TRUE;

    } 

    memset(arr_str, 0, 1024 * sizeof(char));

    RPCL_stringify_array_u8(arr_str, EC_MAX_XYZ_LEN, response);

    printf("-----------------------------------------\n");
    printf("cmd xyz read data\n");
    printf("%s\n", arr_str);
    printf("rdata length = %d\n", blen);
    printf("head %x csum_fpga %x  csum cal %x (%d)\n", req, csum_fpga, csum_cal, csum_check);
    printf("-----------------------------------------\n");

}


```


### Client Example (Python)

아래는 render/py/game_main.py 에서 발췌한 것임


```python

# unix 소켓 연결을 위한 소켓 경로 지정

socket_path = GLOB.EXPORT_SOCK_PATH


while KEEP == True:

    # 연결

    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    client.connect(socket_path)


    print("connected to export socket")


    while KEEP == True:



        if GLOB.CMD_TYPE == 3:


            # CMD_TYPE 3 는
            # 3을 전송하면 됨

            message = "3"

            client.sendall(message.encode())


            # 4 byte 우선 읽고
            # [ x, x, 1, x]
            #  이면 성공

            flag_set_byte = client.recv(GLOB.FLAG_SET_BYTE)

            flag_set_arr = [ x for x in flag_set_byte ]

            print(flag_set_arr)


            # index 2 가 1 이 아니면 continue

            if flag_set_arr[2] != 1 :

                print("invalid flag")

                continue

            # 맞으면
            # CMD_TYPE_3_BYTE_LEN 을 만족할 때까지
            # read 하면 됨

            total_recv_len = 0

            response = b''        

            while total_recv_len != GLOB.CMD_TYPE_3_BYTE_LEN:


                resp = client.recv(GLOB.CMD_TYPE_3_BYTE_LEN)

                print(len(resp))


                response_bytes_len = len(resp)

                total_recv_len += response_bytes_len

                response += resp


            print(total_recv_len)


            # render_plot = render.cmd_type_1(response, render_plot)


            print("cmd_type_3")


            data_raw = []



            for i in range(GLOB.CMD_TYPE_3_LEN):

                start_index = i * GLOB.DOUBLE_T
                end_index = start_index + GLOB.DOUBLE_T

                value_double = struct.unpack("d", response[start_index:end_index])[0]

                data_raw.append(value_double)
            

            print(len(data_raw))

            i = 0

            for x in range(GLOB.BF_DATA_X):

                for y in range(GLOB.BF_DATA_Y):

                    bf_data[x][y] = data_raw[i]

                    i += 1       

            

            for x in range(GLOB.RMS_DATA):

                rms_data[x] = data_raw[i]

                i += 1


            for x in range(GLOB.MIC_DATA):

                mic_data[x] = data_raw[i]

                i += 1       

            

            for x in range(GLOB.BF_MIC_DATA):

                bf_mic_data[x] = data_raw[i]

                i += 1


            print("handled cmd_type_3")

            time.sleep(0.04)

```











## Notes

### 기본
C 프로그램 테스트 및 실행 시 변경을 빈번하게 할 법한 것들은\
include/rpspi/rp_glob.h 에 대부분 정의 되어 있음

CMD_TYPE
SPI0_SPEED
TEST_ENABLE 등을 포함함

변경 하고자 하는 것을 변경 한뒤

```shell
make clean

make v1
```

Python 프로그램 은 
render/py/GLOB.py 에 대부분 정의 되어 있음


### 주의


CMD_TYPE을 설정할 시 꼭 C 프로그램과 renderer python 을 맞추어 줘야함

C 에서 CMD = 1 했으면 python 에서도 CMD = 1



### DMA SPI 관련


DMA 를 직접 구현 하는 코드를 전달 주셨으나\
현 개발자가 응용 구현에 어려움이 있기도 하고\
spidev 커널 모듈에서 96 바이트 이상 전송 시에는 이미 DMA를 사용하므로 [참조1](https://forums.raspberrypi.com/viewtopic.php?t=279587),[참조2](https://github.com/raspberrypi/linux/blob/rpi-4.19.y/drivers/spi/spi-bcm2835.c#L77)

include/rpspi/dma, src/dma 소스 코드 를 현재 프로그램에서 사용하지는 않음


현재 프로그램에서 사용하는 SPI 관련 함수는

include/rpspi/rpspi.h 에 정의 되어 있고\
src/rpspi.c 에 구현 되어 있음

여러 함수가 있으나 실제로 사용하는 함수는

spi_open (시작)\
spi_close (종료)\
spi_xfer2 (송수신)

세 가지임



## Test



### force_turbo

아래는 force_turbo 미적용, 적용 시의 테스트 결과임
이는 doc/log/c-test-noft.txt 및 doc/log/c-test-ft.txt 에서 확인 가능 함


```shell
# force_turbo 미적용

[ 2024-03-28 03:48:52.252 ] total ge40ms   : 26070 / 107700 
[ 2024-03-28 03:48:52.252 ] total ge35ms   : 1925 / 107700 
[ 2024-03-28 03:48:52.252 ] total csum_fail: 0 / 107700 

```


```shell
# force_turbo 적용

[ 2024-03-28 05:09:03.407 ] total ge40ms   : 0 / 107700 
[ 2024-03-28 05:09:03.407 ] total ge35ms   : 2 / 107700 
[ 2024-03-28 05:09:03.407 ] total csum_fail: 0 / 107700 

```


- reproduce 하는 법

```shell

make clean


# CMD_TYPE = 3
# SPI_BPW = 32
# SPI0_SPEED = 24995000 
# EXPORT_X5 = 0
# VERSION_MINOR = 2

make v1


미적용은 /boot/config.txt 에서 맨 아래에 force_turbo=1 를 지우면되고
적용은 추가하면 됨

변경 뒤 리부트 하고

sudo ./v1.run

log/log.txt 에 4초에 한 번씩 (100 회 인터럽트 당 한 번) 리포트 찍히는 것 확인

```

### force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, NO RENDERING

아래는 force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, 그리고 파이썬 쪽에서 렌더링 없이 데이터를 \
읽어가기만 할 경우의 테스트 결과임\
이는 doc/log/c-test-ft-3-all.txt 에서 확인 가능 함

```shell

[ 2024-03-29 09:11:08.886 ] total ge40ms   : 19 / 10000 
[ 2024-03-29 09:11:08.886 ] total ge35ms   : 119 / 10000 
[ 2024-03-29 09:11:08.886 ] total csum_fail: 0 / 10000 
```


- reproduce 하는 법

```shell

make clean


# CMD_TYPE = 3
# SPI_BPW = 32
# SPI0_SPEED = 24995000 
# EXPORT_ ALL = 1
# VERSION_MINOR = 2

make v1

sudo ./v1.run

# 다른 터미널에서

cd render/py

source myenv/bin/activate

# GLOB.py 
# CMD_TYPE = 3
# TEST_3 = 0

log/log.txt 에 4초에 한 번씩 (100 회 인터럽트 당 한 번) 리포트 찍히는 것 확인

```

### force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, RENDERING

아래는 force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, 그리고 파이썬 쪽에서 데이터를 읽어가고\
렌더링 까지 하는 경우의 (CMD_TYPE 1 렌더링) 테스트 결과임\
이는 doc/log/c-test-ft-3-all-render.txt 에서 확인 가능 함

```shell

[ 2024-04-02 01:37:19.535 ] total ge40ms   : 283 / 110200 
[ 2024-04-02 01:37:19.535 ] total ge35ms   : 1023 / 110200 
[ 2024-04-02 01:37:19.535 ] total csum_fail: 0 / 110200 
```


- reproduce 하는 법

```shell

make clean


# CMD_TYPE = 3
# SPI_BPW = 32
# SPI0_SPEED = 24995000 
# EXPORT_ ALL = 1
# VERSION_MINOR = 2

make v1

sudo ./v1.run

# 다른 터미널에서

cd render/py

source myenv/bin/activate

# GLOB.py 
# CMD_TYPE = 3
# TEST_3 = 1

log/log.txt 에 4초에 한 번씩 (100 회 인터럽트 당 한 번) 리포트 찍히는 것 확인


```


### force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, C CLIENT, MINIMAL PRINT

아래는 force_turbo=1, EXPORT_ALL=1, CMD_TYPE=3, 그리고 C 클라이언트에서 데이터를 읽어가고\
프린트를 최소한으로 사용한 테스트 결과임\
이는 doc/log/c-test-ft-3-all-client.txt 에서 확인 가능 함

```shell

[ 2024-04-18 16:19:17.137 ] [REPORT_LAST_100_INT]
[ 2024-04-18 16:19:17.137 ] max:     23, avg: 22, min: 21
[ 2024-04-18 16:19:17.137 ] ge_40ms: 0, ge_35ms: 0, csum_fail: 0
[ 2024-04-18 16:19:17.137 ] total ge40ms   : 0 / 23000 
[ 2024-04-18 16:19:17.137 ] total ge35ms   : 0 / 23000 
[ 2024-04-18 16:19:17.137 ] total csum_fail: 0 / 23000 
[ 2024-04-18 16:19:17.137 ] [END]
```

- reproduce 하는 법

```shell

make clean


# CMD_TYPE = 3
# SPI_BPW = 32
# SPI0_SPEED = 24995000 
# EXPORT_ ALL = 1
# VERSION_MINOR = 2
# PRINTOUT 0

make v1

sudo ./v1.run

# 다른 터미널에서

make client

./client.run

$ test command: 3LOAD

CMD_TYPE_3.
success!
conversion start
conversion success
2024-04-18 16:19:18.370 : load testing: took 1ms
CMD_TYPE_3.
success!
conversion start
conversion success
2024-04-18 16:19:18.371 : load testing: took 1ms
CMD_TYPE_3.
success!
conversion start
conversion success
2024-04-18 16:19:18.372 : load testing: took 1ms



log/log.txt 에 4초에 한 번씩 (100 회 인터럽트 당 한 번) 리포트 찍히는 것 확인


```