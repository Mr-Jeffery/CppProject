for flag in 3; do
    echo "flag="$flag
    gcc -o ./bin/block/matmul_O$flag matmul.c -O$flag -lm
    echo "n,t,runtime" > data/block/matmul_O$flag.csv
    n=2
    t=10
    while [ $n -le 8196 ]; do
        echo "n=$n"
        output=$(./bin/block/matmul_O$flag -n $n -t $t -b 16)
        runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
        echo "$n,$t,$runtime" >> data/block/matmul_O$flag.csv
        n=$((n * 2))
    done
done




# gcc -o matmul matmul.c -O3 -lm
# echo "n,t,runtime" > matmul_O3.csv
# n=1024
# t=100

# while [ $n -ge 16 ]; do
#     echo "n=$n"
#     while true; do
#         output=$(./matmul -n $n -t $t -b 16)
#         runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
#         if (( $(echo "$runtime > 30" | bc -l) )); then
#             echo "$n,$t,$runtime" >> matmul_O3.csv
#             break
#         fi
#         t=$((t * 2))
#     done
#     n=$((n / 2))
#     t=$((t * 4))
# done
