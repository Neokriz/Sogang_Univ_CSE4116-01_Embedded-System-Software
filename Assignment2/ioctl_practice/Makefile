#CROSS_COMPILER = arm-none-linux-gnueabi-
CC = $(CROSS_COMPILER)gcc
CFLAGS = -static -Wall
#DEPS = hw2_20171660.h
OBJS = main.o
TARGET = app 

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(wildcard *.o)
	rm -f $(TARGET)
