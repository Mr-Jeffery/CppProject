class Base
{
    private:
        int x;
    protected:
        int y;
    public:
        int z;
        void funBase (Base& b)
        {
            ++x;
            ++y;
            ++z;
            ++b.x;
            ++b.y;
            ++b.z;
        }
};
class Derived:public Base
{
    public:
        void funDerived (Base& b, Derived& d)
        {
            // ++x;//private
            ++y;
            ++z;
            // ++b.x;// can't access protected members of another instance
            // ++b.y;
            // ++b.z;
            // ++d.x;//private
            ++d.y;
            ++d.z;
        }
};
void fun(Base& b, Derived& d)
{
    // ++x;// not declared in this scope
    // ++y;
    // ++z;
    // ++b.x;//private
    // ++b.y;//protected
    ++b.z;
    // ++d.x;//private
    // ++d.y;//protected
    ++d.z;
}