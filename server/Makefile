CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic
TARGET  = server
SRCS    = main.c network.c client_store.c messaging.c session.c command_handler.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean