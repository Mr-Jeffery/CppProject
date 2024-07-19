g++ -c -Wall -Werror -fpic vabs.cpp
g++ -shared -o libvabs.so vabs.o
g++ -L $(pwd) -Wall -o ex1 ex1.cpp -lvabs
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./ex1
