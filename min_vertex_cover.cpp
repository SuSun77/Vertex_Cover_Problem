#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include <fstream>
#include <string.h>
#include <cmath>
#include <numeric>
//#include "pthread_getcpuclockid.h" // There is no this function in MAC system.
using namespace std;

#define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

// build a array to store edge elements.
int edgeList[10000];
int iarray[5000];
int jarray[5000];
vector<int> ivector; // vector to store the first value of each vertex.
vector<int> jvector; // vector to store the second value of each vertex.
vector<int> indexvector; // vector to store the index number of duplicate vertex coordinate.
vector<int> edgeVector; // construct a vector to store the edge list。
vector<int> CVresult2;
vector<int> CVresult1;
vector<long> timeCNF, timeAVC1, timeAVC2; // to store the CPU time.
vector<float> approxResult1, approxResult2;
int EleLength;// the number of edge end and start points.
int VNumber; //the number of vertexs.
int ENumber; //the number of edges.
float resultCNFSize, CVresult2Size, CVresult1Size; // used to store the result size.
int startendList[2];
clockid_t cidCNF, cid1, cid2;
int CNFDONE = 0, AVC1DONE = 0, AVC2DONE = 0;
int Loopcount = 0;
int Trigger = 0;
string sCNF = "CNF-SAT-VC: ";
string sAVC1 = "APPROX-VC-1: ";
string sAVC2 = "APPROX-VC-2: ";

// function to get the input string.
string getString(){
    string gather;
    getline(cin, gather);
    return gather;
}

// function to append edge elements into the edgeList.
void getEdgeArr(string s){
    int result = 0;
    int index = 0;
    for (int i = 0; i < s.size(); ++i)
    {
        if (s[i] == ' ' || s[i] == '<') {
            continue;
        }
        if (s[i] >= '0'&&s[i] <= '9'){
            result= result * 10 + s[i] - 48;
        }
        if ((s[i] == ',' && s[i+1] >= '0'&&s[i+1] <= '9') || s[i] == '>'){
            edgeList[index] = result;
            result = 0;
            index += 1;
        }
    }
    EleLength = index;
    ENumber = index/2;
}

// function to get the number of vertex. Using after input command V.
void getVNumber(string s){
    int result = 0;
    for (int i = 0; i < s.size(); ++i)
    {
        if (s[i] == ' ' || s[i] == '<') {
            continue;
        }
        if (s[i] >= '0'&&s[i] <= '9'){
            result= result * 10 + s[i] - 48;
        }
    }
    VNumber = result;
}

// append i, j (start and end point of edges) into array.
void twoArray(){
    int valuei = 0;
    int valuej = 1;
    for (int i = 0; i < EleLength/2; ++i) {
        iarray[i] = edgeList[valuei];
        valuei += 2;
    }
    for (int j = 0; j < EleLength/2; ++j) {
        jarray[j] = edgeList[valuej];
        valuej += 2;
    }
}

// If input edge nodes exceeds the vertex number, an error will be output.
int eInVertex(){
    int temp = 1;
    for (int i = 0; i < EleLength; ++i) {
        if (edgeList[i] >= VNumber) {
            cerr << "Error: Input edge node does not exist." << endl;
            temp = 0;
            break;
        }
    }
    return temp;
}

// define a function to output the thread time. Should be modified after all.
//static void
timespec pclock(char *msg, clockid_t cid)
{
    struct timespec ts;

    // printf("%s", msg);
    if (clock_gettime(cid, &ts) == -1)
        handle_error("clock_gettime");
    printf("%s%ld\n", msg, ts.tv_nsec); // this is nano second, 1 nsec = 10^-9 sec.
    return ts;
}

// construct a function to detect if return node is an edge vertex.
// 1 means that the node is an edge vertex.
int nodeInVC(int node){
    for (int i = 0; i < ENumber; ++i) {
        if (node == iarray[i] || node == jarray[i]){
            return 1;
        }
    }
    return 0;
}

