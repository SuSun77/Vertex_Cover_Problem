#include <iostream>
#include <string.h>
#include <memory>
#include <algorithm>
#include <vector>
#include <set>
#include <regex>
#include "minisat/mtl/Vec.h"
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include <sstream>

using namespace Minisat;
using namespace std;

const int MaxSize = 200;
const int INF = 10000;

struct Graph{
    int numVertices;
    int edge[MaxSize][MaxSize];
};

Graph graph;

//intinal
void initGraph(Graph *graph, int n){
    graph->numVertices = n;
    
    for(int i = 0; i < MaxSize; ++i){
        for(int j = 0; j < MaxSize; ++j){
            if(i == j){
                graph->edge[i][j] = 0;
            }
            else{
                graph->edge[i][j] = INF;
            }
        }
    }
}

int handleEdge(Graph *graph){
    char E;
    string s;
    cin >> E >> s;
    
    int x, y;
    if(s == "{}"){
        cout << "";
        return 1;
    }
    else{
        for(int i = 0; s[i] != '\0'; ++i){
            if(s[i] == '<'){
                for(int j = 1; s[i + j] != ','; ++j){
                    if(j == 1){
                        x = int(s[i + j] - 48);
                    }
                    else{
                        x = (10 * x) + int(s[i + j] - 48);
                    }
                }
            }
            else if(s[i] == ','){
                if(s[i - 1] >= '0' && s[i - 1] <= '9'){
                    for(int j = 1; s[i + j] != '>'; ++j){
                        if(j == 1){
                            y = int(s[i + j] - 48);
                        }
                        else{
                            y = (10 * y) + int(s[i + j] - 48);
                        }
                    }
                }
            }
            else if((s[i] == '>') && (x != y)){
                graph->edge[x][y] = 1;
            }
        }
    }
    return 0;
}

int vertexCover(Graph graph){
    int k = 1;
    
    vector<vector<Minisat::Lit>> vars;
    Minisat::vec<Minisat::Lit> lits;
    vars.resize(graph.numVertices);
    
    while(true){
        unique_ptr<Minisat::Solver> solver;
        solver = unique_ptr<Minisat::Solver> (new Minisat::Solver());
        
        for(int i = 0; i < graph.numVertices; i++){
            vars[i].resize(k);
        }
        for(int i = 0; i < graph.numVertices; i++){
            for(int j = 0; j < k; j++){
                vars[i][j] = Minisat::mkLit(solver->newVar());
            }
        }
        
        //1
        for(int i = 0; i < k; i++){
            for(int j = 0; j < graph.numVertices; j++){
                lits.push(vars[j][i]);
            }
            solver->addClause(lits);
            lits.clear();
        }
        
        //2
        for(int i = 0; i < graph.numVertices; i++){
            for(int j = 0; j < k - 1; j++){
                for(int m = j + 1; m < k; m++){
                    solver->addClause(~vars[i][j], ~vars[i][m]);
                }
            }
        }
        
        //3
        for(int i = 0; i < k; i++){
            for(int j = 0; j < graph.numVertices - 1; j++){
                for(int m = j + 1; m < graph.numVertices; m++){
                    solver->addClause(~vars[j][i], ~vars[m][i]);
                }
            }
        }
        
        //4
        for(int i = 0; i < graph.numVertices; i++){
            for(int j = 0; j < graph.numVertices; j++){
                if(graph.edge[i][j] == 1){
                    for(int m = 0; m < k; m++){
                        lits.push(vars[i][m]);
                    }
                    for(int m = 0; m < k; m++){
                        lits.push(vars[j][m]);
                    }
                    solver->addClause(lits);
                    lits.clear();
                }
            }
        }
        
        if(solver->solve() == 0){
            solver.reset(new Minisat::Solver());
            k++;
            continue;
        }
        else{
            int miniCV[k];
            int p = 0;
            for(int i = 0; i < graph.numVertices; i++){
                for(int j = 0; j < k; j++){
                    if(Minisat::toInt(solver->modelValue(vars[i][j])) == 0){
                        miniCV[p] = i;
                        p++;
                    }
                }
            }
            for(p = 0; p < k; p++){
                cout << miniCV[p] << " ";
            }
            cout << endl;
            solver.reset(new Minisat::Solver());
            return 0;
        }
    }
}

int main(){
    while(true){
        char command;
        cin >> command;
        int flag = 1;
        int i;
        
        if(cin.eof()){
            break;}
        else{
            if(command == 'V'){
                int num = 0;
                cin >> num;
                initGraph(&graph, num);
            }
        }
        
        if(flag == 1){
            i = handleEdge(&graph);
            if(i != 1){
                vertexCover(graph);
            }
            flag = 0;
        }
    }
    return 0;
}
