#include <iostream>

union twonumbers 
{
  int n[2];
  double d;
};

using namespace std;

int main()
{
  twonumbers tn;
  tn.d = 1.23;
  tn.n[0] = 0;
  tn.n[1] = 0;
  cout << tn.d << endl;

  return 0;
}