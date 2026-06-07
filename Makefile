CC = gcc

FLAGS = -lX11 -lm -Wall -Wextra -g

all:
	$(CC) main.c -o wm $(FLAGS)

clean:
	rm -f wm

run: all
	./wm