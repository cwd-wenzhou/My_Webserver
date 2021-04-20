#include <iostream>
#include <vector>
using namespace std;
int main(int argc, char const *argv[])
{

    vector<int> test;
    vector<int> test1(10);
    vector<int> test2(10,0);
    vector<int>::const_iterator ptr;
    ptr=test1.begin();
    //ptr=12;
    cout<<"test.capacity()=="<<test.capacity()<<endl;
    cout<<"test1.capacity()=="<<test1.capacity()<<endl;
    cout<<"test2.capacity()=="<<test2.capacity()<<endl;
    test.push_back(1);
    cout<<"test.capacity()=="<<test.capacity()<<endl;
    test1.push_back(1);
    cout<<"test1.capacity()=="<<test1.capacity()<<endl;
    test2.push_back(1);
    cout<<"test2.capacity()=="<<test2.capacity()<<endl;
    return 0;
}

