#include <iostream>
using namespace std;
// test file that print out all the tokens to the console.

int main(int argc, const char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        cout << "*" << argv[i] << "*" << endl;
    }
}