<!-- When dealing with the first requirement, the initial inplement was to use the built-in functionality of scanf to get the two numbers and the operator. 
```c
#include <stdio.h>
#include <string.h>
#define MAX_LIMIT 20
int main(){
    int num1, num2;
    char op;
    scanf("%d %c %d", &num1, &op, &num2);
    switch(op){
        case '+':
        printf("%d + %d = %d\n", num1, num2, num1 + num2);
        break;
        case '-':
        printf("%d - %d = %d\n", num1, num2, num1 - num2);
        break;
        case '*':
        printf("%d * %d = %d\n", num1, num2, num1 * num2);
        break;
        case '/':
        if(num2 == 0){
            printf("Error: Division by zero\n");
        } else {
            printf("%d / %d = %d\n", num1, num2, num1 / num2);
        }
        break;
        default:
        printf("Error: Invalid operator\n");
    }
    return 0;
}
```
However, when trying to fulfill the later demands, such approach seems to fail to get the correct number:
```c
scanf("%lf %c %lf", &num1, &op, &num2);
printf("num1: %lf, op: %c, num2: %lf\n", num1, op, num2);
```
Therefore, another approach is proposed here:
```c
    char string [256];
    fgets(string, sizeof(string), stdin);
    const char s[2] = " ";
    num1 = atof(strtok(string, s));
    op = *strtok(0, " ");
    num2 = atof(strtok(0, " "));
```

```c
Number parseNumber(char * string){
    Number num;
    if (strchr(string, '.') != NULL|| strchr(string, 'e') != NULL) {
        num.type = DOUBLE;
        num.value.d = atof(* string);
    } else {
        num.type = INT;
        num.value.i = atoi(* string);
    }
    return num;
}
```
```c
    char result_char[MAX_LIMIT];
    if (result.type == INT) {
        sprintf(result_char, "%d\n", result.value.i);
    } else {
        sprintf(result_char, "%lf\n", result.value.d);
    }
    strcat(string, result_char);
    printf("%s", string);
```

```
987654321 * 987654321
987654321 * 987654321 = -238269855
```
To deal with overflow in the problem, the simplest way would be setting the variables type as `long long int`. In this way, the calculation requirement in the project is met. Similar approach could be applied to scientific  -->

# Project 1 Report: Calculator
*By Feiyang ZHENG*

## Overview

The project is a command-line calculator application written in C. It can evaluate mathematical expressions provided either as command-line arguments or interactively, one per line. The calculator supports basic arithmetic operations (+, -, *, /), parentheses for grouping, and the use of the previous answer in subsequent calculations.

## Features Outline

1. **Basic Arithmetic Operations**: The calculator can perform addition, subtraction, multiplication, and division operations.

2. **Parentheses for Grouping**: The calculator respects the order of operations and allows the use of parentheses to group operations.

3. **Use of Previous Answer**: The calculator stores the result of the last calculation and allows it to be used in subsequent calculations by typing 'ans'.

4. **Precision Control**: The calculator dynamically adjusts the precision of the output based on the number of decimal places in the input. The user can also manually set the precision with the 'prec' command.

5. **Error Handling**: The calculator checks for and reports various error conditions, such as division by zero, mismatched parentheses, and invalid characters in the input.

## Implementations
### Arithmetic Operations and Basic Error Handling
<!-- When dealing with the first requirement, the initial inplement was to use the built-in functionality of scanf to get the two numbers and the operator. 
```c
scanf("%d %c %d", &num1, &op, &num2);
```

```bash
-1+3
-1+3 = 2
100--1
100--1 = 101
23+-1
23+-1 = 22
``` -->
All the arithmetic operation in the project guideline can be accomplised without error. Beyond that, the calculator has additional arithmetic ability to handle input expressions with more then one operator with or without spaces.
```c
./calculator
Enter expressions to evaluate, one per line:
2 + 3
2+3 = 5
2 - 3
2-3 = -1
2 * 3
2*3 = 6
2 / 3
2/3 = 0.66666667
3.14/0
A number cannot be divided by zero.
a*2
Invalid character: a
987654321 * 987654321
987654321*987654321 = 975461057789971041
1.0e200 * 1.0e200
1.0e200*1.0e200 = 1.0e+400
quit
```
### Parentheses for Grouping
The calculator uses a stack-based algorithm to evaluate expressions. It scans the input from left to right, pushing numbers onto a value stack and operators onto an operator stack. When it encounters a closing parenthesis or an operator with lower precedence than the top of the operator stack, it pops and evaluates the top operation, pushing the result back onto the value stack. It can also detect the mismatch of parentheses.
```c
else if (expression[i] == '(') { // push open parenthesis
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
}
```

