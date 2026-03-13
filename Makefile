CC = gcc
CFLAGS = -Wall -Wextra -std=c17 --coverage
LDFLAGS = --coverage
TARGET = app

SRC = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
