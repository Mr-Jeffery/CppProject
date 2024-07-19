gcc -o matmul matmul.c -O3 -lm
echo "block = [" > block.log
n=512
t=10
b=1

for b in {1,2,4,8,16,32,64,128,256}; do
    echo "b=$b"
    output=$(./matmul -n $n -t $t -b $b)
    runtime=$(echo "$output" | grep -oP 'Time spent: \K[0-9.]+')
    echo "$b,$t,$runtime;" >> block.log
done
echo "];" >> block.log