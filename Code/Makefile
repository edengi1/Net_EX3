# Makefile for TCP project

all: measure sender

measure: Measure.c
	gcc -o measure Measure.c

sender: sender.c
	gcc -o sender sender.c

clean:
	rm -f *.o measure sender

runs:
	./measure

runc:
	./sender

runs-strace:
	strace -f ./sender

runc-strace:
	strace -f ./measure