The calculator uses the `strtoflt128` function to parse numbers from the input, and keeps track of the number of decimal places to control the precision of the output.

### Use of Previous Answer
It uses a separate variable `ans` to store the result of the last calculation, which can be used in subsequent calculations by typing 'ans'.
```c
if (expression[i] == 'a'&& expression[i+1] == 'n' && expression[i+2] == 's') {
   values[++valTop] = ans;
   i+=2;
   precision = ans_precision > precision? ans_precision : precision;
   ans_called = true;
}
```
```bash
1
1 = 1
ans+1
ans+1 = 2
ans+1
ans+1 = 3
```
### Precision Control
The implement of precision is done by setting a global variable `int precision` which will vary according to the input expression and user setting. The default user precision is set to zero, however, the output would have at least the same number of decimal as the input numbers. User can increase the number of decimal displayed by command `prec`. To match the output of the project guideline, the precision would increase to at least 8 digits when division is used in the input.
```bash
2+3
2+3 = 5
prec 2
Precision set to 2
2+3
2+3 = 5.00
2/3
2/3 = 0.66666667
```
Aside from the number of displayed decimals, the program will also automatically decide whether to use scientific notation based on the user input.
```
1.0e3 + -1e2   
1.0e3+-1e2 = 9.0e+02
```

### Implement of Float-128
```
987654321 * 987654321
987654321 * 987654321 = -238269855
```
To deal with overflow in the problem, the simplest way would be setting the variables type as `long long int`. In this way, the calculation requirement in the project is met. Similar approach could be applied to double. Since `__float128` has 112 bits of significands, is can substitude the occation of `long long int`, which is the same as `int64_t`.

