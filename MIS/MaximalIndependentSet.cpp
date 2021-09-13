//
// Created by S273284 on 8/26/2021.
//
#include <iostream>
#include <list>
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>

using namespace std;

class Graph {
    int V;
    list<int> *graph;

public:
    Graph();

    Graph(int V) {
        this->V = V;
        this->graph = new list<int>[this->V + 1];
    }

    ~Graph() {
        delete[] graph;
    }

    void addEdge(int x, int y) {
        this->graph[x].push_back(y);
    }

    void print() {
        for (int i = 1; i <= V; i++) {
            cout << i << " --> ";

            for (int nbr : graph[i])
                cout << nbr << ", ";

            cout << endl;
        }
    }


};

std::vector<int> MaximalIndependentSet(vector<int> A[], int V) {
    //initialization of I
    //vector<int> A[] = graph;
    vector<int> M;
    while (!A->empty()) {
        //firstly choose a random set of V with the probability of 1/d(v)
        vector<int> randomValues;
        //random values
        for (int v = 1; v < V; v++) {
            int count = 0;
            for (auto nbr : A[v]) {
                count += 1;
            }
            randomValues[v] = 1 / count;

        }

        vector<int> M_prim;
        int i = 0;
        //if any of them are neighbors, remove the one with the lowest probability
        //TODO this part can be parallelized
        for (int v = 0; v < A->size(); v++) {
            for (int u = 0; u < A[v].size(); u++) {
                if (randomValues[v] < randomValues[u]) {
                    M_prim.push_back(v);
                }

            }
        }
        //add M_prim to M
        for (int j = 0; j < M_prim.size(); j++) {
            M.push_back(M_prim[j]);
        }
        //remove from v the set s and all the neighbors in s
        for (int j = 0; j < A->size(); j++) {
            for (int k = 0; k < M.size(); k++) {
                vector<int> neighbors = A[M[k]];
                for (i = 0; i < neighbors.size(); i++) {
                    std::remove(A->begin(), A->end(), neighbors[i]);
                }
            }
        }


    }
    return M;


}

chrono::steady_clock::time_point getCurrentClock() {
    return chrono::steady_clock::now();
}

void printElapsedTime(chrono::steady_clock::time_point start, chrono::steady_clock::time_point end) {
    printf("%02ld:%02ld:%02lld.%09lld\n",
           chrono::duration_cast<chrono::hours>(end - start).count(),
           chrono::duration_cast<chrono::minutes>(end - start).count(),
           chrono::duration_cast<chrono::seconds>(end - start).count(),
           chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}

int main() {
    chrono::steady_clock::time_point start_time, start_time_coloring, end_time;

    ifstream file("/Users/giannicito/Documents/SDP/Course Material/project/gragh-coloring/data/rgg_n_2_15_s0.graph");

    if (!file.is_open())
        cout << "failed to open file\n";

    string line;
    bool firstRow = true;
    int V = 1, E;
    Graph *G;
    int cv = 1; // current vertex

    start_time = getCurrentClock();

    while (getline(file, line) && cv <= V) {
        // cout << line << endl;
        stringstream sin(line);

        if (firstRow) {
            firstRow = false;
            sin >> V >> E;
            G = new Graph(V);
        } else {
            int e;
            while (sin >> e)
                G->addEdge(cv, e);

            cv++;
        }
    }

    end_time = getCurrentClock();

    cout << endl << "Reading time: ";
    printElapsedTime(start_time, end_time);
    cout << endl;

    G->print();

    start_time_coloring = getCurrentClock();
//    int *colors = G->MaximalIndependentSet();
    end_time = getCurrentClock();

    cout << endl << "Coloring time: ";
    printElapsedTime(start_time_coloring, end_time);
    cout << endl;


    // print the result
//    for ( int u = 1; u <= V; u++ )
//        cout << "Vertex " << u << " --->  Color "
//        << colors[ u ] << endl;

    cout << endl << "Total elapsed time: ";
    printElapsedTime(start_time, end_time);
    cout << endl;

    return 0;
}//
