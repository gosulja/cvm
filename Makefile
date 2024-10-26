CC = g++
CFLAGS = -Wall -g
SRC_DIR = src
BUILD_DIR = build
SRC = $(SRC_DIR)/main.cpp
TARGET = $(BUILD_DIR)/cvm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -rf $(BUILD_DIR)
