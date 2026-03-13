CC = gcc
CFLAGS = -Wall -Wextra -std=c17 --coverage
LDFLAGS = --coverage

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

test:
	./test/run_tests

clean:
	rm -f src/*.o $(TARGET)
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
