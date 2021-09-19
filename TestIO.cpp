//
// Created by Gianni Cito - s261725@studenti.polito.it
//

#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>
#include <mutex>

using namespace std;

int NTHREAD = 0;
int V, E;
class Graph
{
    int V;
    int vs;
    vector<int>* graph;

public:
    Graph();

    Graph(int V)
    {
        this->V = V;
        this->graph = new vector<int>[this->V + 1];
        this->vs = this->V;
    }

    ~Graph()
    {
        delete[] graph;
    }

    void addEdge(int x, int y)
    {
        this->graph[x].push_back(y);
    }

    void print()
    {
        for (int i = 1; i <= V; i++)
        {
            cout << i << " --> ";

            for (int nbr : graph[i])
                cout << nbr << ", ";

            cout << endl;
        }
    }

    vector<int> SmallestDegreeFirst()
    {
        vector<int> colors(V + 1, -1);

        int color = 0;

        vector<int> randomValues;
        vector<int> weights(V + 1, 0);

        // random value for the 0-index
        randomValues.push_back((int)rand());

        vector<int>* Gp = new vector<int>[V + 1];
        vector<int> sizes(V + 1, 0);

        int _V = V;
        for (int i = 1; i <= V; i++)
        {
            randomValues.push_back((int)rand());
            sizes[i] = graph[i].size();
            for (int j = 0; j < graph[i].size(); j++)
                Gp[i].push_back(graph[i][j]);
        }

        int k = 1;
        int w = 1;

        vector<int> S;
        std::vector<std::thread> threadPool;
        std::vector<std::thread> threadPool2;

        while (_V > 0)
        {
            for (int v = 1; v <= V; v++)
            {
                if (sizes[v] <= 0)
                    continue;

                if (sizes[v] <= k)
                    S.push_back(v);
            }

            if (S.size() > 0)
            {
                int duration2 = S.size() / NTHREAD;
                int NTHREAD2 = NTHREAD;
                if (NTHREAD2 > S.size())
                {
                    duration2 = 1;
                    NTHREAD2 = S.size();
                }

                vector<int> threads_size2(NTHREAD2, duration2);

                // assigning remaining vertex to the corresponding thread
                if (NTHREAD2 * duration2 < S.size())
                {
                    int n = S.size() - (NTHREAD2 * duration2);
                    for (int i = 0; i < n; i++)
                        threads_size2[i]++;
                }

                int end = 0;
                for (int i = 0; i < NTHREAD2; i++)
                {
                    int start = end;
                    end += threads_size2[i];

                    threadPool.emplace_back(
                        thread(
                            [start, end, &Gp, w, S, &weights, &sizes]()
                            {
                                for (int vp_index = start; vp_index < end; vp_index++)
                                {
                                    int v = S[vp_index];
                                    weights[v] = w;
                                    sizes[v] = 0;

                                    for (int u = 0; u < Gp[v].size(); u++)
                                        sizes[Gp[v][u]]--;
                                }
                            }
                        )
                    );
                }

                for (auto& t : threadPool)
                    t.join();

                threadPool.clear();

                _V -= S.size();
                w++;
                S.clear();
            }
            else
            {
                // all weights have already been set
                _V = 0;
            }

            k++;
        }

        vector<int>* A = graph;

        // handling threads according to the number of vertices
        int duration = V / NTHREAD;
        if (NTHREAD > V)
        {
            duration = 1;
            NTHREAD = V;
        }

        vector<int> threads_size(NTHREAD, duration);

        // assigning remaining vertex to the corresponding thread
        if (NTHREAD * duration < V)
        {
            int n = V - (NTHREAD * duration);
            for (int i = 0; i < n; i++)
                threads_size[i]++;
        }

        vector<int> I;

        mutex m;

        while (vs > 0)
        {
            int end = 1;
            for (int i = 0; i < NTHREAD; i++)
            {
                int start = end;
                end += threads_size[i];

                threadPool.emplace_back(
                    thread(
                        [start, end, A, randomValues, weights, &I, &m]()
                        {
                            for (int v = start; v < end; v++)
                            {
                                if (A[v].size() == 0)
                                    continue;

                                bool add = true;

                                for (int u = 0; u < A[v].size(); u++)
                                {
                                    if (weights[v] < weights[A[v][u]] ||
                                        (weights[v] == weights[A[v][u]] &&
                                            randomValues[v] <= randomValues[A[v][u]]))
                                    {
                                        add = false;
                                        break;
                                    }
                                }

                                if (add)
                                {
                                    {
                                        unique_lock<mutex> l{ m };
                                        I.emplace_back(v);
                                    }
                                }
                            }
                        }
                    )
                );
            }

            for (auto& t : threadPool)
                t.join();

            threadPool.clear();

            if (I.empty())
            {
                // terminate the program because all colors are found and there is an error in the graph
                //cout << "vs: " << vs << endl;
                vs = 0;
                break;
            }

            // handling threads according to the number of vertices in the independent set
            int duration2 = I.size() / NTHREAD;
            int NTHREAD2 = NTHREAD;
            if (NTHREAD2 > I.size())
            {
                duration2 = 1;
                NTHREAD2 = I.size();
            }

            vector<int> threads_size2(NTHREAD2, duration2);

            // assigning remaining vertex to the corresponding thread
            if (NTHREAD2 * duration2 < I.size())
            {
                int n = I.size() - (NTHREAD2 * duration2);
                for (int i = 0; i < n; i++)
                    threads_size2[i]++;
            }

            // setting the minimum color for all the vertices of the independent set
            end = 0;
            for (int i = 0; i < NTHREAD2; i++)
            {
                int start = end;
                end += threads_size2[i];

                threadPool2.emplace_back(
                    thread(
                        [start, end, A, I, &colors, &color, &m]()
                        {
                            for (int vp_index = start; vp_index < end; vp_index++)
                            {
                                int vp = I[vp_index];
                                int c = -1;

                                for (int u = 0; u < A[vp].size(); u++)
                                {
                                    if (colors[A[vp][u]] > c)
                                    {
                                        c = colors[A[vp][u]];
                                    }
                                }

                                colors[vp] = c + 1;

                                if (c + 1 > color)
                                {
                                    {
                                        unique_lock<mutex> l{ m };
                                        color = c + 1;
                                    }
                                }
                            }
                        }
                    )
                );
            }

            for (auto& t2 : threadPool2)
                t2.join();

            threadPool2.clear();

            // remove already inserted vertex from the graph
            for (int vindex = 0; vindex < I.size(); vindex++)
            {
                A[I[vindex]].erase(A[I[vindex]].begin(), A[I[vindex]].end());
                randomValues[I[vindex]] = 0;
                weights[I[vindex]] = 0;
            }

            vs -= I.size();
            I.clear();
        }

        cout << endl << "Used color: " << color + 1 << endl;

        return colors;
    }

};

