all:		awimage awflash log2bin

clean:
		rm awimage awflash

log2bin:	log2bin.c
		$(CC) -Wall -o $@ $^

awimage:	twofish.c rc6.c awimage.c
		$(CC) -Wall -o $@ $^

awflash:	awflash.c
		$(CC) -Wall -o $@ $^ -lusb
