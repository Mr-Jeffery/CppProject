# include <iostream>
using namespace std;

class complex {
    private:
        double real;
        double imag;
    public:
        complex(double r, double i):real(r), imag(i){}
        void display(){
            cout << "The complex number is: " << real << " + " << imag << "i" << endl;
        }
        complex operator+(const complex &c){
            return complex(real + c.real, imag + c.imag);
        }

        complex operator-(const complex &c){
            return complex(real - c.real, imag - c.imag);
        }
};

int main(){
    complex c1(3.0, 4.0);
    complex c2(2.0, 3.0);
    complex c3 = c1 + c2;
    complex c4 = c1 - c2;
    c3.display();
    c4.display();
    return 0;
}