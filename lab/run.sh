echo $1
g++ $1.cpp -c
g++ $1.o -o $1
./$1