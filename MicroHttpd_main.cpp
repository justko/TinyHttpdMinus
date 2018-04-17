#include"MicroHttpd.h"
int main(){
    MicroHttpd server(10086);
    server.looping();
    return 0;
}