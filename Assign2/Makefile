all: gameclient379 gameserver379

clean:
	rm -rf *.o
	rm gameclient379
	rm gameserver379

gameclient379: gameclient379.o
	gcc -std=c99 -w gameclient379.o -o gameclient379 -pthread -lncurses

gameclient379.o: gameclient379.c
	gcc -std=c99 -w gameclient379.c -c -pthread -lncurses

gameserver379: gameserver379.o
	gcc -std=c99 -w gameserver379.o -o gameserver379	-pthread

gameserver379.o: gameserver379.c
	gcc -std=c99 -w gameserver379.c -c -pthread
