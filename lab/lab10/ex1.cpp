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
        complex(){
            real = 0;
            imag = 0;
        }

        complex operator+(const complex &c){
            return complex(real + c.real, imag + c.imag);
        }

        complex operator-(const complex &c){
            return complex(real - c.real, imag - c.imag);
        }

        complex operator*(const complex &c){
            return complex(real*c.real - imag*c.real, imag*c.real + real*c.imag);
        }   

        complex operator~(){
            return complex(real, -imag);
        }

        complex operator/(const complex &c){
            return complex((real*c.real + imag*c.imag)/(c.real*c.real + c.imag*c.imag), (imag*c.real - real*c.imag)/(c.real*c.real + c.imag*c.imag));
        }

        complex operator=(const complex &c){
            real = c.real;
            imag = c.imag;
            return *this;
        }

        bool operator==(const complex &c){
            return (real == c.real && imag == c.imag);
        }

        bool operator!=(const complex &c){
            return (real != c.real || imag != c.imag);
        }

        friend ostream& operator<<(ostream &os, const complex &c){
            os << c.real << " + " << c.imag << "i";
            return os;
        }

        friend istream& operator>>(istream &is, complex &c){
            cout << "Enter real part: ";
            is >> c.real;
            cout << "Enter imaginary part: ";
            is >> c.imag;
            return is;
        }


};


int main(){
    complex c1(3.0, 4.0);
    complex c2(2.0, 3.0);
    cout << "c1: " << c1 << endl;
    cout << "c2: " << c2 << endl;
    cout << "~c1: " << ~c1 << endl;
    cout << "c1 + c2: " << c1 + c2 << endl;
    cout << "c1 - c2: " << c1 - c2 << endl;
    cout << "c1 * c2: " << c1 * c2 << endl;
    cout << "c1 / c2: " << c1 / c2 << endl;
    cout << "c1 == c2: " << (c1 == c2) << endl;
    cout << "c1 != c2: " << (c1 != c2) << endl;

    complex c3 = c1, c4 = c2;
    cin >> c3;
    cout << "c: " << c3 << endl;
    cout << "Before assignment c4: " << c4 << endl;
    c4 = c3;
    cout << "After assignment c4: " << c4 << endl;

    return 0;
}