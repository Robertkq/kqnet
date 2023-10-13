include config.mk


clean:
	rm .\out\server.exe
	rm .\out\client.exe

all:
	make -f server/Makefile all
	make -f client/Makefile all 

run:
	./$(OUTPUT_DIR)/server
	./$(OUTPUT_DIR)/client