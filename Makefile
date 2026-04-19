CC = gcc
CFLAGS = -Wall -Wextra
TARGET = uart_conf.o

all: $(TARGET)

$(TARGET): uart_conf.c
	$(CC) $(CFLAGS) -o $(TARGET) uart_conf.c

test: $(TARGET)
	chmod +x test_uart.sh
	./test_uart.sh

clean:
	rm -f $(TARGET)