// Construct a function to implement the reduction and get the vertex cover result.
int getVertexCover(int numVC){
    // allocate on the heap so that we can reset later if needed
    unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
    // construct a matrix to store the atomic propositions.
    vector<vector<Minisat::Lit>> Ttable(VNumber);


    // the n is the number of input vertex.
    // the k is the size of vertex cover (we start the k from 1 to n).
    for (int n = 0; n < VNumber; ++n) {
        for (int k = 0; k < numVC; ++k) {
            Minisat::Lit literal;
            literal = Minisat::mkLit(solver -> newVar());
            Ttable[n].push_back(literal);
        }
    }

    // clause 1: At least one vertex is the ith vertex in the vertex cover.
    // build a vector for literals storing.
    Minisat::vec<Minisat::Lit> literalVector1;
    for (int k = 0; k < numVC; ++k) {
        for (int n = 0; n < VNumber; ++n) {
            literalVector1.push(Ttable[n][k]);
        }
        // add those literals into first clause in solver.
        solver->addClause(literalVector1);
        // clear the literal vector for following loop.
        literalVector1.clear();

        pthread_testcancel();
    }

    // clause 2: No one vertex can appear twice in a vertex cover.
    // (it is not the case that vertex m appears both in positions p and q of the vertex cover.)
    for (int n = 0; n < VNumber; ++n) {
        for (int p = 0; p < numVC - 1; ++p) {
            for (int q = p + 1; q < numVC; ++q) {
                //cout << "p is " << p << " q is " << q << endl;
                solver->addClause(~Ttable[n][p], ~Ttable[n][q]);

                pthread_testcancel();// set a cancelation point.
            }
        }
    }

    // clause 3: No more than one vertex appears in the mth position of the vertex cover.
    // Just like clause2, but change the column to row.
    for (int n = 0; n < numVC; ++n) {
        for (int p = 0; p < VNumber - 1; ++p) {
            for (int q = p + 1; q < VNumber; ++q) {
                solver->addClause(~Ttable[p][n], ~Ttable[q][n]);

                pthread_testcancel();// set a cancelation point.
            }
        }
    }

    // clause 4: Every edge is incident to at least one vertex in the vertex cover.
    // build a vector for literals storing. Just same as clause1.
    Minisat::vec<Minisat::Lit> literalVector4;
    for (int i = 0; i < ENumber; ++i) {
        for (int k = 0; k < numVC; ++k) {
            literalVector4.push(Ttable[iarray[i]][k]);
            literalVector4.push(Ttable[jarray[i]][k]);

            pthread_testcancel(); // set a cancelation point.
        }
        // add those literals into first clause in solver.
        solver->addClause(literalVector4);
        // clear the literal vector for following loop.
        literalVector4.clear();
    }

    vector<int> resultCNF; // construct a vector to store the result from CNF-SAT
    // if the result is a SAT problem, cout the vertex of vertex cover.
    if (solver -> solve() == true){
        //cout << "SAT Result is True" << endl;
        //cout << "VNumber " << VNumber << endl;
        //cout << "numVC " << numVC << endl;
        //printf("CNF-SAT-VC: ");
        for (int m = 0; m < VNumber; ++m) {
            for (int n = 0; n < numVC; ++n) {

                pthread_testcancel(); // set a cancelation point.
                if (Minisat::toInt(solver->modelValue(Ttable[m][n])) == 0){
                    if (nodeInVC(m) == 1) resultCNF.push_back(m);
                }
            }
        }
        // change the int to string to avoid error output.
        //string s = "CNF-SAT-VC: ";
        for (int i = 0; i < resultCNF.size(); ++i) {
            if (resultCNF.size() - 1 != i) sCNF = sCNF + to_string(resultCNF[i]) + ",";
            else sCNF = sCNF + to_string(resultCNF[i]);
        }
        // print the string.
        //printf("%s\n",sCNF.c_str());
        resultCNFSize = resultCNF.size();
        resultCNF.clear();
        return 1;
    }
    else{
        //cerr << "No vertex cover when numVC is " << numVC << endl;
        return  0; // means that the numVC should be added to 1.
    }
}

