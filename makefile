CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c99 -g

TARGET = rid

SRCS = main.c abuf.c term.c rrow.c editor_conf.c utf8.c syntax.c scolor.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
