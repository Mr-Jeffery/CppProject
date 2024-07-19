import java.util.Random;

public class Main {
    public static void main(String[] args) {
        Random rand = new Random(12345); // Initialize random number generator with seed = 1
        for(int i = 0; i < 5; i++) {
            // float randNum = rand.nextFloat();
            int randNum = rand.nextInt();
            System.out.println(randNum);
        }
    }
}