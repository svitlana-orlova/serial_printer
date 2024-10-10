LDFLAGS = -lserialport
CFLAGS = -O3 -Wall -Wextra -pedantic -std=c99

.PHONY: spreader clean

spreader: spreader.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm spreader
