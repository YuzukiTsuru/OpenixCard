all:		awimage awflash

clean:
		rm awimage awflash

awimage:	twofish.c rc6.c awimage.c
		$(CC) -Wall -o $@ $^

awflash:	awflash.c
		$(CC) -Wall -o $@ $^ -lusb
