#include <iostream>
#include <regex>
using namespace std;

//Adjacency Matrix
#define MaxSize 200
#define INF 10000

struct Graph {
    int numVertices;
    int array[MaxSize][MaxSize] = { INF };
};

//floyd, refer https://www.cnblogs.com/skywang12345/p/3711526.html
void Floyd(Graph graph, int src, int dest) {
    int path[MaxSize][MaxSize];
    int shortPath[MaxSize][MaxSize];

    //initial
    for (int l = 0; l < graph.numVertices; l++) {
        for (int i = 0; i < graph.numVertices; i++) {
            path[l][i] = l;
        }
    }
    
    for (int i = 0; i < graph.numVertices; i++) {
           for (int j = 0; j < graph.numVertices; j++) {
               shortPath[i][j] = graph.array[i][j];
               path[i][j] = j;
           }
       }

    //floyd standard algorithm
    for (int k = 0; k < graph.numVertices; k++) {
        for (int i = 0; i < graph.numVertices; i++) {
            for (int j = 0; j < graph.numVertices; j++) {
                if (shortPath[i][j] > (shortPath[i][k] + shortPath[k][j])) {
                    shortPath[i][j] = shortPath[i][k] + shortPath[k][j];
                    path[i][j] = path[i][k];
                }
            }
        }
    }

    //print
    int k = path[src][dest];
    if(k != dest){
        cout << src;
        while (k != dest) {
            cout << "-" << k;
            k = path[k][dest];
    }
    cout << "-" << dest << endl;
    }
    else{
        if (graph.array[src][dest] == INF){
            cout << "Error: the path doesn't exsit" << endl;
        }
        else{
            if(k == src){
                cout << "Error: start and end are the same" << endl;
            }
            else{
                cout << src << "-" << dest << endl;
            }
        }
    }
}

void handleInput() {
    Graph graph;
    int count = 0;
    int pairs[MaxSize];
    int u, v;

    //handle input
    while (true) {
        char command;
        cin >> command;

        if (command == 'V') {  //a set of vertices
            int num = 0;
            cin >> num;
            graph.numVertices = num;
            cout << command << " " << num << endl;
        }

        if (command == 'E') {  //a set of edges
            string s; //refer http://www.cplusplus.com/reference/regex/regex_search/
            cin >> s;
            smatch m;
            regex e("\\d+");
            cout << command << " " << s << endl;

            string::const_iterator iterSrc = s.begin();
            string::const_iterator iterDest = s.end();
            string temp;
            while (regex_search(iterSrc, iterDest, m, e)) {
                temp = m[0];
                pairs[count++] = atoi(temp.c_str());
                iterSrc = m[0].second;
            }
        }

        if (command == 's') {
            cin >> u >> v;
            break;
        }
    }

    for (int i = 0; i < graph.numVertices; i++) {
        for (int j = 0; j < graph.numVertices; j++) {
            if (i == j) {
                graph.array[i][j] = 0;
            }
            else {
                graph.array[i][j] = INF;
            }
        }
    }

    for (int i = 0; i < count; i = i + 2) {
        int src = pairs[i];
        int dest = pairs[i + 1];
        graph.array[src][dest] = 1;
        graph.array[dest][src] = 1;
    }

    if (u >= graph.numVertices || v >= graph.numVertices) {
        cout << "Error: the vertex doesn't exsit" << endl;
    }
    else {
        Floyd(graph, u, v);  //shortest path
    }
}

int main(){
    while(true){
        handleInput();
    }
    return 0;
}
