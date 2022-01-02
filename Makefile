main.out: userspace.c
	gcc -li2c -O -o $@ $^

run: main.out
	./$<
