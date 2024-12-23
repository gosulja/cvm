CC = g++
CFLAGS = -Wall -g
SRC_DIR = src
BUILD_DIR = build
SRC = $(SRC_DIR)/main.cpp
TARGET = $(BUILD_DIR)/cvm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)	&& clear

all: $(BUILD_DIR) $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
