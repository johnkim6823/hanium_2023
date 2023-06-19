#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

using namespace std;

#include "server.h"

int main(){
    if(!initServer()){
        cout << "Failed init socket!" << endl;
        return -1;
    }

	while(1){
		sleep(1);
	}

    termServer();
}