chrono::steady_clock::time_point getCurrentClock()
{
    return chrono::steady_clock::now();
}

void printElapsedTime(chrono::steady_clock::time_point start, chrono::steady_clock::time_point end)
{
    printf("%02ld:%02ld:%02lld.%09lld\n",
        chrono::duration_cast<chrono::hours>(end - start).count(),
        chrono::duration_cast<chrono::minutes>(end - start).count(),
        chrono::duration_cast<chrono::seconds>(end - start).count(),
        chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}
//IO FILES
int cmp(const void* a, const void* b) {
    const int* ia = (const int*)a;
    const int* ib = (const int*)b;
    return *ia - *ib;
}
int readingGraph(FILE* fp, int** pxadj, int** padj, double** pewghts, int** pvwghts, int* pnov) {


    int state = 0, fmt = 0, ncon = 1, i;
    int numVertices = -1, vcount = 0, jv;
    int numEdges = -1, ecount = 0;
    char* temp, * graphLine = (char*)malloc(sizeof(char) * 1000000 + 1);
    while (fgets(graphLine, 1000000, fp) != NULL) {
        for (i = 0; i < (int)strlen(graphLine); i++) {
            char c = graphLine[i];
            if (c != ' ' && c != '\t' && c != '\n') {
                break;
            }
        }
        if (graphLine[0] == '%') {
            continue;
        }
        else if (state == 0) {
            //Reading the first line
            temp = strtok(graphLine, " \t\n");
            numVertices = atoi(temp);
            V = numVertices;
            temp = strtok(NULL, " \t\n");
            numEdges = atoi(temp);
            E = numEdges;
            temp = strtok(NULL, " \t\n");
            if (temp != NULL) {
                fmt = atoi(temp);
                temp = strtok(NULL, " \t\n");
                if (temp != NULL) { ncon = atoi(temp); }
            }
            *pnov = numVertices;
            (*pxadj) = (int*)malloc(sizeof(int) * (numVertices + 1));
            (*pxadj)[0] = 0;
            (*pvwghts) = (int*)malloc(sizeof(int) * numVertices);

            (*padj) = (int*)malloc(sizeof(int) * 2 * numEdges);
            (*pewghts) = (double*)malloc(sizeof(double) * 2 * numEdges);
            state = 1;
        }
        else {
            //Following 
            if (vcount == numVertices) {
                printf("Error: file contains more than %ld lines\n", (long int)numVertices);
                return -1;
            }
            temp = strtok(graphLine, " \t\n");
            if (fmt >= 100) {
                temp = strtok(NULL, " \t\n");
            }
            if (fmt % 100 >= 10) {
                (*pvwghts)[vcount] = atoi(temp);
                for (i = 1; i < ncon; i++) {
                    temp = strtok(NULL, " \t\n");
                }
            }
            else {
                (*pvwghts)[vcount] = 1;
            }
            while (temp != NULL) {
                if (ecount == 2 * numEdges) {
                    printf("Error: file contains more than %ld edges\n", (long int)numEdges);
                    return -1;
                }
                (*padj)[ecount] = atoi(temp) - 1; /* ids start from 1 in the graph */
                temp = strtok(NULL, " \t\n");
                if (fmt % 10 == 1) {
                    (*pewghts)[ecount] = atoi(temp);
                    temp = strtok(NULL, " \t\n");
                }
                else {
                    (*pewghts)[ecount] = 1;
                }
                if ((*pewghts)[ecount] < 0) {
                    printf("negative edge weight %lf between %ld-%ld.\n", (*pewghts)[ecount], (long int)vcount, (long int)((*padj)[ecount]));
                    return -1;
                }
                ecount++;
            }
            vcount++;
            (*pxadj)[vcount] = ecount;
        }
    }
    if (vcount != numVertices) {
        printf("number of vertices do not match %ld %ld\n", (long int)numVertices, (long int)vcount);
        return -1;
    }
    if (ecount != 2 * numEdges) {
        printf("number of edges do not match %ld %ld: realloc memory appropriately\n", (long int)ecount, (long int)(2 * numEdges));
        (*padj) = (int*)realloc((*padj), sizeof(int) * ecount);
        (*pewghts) = (double*)realloc((*pewghts), sizeof(double) * ecount);
    }
    for (jv = 0; jv < vcount; jv++) {
        qsort((*padj) + (*pxadj)[jv], (*pxadj)[jv + 1] - (*pxadj)[jv], sizeof(int), cmp);
    }

    return 1;

}
int readingGra(FILE* fp, int** pxadj, int** padj, double** pewghts, int** pvwghts, int* pnov) {
    int state = 0, fmt = 0, ncon = 1, i;
    int numVertices = -1, vcount = 0, jv;
    int numEdges = -1, ecount = 0;
    char* temp, * graphLine = (char*)malloc(sizeof(char) * 100000 + 1);
    //Number of Edges
    while (fgets(graphLine, 10000000, fp) != NULL) {
        for (i = 0; i < (int)strlen(graphLine); i++) {
            char c = graphLine[i];
            if (c != ' ' && c != '\t' && c != '\n') {
                break;
            }
        }
        //numEdges , fmt,temp ,padj , pewghts
        if (graphLine[0] == '#') {
            continue;
        }
        else if (state == 0) {
            //Reading the first line
            temp = strtok(graphLine, " \t\n");
            numVertices = atoi(temp);
            temp = strtok(NULL, " \t\n");
            state = 1;
        }
        else {
            temp = strtok(graphLine, " \t\n");
            while (temp != NULL) {
                temp = strtok(NULL, " \t\n");
                if ((temp != NULL) == 1) {
                    if (atoi(temp) > 0)
                        numEdges++;
                }
            }
        }
    }
    if (numEdges >= 0)
        numEdges++;
    rewind(fp);
    //
    V = numVertices;
    E = numEdges;
    state = 0;
    while (fgets(graphLine, 1000000, fp) != NULL) {

        for (i = 0; i < (int)strlen(graphLine); i++) {
            char c = graphLine[i];
            if (c != ' ' && c != '\t' && c != '\n') {
                break;
            }
        }
        if (graphLine[0] == '#') {
            continue;
        }
        else if (state == 0) {
            //Reading the first line
            printf(graphLine);
            *pnov = numVertices;
            (*pxadj) = (int*)malloc(sizeof(int) * (numVertices + 1));
            (*pxadj)[0] = 0;
            (*pvwghts) = (int*)malloc(sizeof(int) * numVertices);

            (*padj) = (int*)malloc(sizeof(int) * numEdges);
            (*pewghts) = (double*)malloc(sizeof(double) * numEdges);
            state = 1;
        }
        else {

            //Following 
            if (vcount == numVertices) {
                printf("Error: file contains more than %ld lines\n", (long int)numVertices);
                return -1;
            }
            temp = strtok(graphLine, " \t\n");
            if (fmt >= 100) {
                temp = strtok(NULL, " \t\n");
            }
            if (fmt % 100 >= 10) {
                (*pvwghts)[vcount] = atoi(temp);
                for (i = 1; i < ncon; i++) {
                    temp = strtok(NULL, " \t\n");
                }
            }
            else {
                (*pvwghts)[vcount] = 1;
            }
            temp = strtok(NULL, " \t\n");
            while (temp != NULL && temp[0] != '#') {

                if (ecount == 2 * numEdges) {
                    printf("Error: file contains more than %ld edges\n", (long int)numEdges);
                    return -1;
                }
                (*padj)[ecount] = atoi(temp) - 1; /* ids start from 1 in the graph */
                temp = strtok(NULL, " \t\n");
                if (fmt % 10 == 1) {
                    (*pewghts)[ecount] = atoi(temp);
                    temp = strtok(NULL, " \t\n");
                }
                else {
                    (*pewghts)[ecount] = 1;
                }
                if ((*pewghts)[ecount] < 0) {
                    printf("negative edge weight %lf between %ld-%ld.\n", (*pewghts)[ecount], (long int)vcount, (long int)((*padj)[ecount]));
                    return -1;
                }
                ecount++;
            }
            vcount++;
            (*pxadj)[vcount] = ecount;
        }
    }
    if (vcount != numVertices) {
        printf("number of vertices do not match %ld %ld\n", (long int)numVertices, (long int)vcount);
        return -1;
    }
    if (ecount != 2 * numEdges) {
        printf("number of edges do not match %ld %ld: realloc memory appropriately\n", (long int)ecount, (long int)(2 * numEdges));
        //(*padj) = (int*)realloc((*padj), sizeof(int) * ecount);
        //(*pewghts) = (double*)realloc((*pewghts) , sizeof(double) * ecount);
    }
    for (jv = 0; jv < vcount; jv++) {
        qsort((*padj) + (*pxadj)[jv], (*pxadj)[jv + 1] - (*pxadj)[jv], sizeof(int), cmp);
    }

    return 1;
}
int read_graph(char* filename, int** xadj, int** adj, double** ewghts, int** vwghts, int* nov) {
    FILE* fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s: file does not exist\n", filename);
        return -1;
    }
    //get file extension
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return -1;
    }
    if (strcmp(dot, ".graph") == 0) {
        if (readingGraph(fp, xadj, adj, ewghts, vwghts, nov) == -1) {
            printf("error in reading the file\n");
            fclose(fp);
            return -1;
        }
    }
    else if (strcmp(dot, ".gra") == 0) {
        if (readingGra(fp, xadj, adj, ewghts, vwghts, nov) == -1) {
            printf("error in reading the file\n");
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    return 1;
}

int main(int argc, char* argv[])
{
    srand(time(0));

    NTHREAD = thread::hardware_concurrency();

    chrono::steady_clock::time_point start_time, start_time_coloring, end_time;

    // ifstream file("/Users/rgg_n_2_16_s0.graph");
    int* row_ptr;
    int* col_ind;
    double* ewghts;
    int* vwghts;
    int nov;
    // Graph reading;
    if (read_graph(argv[1], &row_ptr, &col_ind, &ewghts, &vwghts, &nov) == -1)
    {
        printf(" graph reading error...\n");
        return 1;
    }
    printf("number of V is %d and edges is %d", V, E);


    return 0;
}