void * CNFSATVC(void * args){
    CNFDONE = 0;
    for (int numVC = 1; numVC < VNumber; ++numVC) {
        if (getVertexCover(numVC) == 1){
            CNFDONE = 1;
            break;
        }
    }
    // get the CPU time
    /*
    int sCNF;
    sCNF = pthread_getcpuclockid(pthread_self(), &cidCNF);
    if (sCNF != 0)
        handle_error_en(sCNF, "pthread_getcpuclockid");
     */
    // put the CPU time into a vector
    //timespec tCNF = pclock("CNF thread CPU time: ",cidCNF);
    //timeCNF.push_back(tCNF.tv_nsec);
}



// The following is APPROX-VC-1.
// get the maxcount value and set both values in this vertex to 99999.
int getMaxCount(){
    int infCount = 0; // used to count the number of 99999
    int maxCount = 0, index = 0, nCount = 0;
    // get the index of maxcount value.
    for (int i = 0; i < edgeVector.size(); ++i) {
        if (edgeVector[i] == 99999){
            infCount = infCount + 1;
            continue;
        }
        for (int j = 0; j < edgeVector.size(); ++j) {
            if (edgeVector[i] == edgeVector[j]) nCount = nCount + 1;
        }
        //cout << "nCount is " << nCount << endl;
        if (nCount > maxCount){
            maxCount = nCount;
            index = i;
        }
        nCount = 0;
    }

    // assign 99999 to all the vertex (both two values) including countmax value.
    int inf = edgeVector[index];
    for (int k = 0; k < edgeVector.size(); ++k) {
        if (edgeVector[k] == inf){
            edgeVector[k] = 99999;
            if (k % 2 == 0) edgeVector[k + 1] = 99999;
            else edgeVector[k - 1] = 99999;
        }
    }
    // if all value equals to 99999, return 99999 to function APPROXVC1.
    if (infCount == edgeVector.size()){
        return 99999;
    }

    // push the maxcount to vertex cover vector.
    CVresult1.push_back(inf);

    // return the index for iarray and jarray.
    // maybe useless for APPROXVC1.
    if (index % 2 == 0){
        index = index / 2;
    } else index = (index - 1) / 2;
    return index;
}

void * APPROXVC1(void * args){
    for (int i = 0; i < EleLength; ++i) {
        edgeVector.push_back(edgeList[i]);
    }

    while (1){
        if (getMaxCount() == 99999) break;
    }

    // sort the vertex cover result.
    sort(CVresult1.begin(), CVresult1.end(), less<int>());
    // cout << "APPROX-VC-1: ";
    // change the int to string to avoid error output.
    //string s = "APPROX-VC-1: ";
    for (int j = 0; j < CVresult1.size(); ++j) {
        if (j != CVresult1.size()-1) sAVC1 = sAVC1 + to_string(CVresult1[j]) + ",";
        else sAVC1 = sAVC1 + to_string(CVresult1[j]);
    }
    // print the string.
    //printf("%s\n", sAVC1.c_str());
    AVC1DONE = 1;
    // clear all the vector for next input.
    CVresult1Size = CVresult1.size();
    CVresult1.clear();
    edgeVector.clear();
    //pthread_exit(NULL);
    // get the CPU time
    /*
    int s1;
    s1 = pthread_getcpuclockid(pthread_self(), &cid1);
    if (s1 != 0)
        handle_error_en(s1, "pthread_getcpuclockid");
     */
    // put the CPU time into a vector
    //timespec tAVC1 = pclock("APPROX-VC-1 thread CPU time: ",cid1);
    //timeAVC1.push_back(tAVC1.tv_nsec);
}


// The following is APPROX-VC-2.
// function to get the input value index number of input verctor.
int getIndex(int value, vector<int> vector){
    for (int i = 0; i < vector.size(); ++i) {
        if (value == vector[i])
            indexvector.push_back(i);
    }
    return 0;
}

