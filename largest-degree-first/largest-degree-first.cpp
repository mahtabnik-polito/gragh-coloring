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

int NTHREAD = 0;

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

        // handling threads according to the number of vertices
        int duration = V / NTHREAD;
        if ( NTHREAD > V )
        {
            duration = 1;
            NTHREAD = V;
        }

        vector<int> threads_size( NTHREAD, duration );

        // assigning remaining vertex to the corresponding thread
        if ( NTHREAD * duration < V )
        {
            int n = V - ( NTHREAD * duration );
            for ( int i = 0; i < n; i++ )
                threads_size[ i ]++;
        }

        std::vector<std::thread> threadPool;
        vector<int> I;

        mutex m;

        while ( vs > 0 )
        {
            int end = 1;
            for ( int i = 0; i < NTHREAD; i++ )
            {
                int start = end;
                end += threads_size[ i ];

                threadPool.emplace_back(
                        thread(
                                [ start, end, A, randomValues, weights, &I, &m ]()
                                {
                                    for ( int v = start; v < end; v++ )
                                    {
                                        if ( A[ v ].size() == 0 )
                                            continue;

                                        bool add = true;

                                        for ( int u = 0; u < A[ v ].size(); u++ )
                                        {
                                            if ( weights[ v ] < weights[ A[ v ][ u ]] ||
                                                 ( weights[ v ] == weights[ A[ v ][ u ]] &&
                                                   randomValues[ v ] <= randomValues[ A[ v ][ u ]] ))
                                            {
                                                add = false;
                                                break;
                                            }
                                        }

                                        if ( add )
                                        {
                                            {
                                                unique_lock<mutex> l{ m };
                                                I.emplace_back( v );
                                            }
                                        }
                                    }
                                }
                        )
                );
            }

            for ( auto &t : threadPool )
                t.join();

            threadPool.clear();

            if ( I.empty())
            {
                // terminate the program because all colors are found and there is an error in the graph
                //cout << "vs: " << vs << endl;
                vs = 0;
                break;
            }

            // setting the minimum color for all the vertices of the independent set
            std::vector<std::thread> threadPool2;

            // handling threads according to the number of vertices in the independent set
            int duration2 = I.size() / NTHREAD;
            int NTHREAD2 = NTHREAD;
            if ( NTHREAD2 > I.size())
            {
                duration2 = 1;
                NTHREAD2 = I.size();
            }

            vector<int> threads_size2( NTHREAD2, duration2 );

            // assigning remaining vertex to the corresponding thread
            if ( NTHREAD2 * duration2 < I.size())
            {
                int n = I.size() - ( NTHREAD2 * duration2 );
                for ( int i = 0; i < n; i++ )
                    threads_size2[ i ]++;
            }

            end = 0;
            for ( int i = 0; i < NTHREAD2; i++ )
            {
                int start = end;
                end += threads_size2[ i ];

                threadPool2.emplace_back(
                        thread(
                                [ start, end, A, I, &colors, &color, &m ]()
                                {
                                    for ( int vp_index = start; vp_index < end; vp_index++ )
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

            for ( auto &t2 : threadPool2 )
                t2.join();

            threadPool2.clear();

            // remove already inserted vertex from the graph
            for ( int vindex = 0; vindex < I.size(); vindex++ )
            {
                A[ I[ vindex ]].erase( A[ I[ vindex ]].begin(), A[ I[ vindex ]].end());
                randomValues[ I[ vindex ]] = 0;
                weights[ I[ vindex ]] = 0;
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

    NTHREAD = thread::hardware_concurrency();

    chrono::steady_clock::time_point start_time, start_time_coloring, end_time;

    ifstream file( "/Users/giannicito/Documents/SDP/Course Material/project/gragh-coloring/data/rgg_n_2_15_s0.graph" );

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
