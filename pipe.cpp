//gitlab version
#include <iostream>
#include <signal.h>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

//refer: https://blog.csdn.net/yake25/article/details/7447086
//refer: https://github.com/eceuwaterloo/ece650-cpp/blob/master/using_pipe.cpp
//refer: https://stackoverflow.com/questions/47068948/best-practice-to-use-execvp-in-c
//refer: https://www.cnblogs.com/mickole/p/3187409.html
//refer: https://stackoverflow.com/questions/27213580/difference-between-char-argv-and-char-argv-for-the-second-argument-to-main
int rgen(int argc, char *argv[]){
    argv[0] = (char*) "rgen";
    execv("rgen", argv);
    cerr << "Error: rgen" << endl;
    return 0;
}

int a1(void){
    char* argv[] = {"python", "ece650-a1.py", nullptr};
    execvp("python", argv);
    cerr << "Error: a1" << endl;
    return 0;
}

int a2(void){
    char* argv[2];
    argv[0] = (char*) "ece650-a2";
    argv[1] = nullptr;
    execv("ece650-a2", argv);
    cerr << "Error: a2" << endl;
    return 0;
}

int procD(void){
    while(!cin.eof()){
        string line;
        getline(cin, line);
        if(line.size() > 0){
            cout << line << endl;
        }
    }
    return 0;
}

int main (int argc, char **argv) {
    vector<pid_t> kids;
    //create pipe
    int rgentoA1[2];
    pipe(rgentoA1);
    
    pid_t child_pid;
    child_pid = fork();
    
    if(child_pid == 0){
        dup2(rgentoA1[1], STDOUT_FILENO); //newfd
        close(rgentoA1[0]); //close child read
        close(rgentoA1[1]); //close child stdout
        return rgen(argc, argv);
    }
    else if(child_pid < 0){
        cerr << "Error: could not fork\n" << endl;
        return 1;
    }

    //create pipe
    int A1toA2[2];
    pipe(A1toA2);
    
    kids.push_back(child_pid);
    
    child_pid = fork();
    if(child_pid == 0){
        dup2(rgentoA1[0], STDIN_FILENO);
        close(rgentoA1[1]);
        close(rgentoA1[0]);
        
        dup2(A1toA2[1], STDOUT_FILENO);
        close(A1toA2[0]);
        close(A1toA2[1]);
        return a1();
    }
    else if(child_pid < 0){
        cerr << "Error: could not fork\n" << endl;
        return 1;
    }
    
    kids.push_back(child_pid);
    
    child_pid = fork();
    if(child_pid == 0){
        dup2(A1toA2[0], STDIN_FILENO);
        close(A1toA2[1]);
        close(A1toA2[0]);
        return a2();
    }
    else if(child_pid < 0){
        cerr << "Error: could not fork\n" << endl;
        return 1;
    }
    
    kids.push_back(child_pid);
    child_pid = 0;
   
    dup2(A1toA2[1], STDOUT_FILENO);
    close(A1toA2[0]);
    close(A1toA2[1]);
    
    int res = procD();
    
    for(pid_t k : kids){
        int status;
        kill(k, SIGTERM);
        waitpid(k, &status, 0);
    }
    return res;
}
