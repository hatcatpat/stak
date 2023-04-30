all: stak

cc=gcc
warnings=-Wall -Wpedantic
libraries=-ljack -lm

stak: *.c *.h
	$(cc) -g -o stak *.c -ansi $(warnings) $(libraries)

release: *.c *.h
	$(cc) -o stak *.c -ansi -O3 $(warnings) $(libraries)

run: all
	./stak

clean:
	rm stak
