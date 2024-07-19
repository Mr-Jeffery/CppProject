#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>// imported for isdigit
#include <string.h>// imported for strlen
#include <math.h>// imported for NAN
#include <stdbool.h>// imported for bool
#include <quadmath.h>// imported for float128
// #include <gmp.h>
// #include <mpfr.h>

#define MAX_SIZE 256 // maximum size of the expression
#define INT_MAX  5.1922968585348276285304963292201e33Q // maximum int before lossing precision

int precision=0; // number of decimal places to display
int user_precision = 0; // set by user

__float128 ans = 0; // previous answer
int ans_precision = 0; // number of decimal places in the previous answer
bool ans_is_scientific = false; // whether the previous answer is in scientific notation
bool ans_called = false; // whether the previous answer was called

// Build a Stack to store parenthesis and operators
typedef struct {
    int top;
    char arr[MAX_SIZE];
} Stack;

void push(Stack* stack, char op) {
    if (stack->top < MAX_SIZE - 1) {
        stack->arr[++stack->top] = op;
    }
}

char pop(Stack* stack) {
    if (stack->top >= 0) {
        return stack->arr[stack->top--];
    }
    return '\0';
}


// determine calculation order
int precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return 0;
    }
}

// calculate the result
__float128 calculate(__float128 a, __float128 b, char op) {
    switch (op) {
        case '+': 
            if (
                ((a > 0) && (b > FLT128_MAX - a))// inf
              ||((a < 0) && (b < -FLT128_MAX + a))) {// -inf 
                printf("Overflow error.\n");
                return NAN;
            }
            return a + b;
        case '-': 
            if (
                ((a > 0) && (-b > FLT128_MAX - a))// inf
              ||((a < 0) && (-b < -FLT128_MAX + a))) {// -inf 
                printf("Overflow error.\n");
                return NAN;
            }
            return a - b;
        case '*': 
            if (
                a != 0 && b != 0  && (fabsq(a) > FLT128_MAX / fabsq(b))) {
                printf("Overflow error.\n");
                return NAN;
            }
            if (a != 0 && b != 0  && (fabsq(a) < FLT128_MIN / fabsq(b))) {
                printf("Underflow error.\n");
                return NAN;
            }
            if (precision==0&&fabsq(a*b)>INT_MAX){
                printf("The result might be inaccurate\n");
            }
            return a * b;
        case '/': 
            if (b != 0) {
                if (a != 0  && (fabsq(a) < FLT128_MIN * fabsq(b))) {
                    printf("Underflow error.\n");
                    return NAN;
                }
                if (a != 0  && (fabsq(a) > FLT128_MAX * fabsq(b))) {
                    printf("Overflow error.\n");
                    return NAN;
                }
                precision = (8 > precision) ? 8 : precision; // the same precision as the project guideline
                return a / b;
            } else { // x/0 not allowed
                printf("A number cannot be divided by zero.\n");
                return NAN;
            }
    }
    return 0;
}