void * APPROXVC2(void * args){
    // transfer the values in array to vector.
    for (int n = 0; n < ENumber; ++n){
        ivector.push_back(iarray[n]);
        jvector.push_back(jarray[n]);
    }
    int ivalue, jvalue;

    for (; ; ) {
        // define the smallest iterator to get the min value.
        vector<int>::iterator ismallest = min_element(begin(ivector), end(ivector));
        vector<int>::iterator jsmallest = min_element(begin(jvector), end(jvector));
        int smallest = min(*ismallest, *jsmallest);
        int indexVC;
        //cout << "ismallest is " << *ismallest << endl;
        //cout << "jsmallest is " << *jsmallest << endl;

        // Problem is here: when *ismallest == *jsmallest

        // get the index of smallest value.
        if (smallest == *ismallest) {
            indexVC = distance(begin(ivector), ismallest);
        } else indexVC = distance(begin(jvector), jsmallest);

        if (smallest < 99999) {
            ivalue = iarray[indexVC];
            //cout << "ivalue is " << ivalue << endl;
            jvalue = jarray[indexVC];
            //cout << "jvalue is " << jvalue << endl;
            // get all the index of same i and j value.
            getIndex(ivalue, ivector);
            getIndex(jvalue, jvector);
            getIndex(ivalue, jvector);
            getIndex(jvalue, ivector);
            // use the indexvector to set the indicated value to a huge number.
            for (int i = 0; i < indexvector.size(); ++i) {
                ivector[indexvector[i]] = 99999;
                jvector[indexvector[i]] = 99999;
            }
            // store the vertex value into the cover vertex result vector.
            CVresult2.push_back(ivalue);
            CVresult2.push_back(jvalue);
            // clear the indexvector.
            indexvector.clear();
            /*
            for (int j = 0; j < ivector.size(); ++j) {
                cout << ivector[j] << ",";
            }
            cout << endl;
            for (int k = 0; k < jvector.size(); ++k) {
                cout << jvector[k] << ",";
            }
            cout << endl;
             */
        } else break;
    }
    /* problem:
     * when input
     * V 5
     * E {<0,4>,<1,4>,<0,3>,<2,0>,<2,1>,<1,0>,<3,1>}
     * the result returns APPROX-VC-2: 0,1,2,4
     * However,  0, 1 is a better result.
     *
     */
    // sort the vertex cover result.
    sort(CVresult2.begin(), CVresult2.end(), less<int>());
    //cout << "APPROX-VC-2: ";
    // change the int to string to avoid error output.
    //string s = "APPROX-VC-2: ";
    for (int j = 0; j < CVresult2.size(); ++j) {
        if (j != CVresult2.size()-1) sAVC2 = sAVC2 + to_string(CVresult2[j]) + ",";
        else sAVC2 = sAVC2 + to_string(CVresult2[j]);
        //std::cout.flush();
    }
    //printf("%s\n", sAVC2.c_str());
    AVC2DONE = 1;
    ivector.clear();
    jvector.clear();
    CVresult2Size = CVresult2.size();
    CVresult2.clear();
    //pthread_exit(NULL);
    // get the CPU time
    /*
    int s2;
    s2 = pthread_getcpuclockid(pthread_self(), &cid2);
    if (s2 != 0)
        handle_error_en(s2, "pthread_getcpuclockid");
     */
    // put the CPU time into a vector
    //timespec tAVC2 = pclock("APPROX-VC-2 thread CPU time: ",cid2);
    //timeAVC2.push_back(tAVC2.tv_nsec);
}

void timeout(pthread_t pthread){
    if (CNFDONE == 0){
        //printf("CNF-SAT-VC: timeout\n");
        int cancel = pthread_cancel(pthread);
        if (cancel != 0)
            handle_error_en(cancel, "pthread_cancel is failed");
    }
    /*
    else{
        printf("ratio AVC1 = %f\n", CVresult1Size/resultCNFSize);
        approxResult1.push_back(CVresult1Size/resultCNFSize);
        printf("ratio AVC2 = %f\n", CVresult2Size/resultCNFSize);
        approxResult2.push_back(CVresult2Size/resultCNFSize);
    }
     */
}

