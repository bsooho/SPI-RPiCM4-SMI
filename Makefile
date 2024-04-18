


GCC_FLAGS := -Ofast -Wall 

GCC_OBJ_FLAGS := -Ofast -Wall -c

INCLUDES := -I./include

LIBS := -lrt -lpthread -lpigpio -lc 

V1_OBJ := rpspi.o v1_stream.o v1_stream_reg.o v1_export.o v1_command.o v1_utils.o

OBJ_CLIENT := render_client.o 

all:

	@echo "rpspi server"



v1: $(V1_OBJ)

	gcc $(GCC_FLAGS) $(INCLUDES) -o v1.run src/main.c $(V1_OBJ) $(LIBS)


client: $(OBJ_CLIENT)

	gcc $(GCC_FLAGS) $(INCLUDES) -o client.run render/client/main.c $(OBJ_CLIENT)

rpspi.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o rpspi.o src/rpspi.c



v1_stream.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o v1_stream.o src/v1/stream.c


v1_stream_reg.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o v1_stream_reg.o src/v1/stream_reg.c


v1_export.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o v1_export.o src/v1/export.c

v1_command.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o v1_command.o src/v1/command.c


v1_utils.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o v1_utils.o src/v1/utils.c



render_client.o:

	gcc $(GCC_OBJ_FLAGS) $(INCLUDES) -o render_client.o render/client/client.c


clean:


	sudo rm -r *.run *.test *.o log/log.txt log/sock.txt

clean-log:

	sudo rm -r log/log.txt log/sock.txt
