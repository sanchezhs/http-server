CC = gcc
CFLAGS = -Wextra -Wall -ggdb -Iinclude
LDFLAGS = -lsqlite3
BUILD_DIR = build
SRC_DIR = src

SRCS = $(SRC_DIR)/server.c \
       $(SRC_DIR)/http_request.c \
       $(SRC_DIR)/cJSON.c \
	   $(SRC_DIR)/database.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/server

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
