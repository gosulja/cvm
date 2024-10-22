CC = g++
CFLAGS = -Wall -g
SRC = main.cpp
TARGET = cvm

#all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
