gcc -o matmul matmul.c -O3 -lm -march=native
echo "native_O3 = [" > native.log
n=16
t=100
b=16

while [ $n -le 1024 ]; do
    echo "n=$n"
    output=$(./matmul -n $n -t $t -b $b)
    runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
    echo "$n,$t,$runtime;" >> native.log
    n=$((n * 2))
done
echo "];" >> native.log