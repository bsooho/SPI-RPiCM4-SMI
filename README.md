# SPI-RPiCM4-SMI


[Architecture](#architecture)\
[Requirements](#requirements)\
[How to run](#how-to-run)\
[Notes](#notes)\
[Datasheet](#datasheet)

## Architecture

원래는\
FPGA 와 통신하고,\
데이터를 검증/정리하고,\
렌더링 하는 것이 하나의 python 플로우로 되어 있었으나

다음과 같이 변경되었음



- dma-spi streamer (c)

FPGA와 상호작용하여 필요한 데이터를 읽어온 뒤 exporter 가 외부 소켓 클라이언트에게 즉각 제공할 수 있는 형태로 변환하여 저장하는 역할을 함\\

- data exporter (c)

streamer 가 수집, 가공한 데이터를 외부 소켓 클라이언트에게 제공하는 역할을 함 

- renderer (python)

이 부분은 exporter 소켓에서 읽어들인 데이터를 사용하여 CMD_TYPE에 따라 렌더링하는 작업을 함


## Requirements

- Raspberry Pi CM4 (Compute Module 4)
- Raspberry Pi OS (Legacy, 32-bit)
- git
- build-essential, make
- python3 3.9.2, python3-venv


## How to run


```shell

# enable force turbo

sudo nano /boot/config.txt 

# 맨 아래 쪽에 다음 추가
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



## Datasheet


아래는 force_turbo 미적용, 적용 시의 테스트 결과임
이는 doc/c-test-noft.txt 및 doc/c-test-ft.txt 에서 확인 가능 함


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





