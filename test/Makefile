CC = gcc
CFLAGS = 

BIN_DIR = bin
BUILD_DIR = obj
SRC_DIR = src

SOURCES := $(wildcard $(SRC_DIR)/*.c)
INCLUDES := ./$(SRC_DIR)
SERVER_OBJ := $(BUILD_DIR)/server.o $(BUILD_DIR)/list.o
CLIENT_OBJ := $(BUILD_DIR)/client.o $(BUILD_DIR)/client_ui.o $(BUILD_DIR)/client_ui_interact.o
DEPS := $(SOURCES:%.c=%.d)

all: $(BIN_DIR)/server $(BIN_DIR)/client

$(BIN_DIR)/server: $(SERVER_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(SERVER_OBJ)

$(BIN_DIR)/client: $(CLIENT_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(CLIENT_OBJ) -pthread -lncurses 

-include $(DEPS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) -MM $< > $(BUILD_DIR)/$*.d
	$(CC) -I$(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/*