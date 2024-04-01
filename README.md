# SPI-RPiCM4-SMI


[Architecture](#architecture)\
[Requirements](#requirements)\
[How to run](#how-to-run)\
[Socket Export Protocol](#socket-export-protocol)\
[Notes](#notes)\
[Test](#test)

## Architecture

최초 개발은 아래 내용 전체가 python으로 작성됨.\
1.FPGA와 SPI 통신\
2.데이터 검증/정리 (CSUM 확인)
3.이미지 및 소리데이터 보여주기 위한 렌더링\

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

./deb.sh

make v1

# 이때 컴파일러 warning 이 뜨지만 
# 크게 중요한 것은 아님

# run



sudo ./v1.run



# render


cd render/py

python3 -m venv myenv

source myenv/bin/activate

pip install -r requirements

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

- include/rpspi/glob.h (서버 측)
- render/py/GLOB.py (클라이언트 측)
  
에서 각각 수정할 수 있음


### Definition

```shell

REQUEST:
    << [1] : 1 byte, char
            [1, 2, 3] 중 하나의 값
            CMD_TYPE == 1 은 1
            CMD_TYPE == 2 는 2
            CMD_TYPE == 3 은 3

RESPONSE:

    >> [4] : 4 byte, int
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
include/rpspi/glob.h 에 대부분 정의 되어 있음

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

log/log.txt 에 4초에 한 번씩 (100 회 인터럽트 당 한 번) 리포트 찍히는 것 확인

```

