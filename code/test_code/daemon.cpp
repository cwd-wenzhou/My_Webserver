#include <unistd.h>
#include <iostream>
using namespace std;
int main(int argc, char const *argv[])
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("userid is %d,effective userid is : %d\n",uid,euid);

    daemon(1,0);
    int a;
    cin>>a;
    cout<<"cwd write"<<a+3<<endl;
    uid = getuid();
    euid = geteuid();
    printf("userid is %d,effective userid is : %d\n",uid,euid);
    return 0;
}
