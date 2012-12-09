all:		awimage awflash

clean:
		rm awimage awflash

awimage:	twofish.c rc6.c awimage.c
		$(CC) -o $@ $^

awflash:	awflash.c
		$(CC) -o $@ $^ -lusb
