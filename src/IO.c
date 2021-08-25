#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mmio.h"
#include "IO.h"
typedef struct {
  int i;
  int j;
  double w;
} Triple;
int cmp(const void *a, const void *b){
	const int *ia = (const int *)a;
	const int *ib = (const int *)b;
	return *ia  - *ib;
}
int tricmp(const void *t1, const void *t2){
  Triple *tr1 = (Triple *)t1;
  Triple *tr2 = (Triple *)t2;
  if(tr1->i == tr2->i) {
    return (int)(tr1->j - tr2->j);
  }
  return (int)(tr1->i - tr2->i);
}
int reading(FILE* fp,  int **pxadj, int **padj, double **pewghts, int **pvwghts,int* pnov, int loop, int Ischaco) {
 if(!Ischaco){
 //-M rows, -N columns, nz-nonzeros
  Triple *T;
  int i, j, M, N, lower, upper, wi;
  double w;
   int k, nz, ecnt, cnt;
  MM_typecode matcode;
  if (mm_read_banner(fp, &matcode) != 0) {
    printf("Could not process Matrix Market banner.\n");
    return -1;
  }
  if (mm_read_mtx_crd_size(fp, &M, &N, &nz) != 0) {
    printf("ret code is wrong\n");
    return -1;
  }
  if(M != N) {
    printf("It's not a graph...\n");
    return -1;
  }
  lower = 1 ;
  upper = N;
  T = (Triple *)malloc(2 * nz * sizeof(Triple));
  cnt = 0;
  if(mm_is_pattern(matcode)) {
    for (ecnt = 0; ecnt < nz; ecnt++) {
      fscanf(fp, "%d %d\n", &i, &j);

      if(i < lower || j < lower || i > upper || j > upper) {
    	  printf("read coordinate %ld %ld -- lower and upper is %ld and %ld\n", (long int)i, (long int)j, (long int)lower, (long int)upper);
    	  return -1;
      }
      if(loop || i != j) {
		  T[cnt].i = i;
		  T[cnt].j = j;
		  T[cnt].w = 1;
		  cnt++;
		  if(mm_is_symmetric(matcode) && i != j) {
			  T[cnt].i = T[cnt-1].j;  
			  T[cnt].j = T[cnt-1].i;
			  T[cnt].w = 1;
			  cnt++;
		  }
      } 
    }
  } else {
    for (ecnt = 0; ecnt < nz; ecnt++) {
      if(mm_is_real(matcode)) { 
	fscanf(fp, "%d %d %lf\n", &i, &j, &w);
      } else if(mm_is_integer(matcode)) {
	fscanf(fp, "%d %d %d\n", &i, &j, &wi);
      }
      w = (double)wi;
      if(i < lower || j < lower || i > upper || j > upper) {
    	  printf("read coordinate %ld %ld -- lower and upper is %ld and %ld\n", (long int)i, (long int)j, (long int)lower, (long int)upper);
    	  return -1;
      }
      if(w != 0 && (loop || i != j)) {
		  T[cnt].i = i;
		  T[cnt].j = j;
		  T[cnt].w = fabs(w);
		  cnt++;
		  if(mm_is_symmetric(matcode) && i != j) { /* insert the symmetric edge */
			  T[cnt].i = T[cnt-1].j;
			  T[cnt].j = T[cnt-1].i;
			  T[cnt].w = T[cnt-1].w;
			  cnt++;
		  }
      }
    }    
  }  
  qsort(T, cnt, sizeof(Triple), tricmp);
  (*pxadj) = ( int*)malloc(sizeof( int) * (N+1));
  memset((*pxadj), 0, sizeof( int) * (N+1));
  k = 0;
  (*pxadj)[T[0].i ]++;
  for(ecnt = 1; ecnt < cnt; ecnt++) {
	  i = T[ecnt].i;
	  if(i != T[ecnt-1].i || T[ecnt].j != T[ecnt-1].j) {
		  (*pxadj)[i ]++;
		  k = i; 
	  } else { 
		  T[k].w += T[i].w; 
	  }
  }
  for(i = 2; i <= N; i++) (*pxadj)[i] += (*pxadj)[i-1];
  (*padj) = (int*)malloc(sizeof(int) * (*pxadj)[N]);
  (*pewghts) = (double*)malloc(sizeof(double) * (*pxadj)[N]);
  (*padj)[0] = T[0].j - 1; (*pewghts) [0] = T[0].w; k = 1;
  for(ecnt = 1; ecnt < cnt; ecnt++) {
	 i = T[ecnt].i;
	 if(i != T[ecnt-1].i || T[ecnt].j != T[ecnt-1].j) { 
		 (*padj)[k] = T[ecnt].j - 1 ;
		 (*pewghts)[k++] = T[ecnt].w;
	 }
  }
  (*pvwghts)  = (int*)malloc(sizeof(int) * N);
  for(i = 0; i < N; i++) (*pvwghts) [i] = 1;
  *pnov = N; 
  free(T);
  return 1;
 }else{
int state = 0, fmt = 0, ncon = 1, i;
	int numVertices = -1, vcount = 0, jv;
	 int numEdges = -1, ecount = 0;
	char *temp, *graphLine =  (char *) malloc(sizeof(char)*10000000+1);
	while(fgets(graphLine, 10000000, fp) != NULL) {
		for(i = 0; i < (int) strlen(graphLine); i++) {
			char c = graphLine[i];
			if(c != ' ' && c != '\t' && c != '\n') {
				break;
			}
		}
		if(graphLine[0] == '%') {
			continue;
		} else if(state == 0) {
			temp = strtok(graphLine, " \t\n");
			numVertices = atoi(temp);
			temp = strtok(NULL, " \t\n");
			numEdges = atoi(temp);
			temp = strtok(NULL, " \t\n");
			if(temp != NULL) {
				fmt = atoi(temp);
				temp = strtok(NULL, " \t\n");
				if(temp != NULL) {ncon = atoi(temp);}
			}
			*pnov = numVertices;
			(*pxadj) = ( int*)malloc(sizeof( int) * (numVertices+1));
			(*pxadj)[0] = 0;
			(*pvwghts)  = (int*)malloc(sizeof(int) * numVertices);
			(*padj) = (int*)malloc(sizeof(int) * 2 * numEdges);
			(*pewghts) = (double*)malloc(sizeof(double)  * 2 * numEdges);
            state = 1;
		} else { 
			if(vcount == numVertices) {
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
			} else {
				(*pvwghts)[vcount] = 1;
			}
			while(temp != NULL) {
				if(ecount == 2 * numEdges) {
					printf("Error: file contains more than %ld edges\n", (long int)numEdges);
					return -1;
				}
				(*padj)[ecount] = atoi(temp) - 1; /* ids start from 1 in the graph */
				if((*padj)[ecount] == vcount && !loop) {
					continue;
				}
				temp = strtok(NULL, " \t\n");
				if(fmt % 10 == 1) {
					(*pewghts)[ecount] = atoi(temp);
					temp = strtok(NULL, " \t\n");
				} else {
					(*pewghts)[ecount] = 1;
				}
				if((*pewghts)[ecount] < 0) {
					printf("negative edge weight %lf between %ld-%ld.\n", (*pewghts)[ecount], (long int)vcount, (long int)((*padj)[ecount]));
					return -1;
				}
	            ecount++;
			}
			vcount++;
			(*pxadj)[vcount] = ecount;
		}
	}
	if(vcount != numVertices) {
		printf("number of vertices do not match %ld %ld\n", (long int)numVertices, (long int)vcount);
		return -1;
	}
	if(ecount != 2 * numEdges) {
		printf("number of edges do not match %ld %ld: realloc memory appropriately\n", (long int)ecount, (long int)(2 * numEdges));
		(*padj) = (int*)realloc((*padj), sizeof(int) * ecount);
		(*pewghts) = (double*)realloc((*pewghts) , sizeof(double) * ecount);
	}
	for(jv = 0; jv < vcount; jv++) {
		qsort((*padj) + (*pxadj)[jv], (*pxadj)[jv+1] - (*pxadj)[jv], sizeof(int), cmp);
	}

	return 1;
 }
}
int read_graph(char* filename,  int **xadj, int **adj,double **ewghts, int **vwghts,int* nov, int loop) {
  	FILE  *fp;
    fp = fopen(filename, "r");
    if(fp == NULL) {
      printf("%s: file does not exist\n", filename);
      return -1;
    } 
	//get file extension
	 const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) {
	return -1;}
	 if(strcmp(dot,".graph") == 0){
    	  if(reading(fp, xadj, adj, ewghts, vwghts, nov, loop,1) == -1) {
    		  printf("error in reading the file\n");
    		  fclose(fp);
    	      return -1;
    	  }
      }
    else if(strcmp(dot,".mtx")== 0) {
    	  if(reading(fp, xadj, adj, ewghts, vwghts, nov, loop, 0) == -1) {
    		  printf("error in reading the file\n");
    		  fclose(fp);
    		  return -1;
    	  }
      }
      fclose(fp);
  return 1;
}
