CC = gcc
CFLAGS = -Wall -g
TARGET = master slave
OBJS = master_final.o slave_final.o

all: $(TARGET)

master: master_final.o 
	$(CC) master_final.o -o master 

slave: slave_final.o 
	$(CC) slave_final.o -o slave 

master_final.o: master_final.c
	$(CC) $(CFLAGS) -c master_final.c 

slave.o: slave_final.c 
	$(CC) $(CFLAGS) -c slave_final.c

#config.o: config.c 
#	$(CC) $(CFLAGS) -c config.c

clean:
	/bin/rm -f *.o $(TARGET)