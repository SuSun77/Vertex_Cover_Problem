#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

//refer: https://blog.csdn.net/stpeace/article/details/45829161
int genRandom(int small, int large) {
  unsigned int randNum = 0;
  int randomfd = open("/dev/urandom", O_RDONLY);
  if (randomfd == (-1)) {
      fprintf(stderr, "Error: open /dev/urandom error\n");
      exit(1);
  }
  read(randomfd, &randNum, sizeof(unsigned int));
  randNum = (randNum %= (large - small + 1)) + small;
  close(randomfd);
  return randNum;
}

int checkLength(int pAx, int pAy, int pBx, int pBy, int pCx, int pCy, int pDx, int pDy){
    int i = sqrt(sqrt(pAx - pBx) + sqrt(pAy - pBy)) - sqrt(sqrt(pCx - pDx) + sqrt(pCy - pDy));
    if(i == 0){
        return 0;//same
    }
    else{
        return (i > 0)? 1: 2; //1: AB > CD 2: AB < CD
    }
}
    
//refer: https://blog.csdn.net/sinat_38972110/article/details/82115637
int chenkCollinear(int pAx, int pAy, int pBx, int pBy, int pCx, int pCy){
    if(((pAx == pBx) && (pBx == pCx)) || ((pAy == pBy) && (pBy == pCy))){
        return 1;
    }
    else{
        if((pCy - pAy) * (pBx - pAx) - (pBy - pAy) * (pCx - pAx) == 0){
            return 1;
        }
    }
    return 0;
}

//refer: https://blog.csdn.net/liangzhaoyang1/article/details/51088475
int checkInOrOut(int pAx, int pAy, int pBx, int pBy, int pCx, int pCy){
    if(fmin(pAx, pBx) < pCx && pCx < fmax(pAx, pBx) && fmin(pAy, pBy) < pCy && pCy < fmax(pAy, pBy)){
        return 1;
    }
    else{
        return 0;
    }
}

int isZeroLength(int pAx, int pAy, int pBx, int pBy){
    if((pAx == pBx) && (pAy == pBy)){
        return 0;//segment's length = 0
    }
    else{
        return 1;//不重合
    }
}

int checkOverlap(int pAx, int pAy, int pBx, int pBy, int pCx, int pCy, int pDx, int pDy){
    if((isZeroLength(pAx, pAy, pBx, pBy) == 1) && (isZeroLength(pCx, pCy, pDx, pDy) == 1)){
        if((chenkCollinear(pAx, pAy, pBx, pBy, pCx, pCy) == 1) && (chenkCollinear(pAx, pAy, pBx, pBy, pDx, pDy) == 1)){
            if(checkLength(pAx, pAy, pBx, pBy, pCx, pCy, pDx, pDy) == 0){
                if(((pAx == pCx) && (pAy == pCy) && (pBx == pDx) && (pBy == pDy)) || ((pAx == pDx) && (pBx == pCx) && (pAy == pDy) && (pBy == pCy))){
                    return 1; // the two segment totally overlap -> one segment
                }
                else{
                    if((checkInOrOut(pAx, pAy, pBx, pBy, pCx, pCy) == 1) || (checkInOrOut(pAx, pAy, pBx, pBy, pDx, pDy) == 1)){
                        return 1;
                    }
                    else{
                        return 0;
                    }
                }
            }
            else{
                if(checkLength(pAx, pAy, pBx, pBy, pCx, pCy, pDx, pDy) == 1){ // len(AB) > len(CD)
                    if((checkInOrOut(pAx, pAy, pBx, pBy, pCx, pCy) == 1) || (checkInOrOut(pAx, pAy, pBx, pBy, pDx, pDy) == 1)){
                        return 1;
                    }
                    else{
                        return 0;
                    }
                }
                else{
                    if(checkLength(pAx, pAy, pBx, pBy, pCx, pCy, pDx, pDy) == 2){ // len(AB) < len(CD)
                        if((checkInOrOut(pCx, pCy, pDx, pDy, pAx, pAy) == 1) || (checkInOrOut(pCx, pCy, pDx, pDy, pBx, pBy) == 1)){
                            return 1;
                        }
                        else{
                            return 0;
                        }
                    }
                    else{
                        return 0;
                    }
                }
            }
        }
        else{
            return 0;
        }
    }
    else{
        return 1;
    }
}

