#include <iostream>
using namespace std;

class Demo {
public:
    static int num;
    void display(){
        cout << "The value of the static member variable num is: " << num <<endl;//undefined reference to `Demo::num'
    }
};

int Demo::num = Demo::num ? Demo::num : 0;

int main() 
{
    Demo obj;
    obj.display();
    return 0;
}