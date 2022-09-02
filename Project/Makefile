CFLAGS = -Wall -g

bin/projet: obj/audioserver.o obj/audioclient.o obj/audio.o obj/filter.o
	gcc -o audioserver audioserver.o audio.o filter.o
	gcc -o audioclient audioclient.o audio.o filter.o
	
	

obj/audioserver.o:
	gcc $(CFLAGS) -c -I ./include audioserver.c

obj/audioclient.o:
	gcc $(CFLAGS) -c -I ./include audioclient.c
	
obj/audio.o:
	gcc $(CFLAGS) -c include/audio.c
	
obj/filter.o:
	gcc $(CFLAGS) -c include/filter.c

clean:
	rm *.o
	rm audioserver
	rm audioclient
	
