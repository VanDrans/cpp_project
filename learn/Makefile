TARGET := father_son_mq_test
SRCS   := father_son_mq_test.c
CC     := gcc
CFLAGS := -Wall -O2 -std=gnu11
LDLIBS := -lpaho-mqtt3c -lcjson

all: $(TARGET)
	./$(TARGET)
	rm -f $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all