CMD_TYPE = 1
VLEN = 100
FS = 200
TS = 1/FS

DOUBLE_T = 8

SPL_MAX = 122 #dB
numOfMic = 112
BF_BLOCK_LEN = 8000
MIC_SEL = 0 

BF_DATA_X = 30
BF_DATA_Y = 40
BF_DATA_LEN = BF_DATA_X * BF_DATA_Y
RMS_DATA = 112
MIC_DATA = 8000
BF_MIC_DATA = 8000
#integer u48.0 ->  u25.23 format : /2^23
POW_CONSTANT  = (1/(numOfMic**2 * BF_BLOCK_LEN))  / 2**23

# rms mic conversion
# 8000 times accumulation U13.35
#integer u48 -> u13.35 : /2^35
POW_RMS_CONSTANT = 1/(2**35  * BF_BLOCK_LEN)


RS_WORD = 3
FLAG_SET_BYTE = 4

CMD_TYPE_1_RAW_WORD_LEN = 2627
CMD_TYPE_1_RAW_BYTE_LEN = CMD_TYPE_1_RAW_WORD_LEN * RS_WORD

CMD_TYPE_1_LEN = BF_DATA_LEN + RMS_DATA
CMD_TYPE_1_BYTE_LEN = CMD_TYPE_1_LEN * DOUBLE_T

CMD_TYPE_2_LEN = MIC_DATA + BF_MIC_DATA
CMD_TYPE_2_BYTE_LEN = CMD_TYPE_2_LEN * DOUBLE_T