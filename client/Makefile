


GCC_FLAGS := -Ofast -Wall 

GCC_OBJ_FLAGS := -Ofast -Wall -c

OBJ_CLIENT := client.o 

all: $(OBJ_CLIENT)

	gcc $(GCC_FLAGS) -o client.run main.c $(OBJ_CLIENT)



client.o:

	gcc $(GCC_OBJ_FLAGS) -o client.o client.c


clean:


	sudo rm -r *.run *.test *.o log/log.txt log/sock.txt log/client.txt

