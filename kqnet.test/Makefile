include config.mk


clean:
	rm -rf $(OUTPUT_DIR)/*

all:
	make -f server/Makefile all
	make -f client/Makefile all 

run:
	./$(OUTPUT_DIR)/server &
	./$(OUTPUT_DIR)/client