
OUTPUT_FILE_NAME = integration_test
CC = gcc
OBJ_DIR = obj
LIB_SRC_DIR = ../../src
LIB_INC_DIR = ../../inc

all: build

build: integration nrf24l01
	$(CC) -g $(OBJ_DIR)/integration.o $(OBJ_DIR)/nrf24l01.o -lbcm2835 -o $(OUTPUT_FILE_NAME)

integration: integration.c
	$(CC) -c integration.c -o $(OBJ_DIR)/integration.o

# Build the actual NRF24L01+ lib under test
nrf24l01: $(LIB_SRC_DIR)/nrf24l01.c
	$(CC) -c $(LIB_SRC_DIR)/nrf24l01.c -I$(LIB_INC_DIR) -DNRF24L01_TEST_ENVIRONMENT -o $(OBJ_DIR)/nrf24l01.o

clean:
	rm -rf $(OBJ_DIR)/*.o $(OUTPUT_FILE_NAME)