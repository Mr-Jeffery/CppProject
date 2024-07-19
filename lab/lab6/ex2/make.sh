g++ -c swap.cpp
ar -cr libswap.a swap.o
g++ -o main main.cpp -L. -lswap
./main