all:		awimage

awimage:	twofish.c rc6.c awimage.c
		$(CC) -o $@ $^
