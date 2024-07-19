import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class ReadFloatMatrix {
    public static void main(String[] args) {
        try {
            File file = new File("matrices.txt");
            Scanner scanner = new Scanner(file);
            
            float[][] matrix = null;
            int n = -1;
            int rowNumber = 0;
            
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                String[] hexFloats = line.split(" ");
                
                if (n == -1) {
                    n = hexFloats.length;
                    matrix = new float[n][n];
                } else if (hexFloats.length != n) {
                    System.out.println("Size mismatch in matrix.");
                    System.exit(1);
                }
                
                for (int i = 0; i < n; i++) {
                    matrix[rowNumber][i] = Float.parseFloat(hexFloats[i]);
                }
                
                rowNumber++;
            }
            
            scanner.close();
            
            // Print the matrix to verify it was read correctly
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    System.out.print(matrix[i][j] + " ");
                }
                System.out.println();
            }
        } catch (FileNotFoundException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }
    }
}
