include config.mk

APP_NAME = server
SRC = $(wildcard $(APP_NAME)/*.cpp)

all:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) $(LIB_DIR) $(LIB_LINK) -o $(OUTPUT_DIR)/$(APP_NAME)