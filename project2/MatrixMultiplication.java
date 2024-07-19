// import java.util.Random;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class MatrixMultiplication {
    // public static void multiply(float[][] a, float[][] b, float[][] result, int n) {
    //     for (int i = 0; i < n; i++) {
    //         for (int j = 0; j < n; j++) {
    //             result[i][j] = 0;
    //             for (int k = 0; k < n; k++)
    //                 result[i][j] += a[i][k] * b[k][j];
    //         }
    //     }
    // }

    public static void blockMultiply(float[][] a, float[][] b, float[][] result, int n, int blockSize) {
        for (int i = 0; i < n; i += blockSize) {
            for (int j = 0; j < n; j += blockSize) {
                for (int k = 0; k < n; k += blockSize) {
                    for (int ii = i; ii < i + blockSize && ii < n; ii++) {
                        for (int jj = j; jj < j + blockSize && jj < n; jj++) {
                            for (int kk = k; kk < k + blockSize && kk < n; kk++) {
                                result[ii][jj] += a[ii][kk] * b[kk][jj];
                            }
                        }
                    }
                }
            }
        }
    }

    public static void main(String[] args) {
        int n = 2;
        int t = 1;
        int blockSize = 16;
        if (args.length<=0){
            System.out.println(" Input format incorrect");
            System.exit(1);
        } else {
            n = Integer.valueOf(args[0]);
            t = Integer.valueOf(args[1]);
            blockSize = Integer.valueOf(args[2]);
            System.out.println("n: " + n + ", t: " + t + ", block size: " + blockSize);
        }

        float[][] a = new float[n][n];
        float[][] b = new float[n][n];
        float[][] result = new float[n][n];

        try {
            File file = new File("matrices.txt");
            Scanner scanner = new Scanner(file);
            
            for (int i = 0; i < n; i++) {
                String line = scanner.nextLine();
                String[] hexFloats = line.split(" ");
                for (int j = 0; i < n; i++) {
                    a[i][j] = Float.parseFloat(hexFloats[i]);
                }
            }
            for (int i = 0; i < n; i++) {
                String line = scanner.nextLine();
                String[] hexFloats = line.split(" ");
                for (int j = 0; i < n; i++) {
                    b[i][j] = Float.parseFloat(hexFloats[i]);
                }
            }
            
            scanner.close();
        } catch (FileNotFoundException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }

        // Random rand = new Random(0);
        // for (int i = 0; i < n; i++) {
        //     for (int j = 0; j < n; j++) {
        //         a[i][j] = rand.nextFloat();
        //         b[i][j] = rand.nextFloat();
        //     }
        // }


        long startTime = System.nanoTime();
        // for (int i = 0; i < t; i++){
        //     multiply(a, b, result, n);
        // }
        long endTime = System.nanoTime();
        // System.out.println("Time spent: " + (endTime - startTime) / 1e9);

        startTime = System.nanoTime();
        for (int i = 0; i < t; i++){
            blockMultiply(a, b, result, n, 8);
        }
        endTime = System.nanoTime();
        System.out.println("Time spent: " + String.format("%.6f", (endTime - startTime) / 1e9));
    }
}
