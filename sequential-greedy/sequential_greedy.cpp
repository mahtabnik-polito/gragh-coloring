/*
 * Implementation of Sequential Greedy Algorithm
 * Created by Gianni Cito - s261725@studenti.polito.it
 */
#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

int V, E;

class Graph
{
    int V;
    vector<int> *graph;

public:
    Graph();

    Graph( int V )
    {
        this->V = V;
        this->graph = new vector<int>[this->V + 1];
    }

    ~Graph()
    {
        delete[] graph;
    }

    void setV( int V )
    {
        delete[] graph;

        this->V = V;
        this->graph = new vector<int>[V + 1];
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
    int *GreedyAlgorithm()
    {
        // this->graph[ x ].remove( v );
        int *colors = new int[V + 1];

        colors[ 1 ] = 0; // assigning the first color to the first vertex

        for ( int u = 2; u <= V; u++ )
            colors[ u ] = -1;

        bool available[V + 1];
        for ( int cr = 0; cr <= V; cr++ )
            available[ cr ] = true;

        int max_color = 0;
        // Assign colors to remaining V-1 vertices
        for ( int u = 2; u <= V; u++ )
        {
            // Process all adjacent vertices and flag their colors
            // as unavailable
            vector<int>::iterator i;
            for ( i = graph[ u ].begin(); i != graph[ u ].end(); ++i )
                if ( colors[ *i ] != -1 )
                    available[ colors[ *i ]] = false;

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

        // check wether there are vertices not colored due to error in graph
        for ( int v = 1; v <= V; v++ )
        {
            if ( colors[ v ] == -1 )
            {
                max_color ++;
                colors[ v ] = max_color;
            }
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
            chrono::duration_cast<chrono::nanoseconds>( end - start ).count());
}

//IO FILES
int cmp( const void *a, const void *b )
{
    const int *ia = ( const int * ) a;
    const int *ib = ( const int * ) b;
    return *ia - *ib;
}

int readingGraph( FILE *fp, Graph *G )
{


    int state = 0, fmt = 0, ncon = 1, i;
    int numVertices = -1, vcount = 0, jv;
    int numEdges = -1, ecount = 0;
    char *temp, *graphLine = ( char * ) malloc( sizeof( char ) * 1000000 + 1 );
    while ( fgets( graphLine, 1000000, fp ) != NULL )
    {
        for ( i = 0; i < ( int ) strlen( graphLine ); i++ )
        {
            char c = graphLine[ i ];
            if ( c != ' ' && c != '\t' && c != '\n' )
            {
                break;
            }
        }
        if ( graphLine[ 0 ] == '%' )
        {
            continue;
        } else if ( state == 0 )
        {
            //Reading the first line
            temp = strtok( graphLine, " \t\n" );
            numVertices = atoi( temp );

            // creating graph
            G->setV( numVertices );

            V = numVertices;
            temp = strtok( NULL, " \t\n" );
            numEdges = atoi( temp );
            E = numEdges;
            temp = strtok( NULL, " \t\n" );
            if ( temp != NULL )
            {
                fmt = atoi( temp );
                temp = strtok( NULL, " \t\n" );
                if ( temp != NULL ) { ncon = atoi( temp ); }
            }
            state = 1;
        } else
        {
            //Following
            if ( vcount == numVertices )
            {
                printf( "Error: file contains more than %ld lines\n", ( long int ) numVertices );
                return -1;
            }
            temp = strtok( graphLine, " \t\n" );
            if ( fmt >= 100 )
            {
                temp = strtok( NULL, " \t\n" );
            }
            if ( fmt % 100 >= 10 )
            {
                for ( i = 1; i < ncon; i++ )
                {
                    temp = strtok( NULL, " \t\n" );
                }
            }

            while ( temp != NULL )
            {
                if ( ecount == 2 * numEdges )
                {
                    printf( "Error: file contains more than %ld edges\n", ( long int ) numEdges );
                    return -1;
                }

                G->addEdge( vcount + 1, atoi( temp ));

                temp = strtok( NULL, " \t\n" );
                if ( fmt % 10 == 1 )
                {
                    temp = strtok( NULL, " \t\n" );
                }

                ecount++;
            }
            vcount++;
        }
    }
    if ( vcount != numVertices )
    {
        printf( "number of vertices do not match %ld %ld\n", ( long int ) numVertices, ( long int ) vcount );
        return -1;
    }
    if ( ecount != 2 * numEdges )
    {
        printf( "number of edges do not match %ld %ld: realloc memory appropriately\n", ( long int ) ecount,
                ( long int ) ( 2 * numEdges ));
    }

    return 1;

}

int readingGra( FILE *fp, Graph *G )
{
    int state = 0, fmt = 0, ncon = 1, i;
    int numVertices = -1, vcount = 0, jv;
    int numEdges = -1, ecount = 0;
    char *temp, *graphLine = ( char * ) malloc( sizeof( char ) * 100000 + 1 );
    //Number of Edges
    while ( fgets( graphLine, 10000000, fp ) != NULL )
    {
        for ( i = 0; i < ( int ) strlen( graphLine ); i++ )
        {
            char c = graphLine[ i ];
            if ( c != ' ' && c != '\t' && c != '\n' )
            {
                break;
            }
        }
        //numEdges , fmt,temp ,padj , pewghts
        if ( graphLine[ 0 ] == '#' )
        {
            continue;
        } else if ( state == 0 )
        {
            //Reading the first line
            temp = strtok( graphLine, " \t\n" );
            numVertices = atoi( temp );

            G->setV( numVertices );

            temp = strtok( NULL, " \t\n" );
            state = 1;
        } else
        {
            temp = strtok( graphLine, " \t\n" );
            while ( temp != NULL )
            {
                temp = strtok( NULL, " \t\n" );
                if (( temp != NULL ) == 1 )
                {
                    if ( atoi( temp ) > 0 )
                        numEdges++;
                }
            }
        }
    }
    if ( numEdges >= 0 )
        numEdges++;
    rewind( fp );
    //
    V = numVertices;
    E = numEdges;
    state = 0;
    while ( fgets( graphLine, 1000000, fp ) != NULL )
    {

        for ( i = 0; i < ( int ) strlen( graphLine ); i++ )
        {
            char c = graphLine[ i ];
            if ( c != ' ' && c != '\t' && c != '\n' )
            {
                break;
            }
        }
        if ( graphLine[ 0 ] == '#' )
        {
            continue;
        } else if ( state == 0 )
        {
            //Reading the first line
            printf( graphLine );
            state = 1;
        } else
        {

            //Following
            if ( vcount == numVertices )
            {
                printf( "Error: file contains more than %ld lines\n", ( long int ) numVertices );
                return -1;
            }
            temp = strtok( graphLine, " \t\n" );
            if ( fmt >= 100 )
            {
                temp = strtok( NULL, " \t\n" );
            }
            if ( fmt % 100 >= 10 )
            {
                for ( i = 1; i < ncon; i++ )
                {
                    temp = strtok( NULL, " \t\n" );
                }
            }

            temp = strtok( NULL, " \t\n" );
            while ( temp != NULL && temp[ 0 ] != '#' )
            {
                if ( ecount == 2 * numEdges )
                {
                    printf( "Error: file contains more than %ld edges\n", ( long int ) numEdges );
                    return -1;
                }

                G->addEdge( vcount + 1, atoi( temp ));

                temp = strtok( NULL, " \t\n" );
                if ( fmt % 10 == 1 )
                {
                    temp = strtok( NULL, " \t\n" );
                }

                ecount++;
            }
            vcount++;
        }
    }
    if ( vcount != numVertices )
    {
        printf( "number of vertices do not match %ld %ld\n", ( long int ) numVertices, ( long int ) vcount );
        return -1;
    }
    if ( ecount != numEdges )
    {
        printf( "number of edges do not match %ld %ld: realloc memory appropriately\n", ( long int ) ecount,
                ( long int ) ( 2 * numEdges ));

    }

    return 1;
}

int read_graph( char *filename, Graph *G )
{
    FILE *fp;
    fp = fopen( filename, "r" );
    if ( fp == NULL )
    {
        printf( "%s: file does not exist\n", filename );
        return -1;
    }
    //get file extension
    const char *dot = strrchr( filename, '.' );
    if ( !dot || dot == filename )
    {
        return -1;
    }
    if ( strcmp( dot, ".graph" ) == 0 )
    {
        if ( readingGraph( fp, G ) == -1 )
        {
            printf( "error in reading the file\n" );
            fclose( fp );
            return -1;
        }
    } else if ( strcmp( dot, ".gra" ) == 0 )
    {
        if ( readingGra( fp, G ) == -1 )
        {
            printf( "error in reading the file\n" );
            fclose( fp );
            return -1;
        }
    }
    fclose( fp );
    return 1;
}

int main( int argc, char *argv[] )
{
    chrono::steady_clock::time_point start_time, start_time_coloring, end_time;ì

    Graph *G = new Graph( 0 );

    start_time = getCurrentClock();

    // Graph reading;
    if ( read_graph( argv[ 1 ], ref( G )) == -1 )
    {
        printf( " graph reading error...\n" );
        return 1;
    }
    printf( "number of V is %d and edges is %d", V, E );

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
