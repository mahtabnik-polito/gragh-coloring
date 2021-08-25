#include <string.h>
#include <stdlib.h>

#ifndef IO_H
#define IO_H

int read_graph(char* gfile,  int **xadj, int **adj, double **ewghts, int **vwghts, int* nov, int loop);

#endif
