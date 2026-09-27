TARGET  := onenet_mqtt
SRCS    := onenet_mqtt.c
CC      := gcc
CFLAGS  := -Wall -O2 -std=gnu11
LDLIBS  := -lpaho-mqtt3c -lcjson

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)