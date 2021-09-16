//
// Created by Gianni Cito - s261725@studenti.polito.it
//

#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>

using namespace std;

class Graph
{
    int V;
    int vs;
    vector<int> *graph;

public:
    Graph();

    Graph( int V )
    {
        this->V = V;
        this->graph = new vector<int>[this->V + 1];
        this->vs = this->V;
    }

    ~Graph()
    {
        delete[] graph;
    }

    void addEdge( int x, int y )
    {
        this->graph[ x ].push_back( y );
    }

    void print()
    {
        for ( int i = 1; i <= V; i++ )
        {
            cout << i << " --> ";

            for ( int nbr : graph[ i ] )
                cout << nbr << ", ";

            cout << endl;
        }
    }

    vector<int> LargestDegreeFirst()
    {
        vector<int> colors( V + 1, -1 );

        int color = 0;

        vector<int> randomValues;
        vector<int> weights;

        for ( int i = 0; i <= V; i++ )
        {
            weights.push_back( graph[ i ].size());
            randomValues.push_back(( int ) rand());
        }

        vector<int> *A = graph;

        while ( vs > 0 )
        {
            vector<int> I;

            //random values
            std::vector<std::thread> threadPool;

            for ( int v = 1; v <= V; v++ )
            {
                if ( A[ v ].size() == 0 )
                    continue;

                threadPool.emplace_back(
                        thread(
                                [ v, randomValues, weights, A, &I ]()
                                {
                                    bool add = true;

                                    for ( int u = 0; u < A[ v ].size(); u++ )
                                    {
                                        if ( weights[ v ] < weights[ A[ v ][ u ]] ||
                                             ( weights[ v ] == weights[ A[ v ][ u ]] &&
                                               randomValues[ v ] <= randomValues[ A[ v ][ u ]] ) )
                                        {
                                            add = false;
                                            break;
                                        }
                                    }

                                    if ( add )
                                        I.push_back( v );
                                }
                        )
                );
            }

            for ( auto &t : threadPool )
                t.join();

            // setting the minimum color for all the vertices of the independent set
            std::vector<std::thread> threadPool2;

            for ( int vp_index = 0; vp_index < I.size(); vp_index++ )
            {
                threadPool2.emplace_back(
                        thread(
                                [ vp_index, &colors, A, I, &color ]()
                                {
                                    int vp = I[ vp_index ];
                                    int c = -1;

                                    for ( int u = 0; u < A[ vp ].size(); u++ )
                                    {
                                        if ( colors[ A[ vp ][ u ]] > c )
                                        {
                                            c = colors[ A[ vp ][ u ]];
                                        }
                                    }

                                    colors[ vp ] = c + 1;

                                    if ( c + 1 > color )
                                        color = c + 1;
                                }
                        )
                );
            }

            for ( auto &t2 : threadPool2 )
                t2.join();

            // remove already inserted vertex from the graph
            for ( int vindex = 0; vindex < I.size(); vindex++ )
            {
                A[ I[ vindex ]].erase( A[ I[ vindex ]].begin(), A[ I[ vindex ]].end());
                randomValues[ I[ vindex ]] = 0;
                weights[ I[ vindex ] ] = 0;
            }

            vs -= I.size();
        }

        cout << endl << "Used color: " << color + 1 << endl;

        return colors;
    }

};

chrono::steady_clock::time_point getCurrentClock()
{
    return chrono::steady_clock::now();
}

void printElapsedTime( chrono::steady_clock::time_point start, chrono::steady_clock::time_point end )
{
    printf( "%02ld:%02ld:%02lld.%09lld\n",
            chrono::duration_cast<chrono::hours>( end - start ).count(),
            chrono::duration_cast<chrono::minutes>( end - start ).count(),
            chrono::duration_cast<chrono::seconds>( end - start ).count(),
            chrono::duration_cast<chrono::nanoseconds>( end - start ).count());
}

int main()
{
    srand( time( 0 ));

    chrono::steady_clock::time_point start_time, start_time_coloring, end_time;

    ifstream file( "/Users/giannicito/Documents/SDP/Course Material/project/gragh-coloring/data/test3.graph" );

    if ( !file.is_open())
        cout << "failed to open file\n";

    string line;
    bool firstRow = true;
    int V = 1, E;
    Graph *G;
    int cv = 1; // current vertex

    start_time = getCurrentClock();

    while ( getline( file, line ) && cv <= V )
    {
        // cout << line << endl;
        stringstream sin( line );

        if ( firstRow )
        {
            firstRow = false;
            sin >> V >> E;
            G = new Graph( V );
        } else
        {
            int e;
            while ( sin >> e )
                G->addEdge( cv, e );

            cv++;
        }
    }

    end_time = getCurrentClock();

    cout << endl << "Reading time: ";
    printElapsedTime( start_time, end_time );
    cout << endl;

    G->print();

    start_time_coloring = getCurrentClock();
    vector<int> colors = G->LargestDegreeFirst();
    end_time = getCurrentClock();

    cout << endl << "Coloring time: ";
    printElapsedTime( start_time_coloring, end_time );
    cout << endl;


    // print the result
    for ( int u = 1; u <= V; u++ )
        cout << "Vertex " << u << " --->  Color "
             << colors[ u ] << endl;

    cout << endl << "Total elapsed time: ";
    printElapsedTime( start_time, end_time );
    cout << endl;

    return 0;
}