//refer: https://github.com/eceuwaterloo/ece650-cpp/blob/master/using_getopt.cpp
int main(int argc, char *argv[]){
    //refer: https://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
    setvbuf(stdout, NULL, _IONBF, 0);
    int s = 10;
    int n = 5;
    int l = 5;
    int c = 20;
    int opt = 0;
    int index;
    
    while((opt = getopt(argc, argv, "s:n:c:l:")) != -1)
        switch(opt){
            case 's':
                s = atoi(optarg);
                if(s < 2){
                    fprintf(stderr, "Error: s cannot lower than 2\n");
                    exit(EXIT_SUCCESS);
                }
                break;
            case 'n':
                n = atoi(optarg);
                if (n < 1) {
                    fprintf(stderr, "Error: s cannot lower than 1\n");
                    exit(EXIT_SUCCESS);
                }
                break;
            case 'l':
                l = atoi(optarg);
                if (l < 5){
                    fprintf(stderr, "Error: s cannot lower than 5\n");
                    exit(EXIT_SUCCESS);
                }
                break;
            case 'c':
                c = atoi(optarg);
                if (c < 1) {
                    fprintf(stderr, "Error: s cannot lower than 1\n");
                    exit(EXIT_SUCCESS);
                }
                break;
        }
    
    if (optind < argc) {
        cout << "Found positional arguments\n";
        for (index = optind; index < argc; index++)
            cout << "Non-option argument: " << argv[index] << "\n";
    }
    
    int realStreetNum, realWaitTime, street, vertex, streetID;
    
    while(true){
        realStreetNum = genRandom(2, s);
        realWaitTime = genRandom(5, l);
        
        char streetName[realStreetNum][15];
        int lineSeg[realStreetNum];
        int point[realStreetNum][2000];
        
        for(street = 0; street < realStreetNum; street ++){
            streetID = street;
            sprintf(streetName[street], "street\t%d", streetID);//name street
            //gen point for street
            lineSeg[street] = genRandom(1, n);//current
            
            int i = lineSeg[street] * 2;//(x,y)
            for(vertex = 0; vertex <= i; vertex = vertex + 2){
                point[street][vertex] = genRandom(-c, c); //x
                point[street][vertex + 1] = genRandom(-c, c); //y
                
                if(street == 0 && vertex == 2){//check point
                    int tryGenerateNum = 1;
                    while(point[street][0] == point[street][vertex] && point[street][1] == point[street][vertex + 1]){
                        tryGenerateNum ++;
                        if(tryGenerateNum > 25){
                            fprintf(stderr, "Error: failed to generate valid input for 25 simultaneous attempts\n");
                            exit(3);
                        }
                        point[street][vertex] = genRandom(-c, c);
                        point[street][vertex + 1] = genRandom(-c, c);
                    }
                }
                else{
                    if(street == 0 && vertex > 2){//check point and overlap
                        int j = 0;
                        while(j < vertex - 2){
                            int tryGenerateNum2 = 1;
                            while((checkOverlap(point[street][j], point[street][j + 1], point[street][j + 2], point[street][j + 3], point[street][vertex - 2], point[street][vertex - 1], point[street][vertex], point[street][vertex + 1]) == 1)){
                                tryGenerateNum2 ++;
                                if(tryGenerateNum2 > 25){
                                   fprintf(stderr, "Error: failed to generate valid input for 25 simultaneous attempts\n");
                                    exit(3);
                                }
                                point[street][vertex] = genRandom(-c, c);
                                point[street][vertex + 1] = genRandom(-c, c);
                            }
                            j = j + 2;
                        }
                    }
                    else{//check seg
                        if(street != 0 && vertex >= 2){
                            int k = 0;
                            int tryGenerateNum3 = 1;
                            while(k == 0){
                                int flag = 0;
                                if(point[street][vertex - 2] == point[street][vertex] && point[street][vertex - 1] == point[street][vertex + 1]){//check point
                                    tryGenerateNum3 ++;
                                    if(tryGenerateNum3 > 25){
                                        fprintf(stderr, "Error: failed to generate valid input for 25 simultaneous attempts\n");
                                        exit(3);
                                    }
                                    point[street][vertex] = genRandom(-c, c);
                                    point[street][vertex + 1] = genRandom(-c, c);
                                    continue;
                                }
                                int u = 0;
                                int v = 0;
                                while(u < street){
                                    int z = lineSeg[u] * 2;
                                    while(v < z){
                                        if(checkOverlap(point[u][v], point[u][v + 1], point[u][v + 2], point[u][v + 3], point[street][vertex - 2], point[street][vertex - 1], point[street][vertex], point[street][vertex + 1]) == 1){
                                            flag = 1;
                                            break;
                                        }
                                        v = v + 2;
                                    }
                                    if(flag == 1){
                                        break;
                                    }
                                    u ++;
                                }
                                if(flag == 1){
                                    tryGenerateNum3 ++;
                                    if(tryGenerateNum3 > 25){
                                        fprintf(stderr, "Error: failed to generate valid input for 25 simultaneous attempts\n");
                                        exit(3);
                                    }
                                    point[street][vertex] = genRandom(-c, c);
                                    point[street][vertex + 1] = genRandom(-c, c);
                                    continue;
                                }
                                //check street
                                u = 0;
                                while(u < (vertex - 2)){
                                    if(checkOverlap(point[street][u], point[street][u + 1], point[street][u + 2], point[street][u + 3], point[street][vertex - 2], point[street][vertex - 1], point[street][vertex], point[street][vertex + 1]) == 1){
                                        flag = 1;
                                        break;
                                    }
                                    u = u + 2;
                                }
                                if(flag == 1){
                                    tryGenerateNum3 ++;
                                    if(tryGenerateNum3 > 25){
                                        fprintf(stderr, "Error: failed to generate valid input for 25 simultaneous attempts\n");
                                        exit(3);
                                    }
                                    point[street][vertex] = genRandom(-c, c);
                                    point[street][vertex + 1] = genRandom(-c, c);
                                    continue;
                                }
                                k = 1;
                            }
                        }
                    }
                }
            }
        }
        street = 0;
        while(street < realStreetNum){
            printf("a \"%s\" ", streetName[street]);
            vertex = 0;
            int h = lineSeg[street] * 2;
            while(vertex <= h){
                printf("(%d,%d) ", point[street][vertex], point[street][vertex + 1]);
                vertex = vertex + 2;
            }
            printf("\n");
            street++;
        }
        
        printf("g\n");
        sleep(realWaitTime);
        int q = 0;
        while(q < realStreetNum){
            printf("r \"%s\" \n", streetName[q]);
            q ++;
        }
    }
    return 0;
}

