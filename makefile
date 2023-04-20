all: stak

stak: *.c *.h
	gcc -g -o stak *.c -Wall -Wpedantic -ansi -lm -ljack

run: all
	./stak

clean:
	rm stak
