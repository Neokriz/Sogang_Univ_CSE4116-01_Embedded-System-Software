CROSS_COMPILER = arm-none-linux-gnueabi-
CC = $(CROSS_COMPILER)gcc
CFLAGS = -static -Wall
OBJS = main.o
TARGET = app

INCLUDE = -I ../module/

all: $(TARGET)  

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) 

clean:
	rm -rf $(wildcard *.o)
	rm -rf $(TARGET) 
