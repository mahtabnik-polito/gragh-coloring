main: ./main.cpp
	gcc ./mmio.c -c -O3
	g++ ./main.cpp -fopenmp -c -O3 -std=c++14
	g++ -o main main.o mmio.o -O3 -fopenmp -std=c++14
clean:
	rm main *.o

