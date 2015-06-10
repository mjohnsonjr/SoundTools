all: sndinfo sndcat sndchan sndmix sndgen sndplay

sndinfo: sndinfo.o soundfunctions.o
	gcc sndinfo.o soundfunctions.o -o sndinfo

sndcat: sndcat.o soundfunctions.o
	gcc sndcat.o soundfunctions.o -o sndcat

sndchan: sndchan.o soundfunctions.o
	gcc sndchan.o soundfunctions.o -o sndchan

sndmix: sndmix.o soundfunctions.o
	gcc sndmix.o soundfunctions.o -o sndmix

sndgen: sndgen.o soundfunctions.o
	gcc -lm sndgen.o soundfunctions.o -o sndgen

sndplay: sndplay.o soundfunctions.o
	gcc -lm sndplay.o soundfunctions.o -o sndplay

sndinfo.o: sndinfo.c	
	gcc -c sndinfo.c -o sndinfo.o

soundfunctions.o: soundfunctions.c
	gcc -c soundfunctions.c -o soundfunctions.o

sndcat.o: sndcat.c
	gcc -c sndcat.c -o sndcat.o

sndchan.o: sndchan.c
	gcc -c sndchan.c -o sndchan.o

sndmix.o: sndmix.c
	gcc -c sndmix.c -o sndmix.o

sndgen.o: sndgen.c
	gcc -c sndgen.c -o sndgen.o

sndplay.o: sndplay.c
	gcc -c sndplay.c -o sndplay.o
	
clean: 
	rm -f *.o
	rm -f sndinfo
	rm -f sndcat
	rm -f sndchan
	rm -f sndmix 
	rm -f sndgen 
	rm -f sndplay

tarball:
	tar -cvzf Michael-Johnson-Project-1.tar.gz soundfunctions.c soundfunctions.h sndinfo.c sndcat.c sndchan.c sndmix.c sndgen.c sndplay.c README.txt makefile