long getSD(std::vector<long> data)
{
    long sum = 0;
    long average;
    long SD = 0;
    for(int i = 0; i < data.size(); ++i)
    {
        sum += data[i];
    }
    average = sum / data.size();
    for(int j = 0; j < data.size(); ++j)
         SD += pow(data[j] - average, 2);
    return sqrt(SD / data.size());
}

float getSDFloat(std::vector<float> data)
{
    float sum = 0;
    float average;
    float SD = 0;
    for(int i = 0; i < data.size(); ++i)
    {
        sum += data[i];
    }
    average = sum / data.size();
    for(int j = 0; j < data.size(); ++j)
        SD += pow(data[j] - average, 2);
    return sqrt(SD / data.size());
}


int writeFile(){
    long averageAVC1 = accumulate( timeAVC1.begin(), timeAVC1.end(), 0.0) / timeAVC1.size();
    long sdAVC1 = getSD(timeAVC1);
    long averageAVC2 = accumulate( timeAVC2.begin(), timeAVC2.end(), 0.0) / timeAVC2.size();
    long sdAVC2 = getSD(timeAVC2);
    float averageRatio1 = accumulate( approxResult1.begin(), approxResult1.end(), 0.0) / approxResult1.size();
    float sdRatio1 = getSDFloat(approxResult1);
    float averageRatio2 = accumulate( approxResult2.begin(), approxResult2.end(), 0.0) / approxResult2.size();
    float sdRatio2 = getSDFloat(approxResult2);

    // create and open the .txt file.
    ofstream OutFile("output_prj.txt");

    // detect if CNF finished in the given time.
    if (CNFDONE == 1){
        long averageCNF = accumulate( timeCNF.begin(), timeCNF.end(), 0.0) / timeCNF.size();
        long sdCNF = getSD(timeCNF);
        // write done the CNF result
        OutFile << "CNF CPU time write Start" << endl;
        for (int i = 0; i < timeCNF.size(); ++i) {
            OutFile << timeCNF[i] << endl;
        }
        OutFile << "CNF Average = " << averageCNF << endl;
        OutFile << "CNF SD = " << sdCNF << endl;
        OutFile << "CNF Finished\n" << endl;

        printf("CNF Average time = %ld\nCNF SD = %ld\n", averageCNF, sdCNF);
    }

    // write done the APPROX-VC-1 result.
    OutFile << "AVC1 CPU time write Start" << endl;
    for (int i = 0; i < timeAVC1.size(); ++i) {
        OutFile << timeAVC1[i] << endl;
    }
    OutFile << "AVC1 Average = " << averageAVC1 << endl;
    OutFile << "AVC1 SD = " << sdAVC1 << endl;
    OutFile << "AVC1 Finished\n" << endl;
    printf("AVC1 Average time = %ld\nAVC1 SD = %ld\n", averageAVC1, sdAVC1);
    printf("Ratio 1 Average = %f\nRatio1 SD = %f\n", averageRatio1, sdRatio1);

    // write done the APPROX-VC-2 result.
    OutFile << "AVC2 CPU time write Start" << endl;
    for (int i = 0; i < timeAVC2.size(); ++i) {
        OutFile << timeAVC2[i] << endl;
    }
    OutFile << "AVC2 Average = " << averageAVC2 << endl;
    OutFile << "AVC2 SD = " << sdAVC2 << endl;
    OutFile << "AVC2 Finished\n" << endl;
    printf("AVC2 Average time = %ld\nAVC2 SD = %ld\n", averageAVC2, sdAVC2);
    printf("Ratio 2 Average = %f\nRatio2 SD = %f\n", averageRatio2, sdRatio2);

    // close the .txt file.
    OutFile.close();

    // clear the vector for next generation.
    timeCNF.clear();
    timeAVC1.clear();
    timeAVC2.clear();
    approxResult1.clear();
    approxResult2.clear();
}

