compile: geracodigo.o main.o programa.txt
	clear
	gcc -Wall -o test geracodigo.o main.o
	./test programa.txt

test: geracodigo.o main.o
	clear
	gcc -Wall -o main geracodigo.o main.o
	./main samples/p1.txt
	./main samples/p2.txt
	./main samples/p3.txt
	./main samples/p4.txt
	./main samples/p5.txt
