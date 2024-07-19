gcc -o ./bin/block/matmul_O3 matmul.c -Ofast -lm
javac MatrixMultiplication.java
echo "n,t,runtime" > data/block/matmul_Java.csv
n=2
t=100
while [ $n -le 1024 ]; do
    echo "n=$n"
    ./bin/block/matmul_O3 -n $n
    output=$(java MatrixMultiplication $n $t 16)
    runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
    echo "$n,$t,$runtime" >> data/block/matmul_Java.csv
    n=$((n * 2))
done

# gcc -o matmul matmul.c -Ofast -lm
# javac MatrixMultiplication.java
# n=256
# t=1024
# while [ $t -ge 1 ]; do
#     echo "n=$n"
#     output=$(java MatrixMultiplication $n $t 16)
#     runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
#     echo "$n,$t,$runtime" >> data/naive/matmul_Java_t.csv
#     t=$((t / 2))
# done