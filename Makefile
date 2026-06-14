CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lutil -lSDL2 -lSDL2_ttf
TARGET = term11

SRCS = main.c parser.c term_grid.c

# Rules

all: $(TARGET)

#How to build the target

$(TARGET): $(SRCS)  parser.h term_grid.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)