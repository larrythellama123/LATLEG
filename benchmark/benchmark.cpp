//run multiple client_AI
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>   
#include <chrono>
#include <thread>


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

    )
}

int main(){

    vector<pid_t> vec;
    for(int i=0; i< 5; i++){
        pid_t pid = fork();
        vec.push_back(pid);
        if(pid < 0){
            cerr<"process has failed"<<endl;
            return;
        }
        if(pid == 0){
            char *args[] = {(char*)"./client_AI", NULL};
            execvp(args[0],args);
            cerr<<"Failed to launch task"<<endl;
            return 1;
        }
    }
    
    std::cout << "Launched background program with PID: " << pid << "\n";
    std::cout << "Main program is still running and unblocked...\n";
    
    

    uint64_t start = cycle_start();
    char *args[] = {(char*)"./server", NULL};
    execvp(args[0],args);
    system("")
    uint64_t end = cycle_end();
    cout<<"total cycles"<<end -  start<<endl;
    cout<<"stopping the external program now"<<endl;
    for(const auto& pid : vec){
        kill(pid, SIGTERM);
    }





    return 0;
}



//run rtdsc check on server program to see cycles per operation and total time used (use my own software)
