#include <iostream>
#include <memory>
using namespace std;
int main(){
    double *p_reg = new double(5);
    shared_ptr<double> pd(p_reg); // can not use pd = p_reg directly
    cout << "*pd = " << *pd << endl;
    shared_ptr<double> pshared = pd; // Copy pd, not creating another
    cout << "*pshared = " << *pshared << endl;
    string str("Hello World!"); //“
    shared_ptr<string> pstr = make_shared<string>(str); 
    cout << "*pstr = " << *pstr << endl;
    return 0;
}