```
  if (strstr(expression,"e")!= NULL||(ans_called&&ans_is_scientific)) {
        quadmath_snprintf(buf, sizeof buf, "%.*Qe", precision,ans);// printf can not directly print float128 type
        printf("%s\n", buf);
        ans_is_scientific = true;
    }else {
        quadmath_snprintf(buf, sizeof buf, "%.*Qf", precision,ans);
        printf("%s\n", buf);
    }
```
To use [`__float128`](https://gcc.gnu.org/onlinedocs/libquadmath/Math-Library-Routines.html#Math-Library-Routines), special functions and constants from `quadmath.h` must be used:
- `strtoflt128()`: from string to float128.
- `isnanq()`: `isnan()` for float128.
- `quadmath_snprintf()` printf can not directly print float128 type, therefore the function is introduced to tranform float128 back to string.
- `FLT128_MAX`: the largest finite number that can be stored by `__float128`, exceeding which will cause overflow error.
- `FLT128_MIN`: the smallest positive number that can be stored by `__float128`, exceeding which will cause underflow error.
## What I Have Learnt from the Project
- **Pointer**
  Through this project of building a calculator from scratch, I have obtain the basic idea of programming language `C`. Char pointers are heavily used in the whole project, from my understanding, a pointer stores the address of the memory where the data is actually located. Therefore by adding or subtracting the pointer, the program can access the memory next to or near current location. 
  ```c
  for (char* ptr = &expression[i]; ptr < end; ptr++) {
                  if (decimal) {
                      if (isdigit(*ptr)) {
                          val_precision++;//move pointer to the next position
                      }
  ...
  ```
  What's more, the presence of pointer makes it far less computationally expensive to make a stack for continuous arithmetic operation.

- **Stack**
  In this calculator project, a stack data structure is used to manage the order of operations and handle nested expressions. The stack is defined as a struct with an integer top and a character array arr of size MAX_SIZE. The top variable keeps track of the top element in the stack, and the arr array is used to store the elements of the stack.
  ```c
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
  ```

- **The Use of NAN**
  I learnt to deal with illegal error by actively returning `NAN` in calculation. Due to the implement of continuous arithmetic calculation, the core calculation function `__float128 calculate(__float128 a, __float128 b, char op)` will be called multiple times in a single evaluation of the input. Under such circumstance,returning a value as error code would not be useful since the returned value would only be treated as a normal number which will be part of the later calculation; The use of global variable could be a solution, but the code would look clumsy and hard to track; directly exiting the program would be kind of bummer since this would stop the program even in interactive mode. When I asked `copilot` how to deal with this situation, it suggested the implement of `NAN`: The arithmetic calculation of any numbers with it will always return itself, this assures the final outcome of the evaluation will be `NAN`. 
  ```c
  ...
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
          precision = (8 > precision) ? 8 : precision; // the same precision as the project guideline // the same precision as the project guideline
          return a / b;
      } else { // x/0 not allowed
          printf("A number cannot be divided by zero.\n");
          return NAN;
      }
  ```
  When the program detects the presence of `NAN`, it will not print the wrong answer, leaving only error reported.
  ```c
  if (!isnanq(ans)) {
    printf("%s = ", expression); // echo the expression
    ...
    }
    ```

### Conditional Operator
I did not understand why so many functions in `C` use `int` as output until I try to write this line of code. Conditional Operator will execute `sscanf` and assign it to `user_precision` if the aforementioned function successfully retrieve an integer.
```c
user_precision = sscanf(expression, "prec %d", &p)? p : user_precision;
// if the user input is not a number, then keep the previous precision
```
## Limitations

Due to the use of `quadmath.h`, this program has to be compiled with flag `-lquadmath` enabled when using gcc.

Although type `long double` or `__float128` can handle the multiplication like `987654321*987654321` without losing precision, such a kind of implement would fail if the input is even larger, in this situation, the program will produce a warning:
```
./calculator 987654321987654321 * 987654321987654321
The result might be inaccurate
987654321987654321*987654321987654321 = 975461059740893157555403139789971072
```
 According to [IEEE 754](https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format), quadruple-precision binary floating-point format is stored as followed:

![binary128]([image/IEEE_754_Quadruple_Floating_Point_Format.png](https://upload.wikimedia.org/wikipedia/commons/thumb/2/24/IEEE_754_Quadruple_Floating_Point_Format.svg/1200px-IEEE_754_Quadruple_Floating_Point_Format.svg.png))

we can see that the number consists of a sign bit, a 15-bit exponent and a 112-bit significand. With this significand, `float128` can actually be used as an "`int113`". By simple calculation, we can know that this calculator can handle $(2^{112} \approx 10^{34})$ decimals of precision, which is already larger than most of the precticle use. 

## Alternatives
Although using external library like `mpfr.h` will allow calculation of arbitary precision, Prof. Yu might not have the library installed. Since only a single `.c` file is permitted in this project, such a project is abandoned. 

Another possible solution to infinite precision calculation would be write the program to store and calculate all expressions and numbers in string, but this is hardly an efficient approach and would make the program hard to read or maintain. For the mental health of my own and Prof. Yu, I decide not to do so.

## Use of GPT
Github Copilot had been a very useful coach during the process, it teached me how to use pointers, string related functions and the use of `NAN`. Besides, it is very good at help writing comments for code, in most cases I only have to type `//` with the first few words, which is already adequate for it to generate the whole comments, as a result, I wrote way more comments than I used to. However, Copilot stills has a lot of errors, for example, it could not write the code for detecting overflow and underflow correctly, for it seems to misunderstand the concept of underflow.

 Also, New Bing Copilot was handy for finding documentation of the functions, it is also the one who suggest me to use `__float128` and to implement `mpfr.h` for the project.

## Future Work

External packages for higher precision like [`gmp.h`](https://gmplib.org/) and [`mpfr.h`](https://www.mpfr.org/index.html) can allow definition of float number and integer of arbitary precision. In fact, the code can be simply rewrite to use the library with `mpfr_strtofr` and `mpfr_printf`.

What's more, The user interface could also be improved, for example, by adding a history feature from <readline.h> that allows the user to recall and edit previous calculations.