void * IO(void * args){
    while (1){
        string gather = getString();
        int judge;
        char firstChar = gather[0];
        if (firstChar == 'V') {
            getVNumber(gather);
            continue;
        }
        if (firstChar == 'E'){
            getEdgeArr(gather);
            if (eInVertex() == 0){
                judge = 1;
                continue;
            }
            judge = 0;
            twoArray();
            Trigger = 1;
        }
        while (1){
            sleep(2);
            if (AVC1DONE == 1 && AVC2DONE == 1 && CNFDONE == 1){
                printf("%s\n", sCNF.c_str());
                printf("%s\n", sAVC1.c_str());
                printf("%s\n", sAVC2.c_str());
                sCNF = "CNF-SAT-VC: ";
                sAVC1 = "APPROX-VC-1: ";
                sAVC2 = "APPROX-VC-2: ";
                AVC1DONE = 0;
                AVC2DONE = 0;
                //CNFDONE = 0;
                break;
            }
            if (AVC1DONE == 1 && AVC2DONE == 1 && CNFDONE == 0){
                printf("%s\n", sAVC1.c_str());
                printf("%s\n", sAVC2.c_str());
                printf("CNF-SAT-VC: timeout\n");
                sCNF = "CNF-SAT-VC: ";
                sAVC1 = "APPROX-VC-1: ";
                sAVC2 = "APPROX-VC-2: ";
                AVC1DONE = 0;
                AVC2DONE = 0;
                CNFDONE = 0;
                break;
            }
        }
        break;
    }
}


int main() {
    for (;;) {
        // start the multi-thread task.
        int threadResultIO, threadResultCNF, threadResult1, threadResult2;

        pthread_t threadIO;
        threadResultIO = pthread_create(&threadIO, NULL, IO, NULL);
        if (threadResultIO != 0) {
            cout << "The threadIO is failed. error_code = " << threadResultIO << endl;
        }

        while (1){
            if (Trigger == 1){
                pthread_t threadCNF;
                threadResultCNF = pthread_create(&threadCNF, NULL, CNFSATVC, NULL);
                if (threadResultCNF != 0) {
                    cout << "The threadCNF is failed. error_code = " << threadResultCNF << endl;
                }
                pthread_t thread1;
                threadResult1 = pthread_create(&thread1, NULL, APPROXVC1, NULL);
                if (threadResult1 != 0) {
                    cout << "The thread1 is failed. error_code = " << threadResult1 << endl;
                }
                pthread_t thread2;
                threadResult2 = pthread_create(&thread2, NULL, APPROXVC2, NULL);
                if (threadResult2 != 0) {
                    cout << "The thread2 is failed. error_code = " << threadResult2 << endl;
                }
                // if CNF does not finish in a given time, the cancel signal will be sent.
                sleep(2); // you can change the time of timeout here.
                timeout(threadCNF);
                pthread_join(threadCNF, NULL); // I do not know if we can delete this without any thread problems.
                pthread_join(thread1, NULL);
                pthread_join(thread2, NULL);
                Trigger = 0;
                break;
            }
        }
        pthread_join(threadIO, NULL);

        Loopcount = Loopcount + 1;
        if (Loopcount == 100) {
            writeFile(); // output the average time and SD.
            cout << "Output.txt finished." << endl;
        }

        /*clockid_t cid;
        s = pthread_getcpuclockid(pthread_self(), &cid);
        if (s != 0)
            handle_error_en(s, "pthread_getcpuclockid");
        pclock("Main thread CPU time:   ", cid);
         */
        //pclock("Main thread CPU time: ", CLOCK_THREAD_CPUTIME_ID);
        //pclock("Process total CPU time: ", CLOCK_PROCESS_CPUTIME_ID);
        //Trigger = 0;
        //pthread_exit(NULL);
        //return 0;
    }
    return 0;
}
