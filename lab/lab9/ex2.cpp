# include <iostream>
using namespace std;

class ConstMember {
    private:
        // const 
        int m_a;
    public:
        ConstMember(int a):m_a(a){}

    void display(){
        cout << "The value of the constant member variable m_a is: " << m_a <<endl;
    }

};

int main() 
{
    ConstMember o1(100);
    ConstMember o2(200);
    
    o1.display();
    o2.display();

    // o1 = o2;
    //error: use of deleted function ‘ConstMember& ConstMember::operator=(const ConstMember&)’
    // const will delete copy method
    return 0;
}