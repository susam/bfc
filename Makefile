CC = gcc
CFLAGS = -O
PROJECT = bfc

$(PROJECT): $(PROJECT).c
	$(CC) $(CFLAGS) -o $(PROJECT) $(PROJECT).c
man: $(PROJECT) 
	help2man ./$(PROJECT) -n "Brainfuck compiler" -N -o $(PROJECT).1
install: $(PROJECT) 
	install -m 755 $(PROJECT) /usr/local/bin
	install -d /usr/local/man/man1
	gzip < $(PROJECT).1 > /usr/local/man/man1/$(PROJECT).1.gz
uninstall:
	rm -rf /usr/local/bin/$(PROJECT) /usr/local/man/man1/$(PROJECT).1.gz
clean:
	rm -rf $(PROJECT)
