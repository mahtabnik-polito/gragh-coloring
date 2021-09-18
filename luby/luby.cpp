//
// Created by Gianni Cito - s261725@studenti.polito.it
//

#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>
#include <set>
#include <mutex>

using namespace std;
std::mutex mu;

class Graph {
    int V;
    int vs;
    vector<int> *graph;

    int M_counter;

public:
    Graph();

    Graph(int V) {
        this->V = V;
        this->graph = new vector<int>[this->V + 1];
        this->M_counter = 0;
        this->vs = this->V;
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

    vector<int> FindMaximalIndependentSet() {
        //initialization of I
        //vector<int> A[] = graph;
        int _V = vs;
//        int _V = 0;
        int M_counter = 0;
        vector<int> *A = new vector<int>[V + 1];
        vector<int> M;

        for (int i = 1; i <= V; i++)
            for (int j = 0; j < graph[i].size(); j++)
                A[i].push_back(graph[i][j]);

        while (_V > 0) {
            /*cout << endl << "print" << endl;
            for ( int i = 1; i <= V; i++ )
            {
                cout << i << " --> ";

                for ( int nbr : A[ i ] )
                    cout << nbr << ", ";

                cout << endl;
            }*/

            //firstly choose a random set of V with the probability of 1/d(v)
            vector<double> randomValues(V + 1);
            //random values
            std::vector<std::thread> threadPool;
            for (int i = 0; i < 8; i++) {
                int duration = V / 8;
                int start = i * duration;
                int end = (i + 1) * duration;
                threadPool.emplace_back(
                        thread(
                                [start, end, A, &randomValues]() {

                                    for (int v = start; v <= end; v++) {
                                        if (A[v].size() == 0)
                                            randomValues[v] = 0;
                                        else
                                            randomValues[v] = (double) 1 / (2 * A[v].size());
                                    }


                                }
                        )
                );
            }
            for (auto &t : threadPool)
                t.join();

            threadPool.clear();

            set<int> M_prim;

            std::vector<std::thread> threadPool2;
            //if any of them are neighbors, remove the one with the lowest probability
            for (int i = 0; i < 8; i++) {
                int duration = V / 8;
                int start = i * duration;
                int end = (i + 1) * duration;

                threadPool2.emplace_back(
                        thread(
                                [start, end, A, randomValues, &M_prim]() {

                                    for (int v = start; v <= end; v++) {

                                        for (int u = 0; u < A[v].size(); u++) {

                                            if (randomValues[v] > randomValues[A[v][u]]) {
                                                std::lock_guard<std::mutex> lock(mu);
                                                M_prim.insert(v);
                                            }


                                        }

                                    }


                                }
                        )
                );

            }

            for (auto &t : threadPool2)
                t.join();

            threadPool2.clear();
            // check if M_prim is empty, in that case add the first vertex to be colored
            if (M_prim.size() == 0) {
                bool isFound=false;
                for (int i = 0; i <= V; i++) {
                    if (A[i].size()>0) {
                        M_prim.insert(i);
                        isFound=true;
                        break;
                    }
                }


            }
            int size = M_prim.size();
            //add M_prim to M
            for (int value: M_prim)
                M.push_back(value);

            _V -= (int) M_prim.size();
            vs -= (int) M_prim.size();
            if(M_prim.size()==0){
                _V--;
                vs-=1;
            }
            //remove from v the set s and all the neighbors in s

            //cout << "MIS: {";
            set<int, greater<int> >::iterator i;
            for (i = M_prim.begin(); i != M_prim.end(); i++) {
                int index = *i;


                // removing neighbors of vertex v
                for (int j = 0; j < A[index].size(); j++) {
                    _V--;
                    if (A[A[index][j]].size() > 0) {
                        A[A[index][j]].erase(A[A[index][j]].begin(), A[A[index][j]].end());


                    }
                }

                // removing the vertex v
                A[index].erase(A[index].begin(), A[index].end());
                graph[index].erase(graph[index].begin(), graph[index].end());
            }
            M_prim.clear();

            cout << _V << endl;

        }

        return M;
    }

    int *colorMIS() {
        // this->graph[ x ].remove( v );
        int *colors = new int[V + 1]{-1};

        int color = 0;

        while (vs > 0) {
            vector<int> mis = this->FindMaximalIndependentSet();
            int size= mis.size();
            if (size > 0) {
                for (int i = 0; i < mis.size(); i++){
                    colors[mis[i]] = color;
                }

               color+=1;
            }
        }
        for(int i=1;i<V+1 ; i++){
            if(colors[i]==-1){
                colors[i]=color-1;
            }
        }



        cout << endl << "Used color: " << color << endl;

        return colors;
    }

};

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

    ifstream file("/Users/S273284/CLionProjects/untitled/rgg_n_2_15_s0.graph");

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

//    G->print();

    start_time_coloring = getCurrentClock();
    int *colors = G->colorMIS();
    end_time = getCurrentClock();

    cout << endl << "Coloring time: ";
    printElapsedTime(start_time_coloring, end_time);
    cout << endl;


    // print the result
    for (int u = 1; u <= V; u++)
        cout << "Vertex " << u << " --->  Color "
             << colors[u] << endl;

    cout << endl << "Total elapsed time: ";
    printElapsedTime(start_time, end_time);
    cout << endl;

    return 0;
}