void evaluate(char* expression) {
    __float128 values[MAX_SIZE];
    int valTop = -1;
    Stack ops = {-1, {0}};
    int parentheses = 0;// count the number of open parentheses

    for (int i = 0; i < strlen(expression); i++) {

        if (expression[i] == 'a'&& expression[i+1] == 'n' && expression[i+2] == 's') {
            values[++valTop] = ans;
            i+=2;
            precision = precision = (ans_precision > precision) ? ans_precision : precision;;
            ans_called = true;
        } else if (
            isdigit(expression[i]) || expression[i] == '.'
            //negtive number
            || (expression[i] == '-' && (i == 0 || (!isdigit(expression[i-1]) && expression[i-1] != ')')) 
        && (isdigit(expression[i+1]) || isspace(expression[i+1]) || expression[i+1] == '-'))) {

            bool isNegative = false;
            while(expression[i] == '-') {
                isNegative = !isNegative;
                i++;
            }
            char* end;// find the end of the number
            // mpfr_strtofr(&values[++valTop], &expression[i], &end, MPFR_RNDN);
            __float128 val = strtoflt128(&expression[i], &end);

            if (isNegative) {
                val = -val;
            }
            // count the number of decimal places, so that we can display the answer with the same precision
            bool decimal = false;
            int val_precision = 0;
            for (char* ptr = &expression[i]; ptr < end; ptr++) {
                if (decimal) {
                    if (isdigit(*ptr)) {
                        val_precision++;
                    }else if (*ptr == 'e') {
                        break;
                    }else {
                        printf("Error: Invalid number format\n");
                        return;
                    }
                }else if (*ptr == '.') {
                    decimal = true;
                }
            }
            precision = val_precision > precision? val_precision : precision; // set the precision to the maximum number of decimal places with ternary operation

            i += end - &expression[i] - 1;
            values[++valTop] = val;
        } else if (expression[i] == '(') { // push open parenthesis
            parentheses++;
            push(&ops, expression[i]);
        } else if (expression[i] == ')') {// pop all operators until open parenthesis
            parentheses--;
            if (parentheses < 0) {
                printf("Error: Mismatched parentheses in expression\n");
                return;
            }
            while (ops.top != -1 && ops.arr[ops.top] != '(') {
                __float128 val2 = values[valTop--];
                __float128 val1 = values[valTop--];
                char op = pop(&ops);
                values[++valTop] = calculate(val1, val2, op);
            }
            pop(&ops);
        } else if(expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/'){// pop all operators with higher or equal precedence
            while (ops.top != -1 && precedence(ops.arr[ops.top]) >= precedence(expression[i])) {
                __float128 val2 = values[valTop--];
                __float128 val1 = values[valTop--];
                char op = pop(&ops);
                values[++valTop] = calculate(val1, val2, op);
            }
            push(&ops, expression[i]);
        } else {
            printf("Invalid character: %c\n", expression[i]);
            return;
        }
    }

    if (parentheses != 0) {
        printf("Error: Mismatched parentheses in expression\n");
        return;
    }

    while (ops.top != -1) {
        __float128 val2 = values[valTop--];
        __float128 val1 = values[valTop--];
        char op = pop(&ops);
        values[++valTop] = calculate(val1, val2, op);
    }

    ans = values[valTop];
    ans_precision = precision;
    char buf[MAX_SIZE];
    if (!isnanq(ans)) {
        printf("%s = ", expression); // echo the expression
        if (strstr(expression,"e")!= NULL||(ans_called&&ans_is_scientific)) {
            //mpfr_get_str(buf, NULL, 10, precision, ans, MPFR_RNDN);
            quadmath_snprintf(buf, sizeof buf, "%.*Qe", precision,ans);// printf can not directly print float128 type
            printf("%s\n", buf);
            ans_is_scientific = true;
        }else {
            quadmath_snprintf(buf, sizeof buf, "%.*Qf", precision,ans);
            printf("%s\n", buf);
        }
    }
}

char* rm_spaces(char* input) {
    int length = strlen(input);
    char* result = (char*)malloc(length + 1); // +1 for the null-terminator
    int index = 0;

    for (int i = 0; i < length; i++) {
        if (input[i] != ' ') {
            result[index++] = input[i];
        }
    }

    result[index] = '\0'; // End of string
    return result;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        // Concatinate the arguments into a single string
        char expression[MAX_SIZE] = "";
        for (int i = 1; i < argc; i++) {
            strcat(expression, argv[i]);
        }
        evaluate(rm_spaces(expression));
    } else {
        char expression[MAX_SIZE];
        printf("Enter expressions to evaluate, one per line:\n");
        while (1) {
            if (fgets(expression, MAX_SIZE, stdin) == NULL || strcmp(expression, "quit\n") == 0) {
                return 0;
            }            
            if (expression[0] == 'p' && expression[1] == 'r' && expression[2] == 'e' && expression[3] == 'c') {
                int p;
                user_precision = sscanf(expression, "prec %d", &p)? p : user_precision;// if the user input is not a number, then keep the previous precision
                precision = user_precision;
                printf("Precision set to %d\n", user_precision);
            } else {
                expression[strcspn(expression, "\n")] = 0;  // Remove <EOL>
                evaluate(rm_spaces(expression));
                precision=user_precision;
            }
        }
    }
    return 0;
}
