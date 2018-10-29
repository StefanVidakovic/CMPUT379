all: valws379.bin valws379

clean:
	rm -rf *.o
	rm -f valws379.bin
	rm -f valws379	# Remove link

valws379:
	ln -s valws379.bin valws379 # Add symbolic link

valws379.bin: valws379.o linkedlist.o
	gcc -std=c99 -w valws379.o linkedlist.o -o valws379.bin

valws379.o: valws379.c
	gcc -std=c99 -w valws379.c -c

linkedlist.o: linkedlist.c
	gcc -std=c99 -w linkedlist.c -c
