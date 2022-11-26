compile: geracodigo.o main.o programa.txt
	clear
	gcc -Wall -o test geracodigo.o main.o
	./test programa.txt

test: geracodigo.o main.o
	clear
	gcc -Wall -Wa,--execstack -o main geracodigo.o main.o
	./main samples/p1.txt 10
	./main samples/p2.txt 0
	./main samples/p3.txt 10 20
	./main samples/p4.txt 10
	./main samples/p5.txt 5
