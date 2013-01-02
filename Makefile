
ifeq ($(shell uname -s),Darwin)
EXTRACFLAGS:=-I/opt/local/include -L/opt/local/lib
endif

all:		awimage awflash log2bin

clean:
		rm awimage awflash

log2bin:	log2bin.c
		$(CC) $(EXTRACFLAGS) -Wall -o $@ $^

awimage:	twofish.c rc6.c awimage.c
		$(CC) $(EXTRACFLAGS) -Wall -o $@ $^

awflash:	awflash.c
		$(CC) $(EXTRACFLAGS) -Wall -o $@ $^ -lusb
