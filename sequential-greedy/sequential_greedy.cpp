//
// Created by Gianni Cito - s261725@studenti.polito.it
//
#include <iostream>
#include <list>
#include <fstream>
#include <chrono>
#include <sstream>

using namespace std;

class Graph
{
    int V;
    list<int> *graph;

public:
    Graph();

    Graph( int V )
    {
        this->V = V;
        this->graph = new list<int>[this->V + 1];
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

    // sequential algorithm ---> Sequential Greedy Algorithm
    int * GreedyAlgorithm()
    {
        // this->graph[ x ].remove( v );
        int *colors = new int[ V + 1 ];

        colors[ 1 ] = 0; // assigning the first color to the first vertex

        for ( int u = 2; u <= V; u++ )
            colors[ u ] = -1;

        bool available[ V + 1 ];
        for ( int cr = 0; cr <= V; cr++ )
            available[ cr ] = true;

        int max_color = 0;
        // Assign colors to remaining V-1 vertices
        for ( int u = 2; u <= V; u++ )
        {
            // Process all adjacent vertices and flag their colors
            // as unavailable
            list<int>::iterator i;
            for ( i = graph[ u ].begin(); i != graph[ u ].end(); ++i )
                if ( colors[ *i ] != -1 )
                    available[ colors[ *i ] ] = false;

            // Find the first available color
            for ( int cr = 0; cr <= V; cr++ )
            {
                if ( available[ cr ] )
                {
                    colors[ u ] = cr; // Assign the found color

                    if ( max_color < cr )
                        max_color = cr;
                    break;
                }
            }

            // Reset the values back to false for the next iteration
            for ( int cr = 0; cr <= V; cr++ )
                available[ cr ] = true;
        }

        cout << endl << "Used color: " << ( max_color + 1 ) << endl;

        return colors;

        // print the result
        for ( int u = 1; u <= V; u++ )
            cout << "Vertex " << u << " --->  Color "
                 << colors[ u ] << endl;
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
            chrono::duration_cast<chrono::nanoseconds>( end - start ).count() );
}

int main()
{
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
        }
        else
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
    int *colors = G->GreedyAlgorithm();
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
