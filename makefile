all: stak

stak: *.c *.h
	gcc -g -o stak *.c -ansi -Wall -Wpedantic -ljack -lm

run: all
	./stak

clean:
	rm stak
