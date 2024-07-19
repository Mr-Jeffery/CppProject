clear A B C;
n = 8e3;
A = single(ones(n,n));
B = single(ones(n,n));
tic
C = A*B;
toc