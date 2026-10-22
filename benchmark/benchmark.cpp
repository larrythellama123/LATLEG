//run multiple client_AI
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>   
#include <chrono>
#include <thread>
#include <vector>


using namespace std;

inline uint64_t cycle_start() {
    uint32_t lo, hi;
    asm volatile(
        "cpuid\n\t"
        "rdtsc"
        : "=a"(lo), "=d"(hi)
        : "a"(0)
        : "rbx", "rcx"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

inline uint64_t cycle_end(){
    uint32_t lo, hi, aux;
    asm volatile(
        "rdtscp\n\t"
        "mov %%eax, %0 \n\t"
        "mov %%edx, %1 \n\t"
        "cpuid"
        :"=r"(lo), "=r"(hi), "=c"(aux)
        :
        :"rax", "rbx", "rdx"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

int main(){

    vector<pid_t> vec;
    for(int i=0; i< 5; i++){
        pid_t pid = fork();
        vec.push_back(pid);
        if(pid < 0){
            cerr<<"process has failed"<<endl;
            return -1;
        }
        if(pid == 0){
            char *args[] = {(char*)"./client_AI_headless", NULL};
            execvp(args[0],args);
            cerr<<"Failed to launch task"<<endl;
            return 1;
        }
    }
    
    
    
    
    cout<<"1 debug"<<endl;
    uint64_t start = cycle_start();
    cout<<"2 debug"<<endl;

    pid_t server_pid = fork();
    if(server_pid < 0){
        cerr<<"server process has failed"<<endl;
        return -1;
    }
    if(server_pid == 0){
        char *args[] = {(char*)"./server", NULL};
        cout<<"Launching server task"<<endl;
        execvp(args[0],args);
        cerr<<"Failed to launch server task"<<endl;
        return 1;
    }
    else{
        cout<<"3 debug"<<endl;

        this_thread::sleep_for(chrono::seconds(2));
        cout<<"4 debug"<<endl;
        uint64_t end = cycle_end();
        kill(server_pid,SIGTERM);
        cout<<"total cycles: "<<end -  start<<endl;
        cout<<"stopping the external program now"<<endl;
        for(const auto& pid : vec){
            kill(pid, SIGTERM);
        }      
    } 
    return 0;
}



