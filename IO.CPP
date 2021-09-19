#include <iomanip>
#include <algorithm>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
}
#endif 


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
int readingGraph(FILE* fp,  int **pxadj, int **padj, double **pewghts, int **pvwghts,int* pnov) {
 

int state = 0, fmt = 0, ncon = 1, i;
	int numVertices = -1, vcount = 0, jv;
	 int numEdges = -1, ecount = 0;
	char *temp, *graphLine =  (char *) malloc(sizeof(char)*1000000+1);
	while(fgets(graphLine, 1000000, fp) != NULL) {
		for(i = 0; i < (int) strlen(graphLine); i++) {
			char c = graphLine[i];
			if(c != ' ' && c != '\t' && c != '\n') {
				break;
			}
		}
		if(graphLine[0] == '%') {
			continue;
		} else if(state == 0) {
			//Reading the first line
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
			//Following 
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
int readingGra(FILE* fp,  int **pxadj, int **padj, double **pewghts, int **pvwghts,int* pnov){
	int state = 0, fmt = 0, ncon = 1, i;
	int numVertices = -1, vcount = 0, jv;
	 int numEdges = -1, ecount = 0;
	char *temp, *graphLine =  (char *) malloc(sizeof(char)*100000+1);
	//Number of Edges
	while(fgets(graphLine, 10000000, fp) != NULL) {
		for(i = 0; i < (int) strlen(graphLine); i++) {
			char c = graphLine[i];
			if(c != ' ' && c != '\t' && c != '\n') {
				break;
			}
		}
		//numEdges , fmt,temp ,padj , pewghts
		if(graphLine[0] == '#') {
			continue;
		} else if(state == 0) {
			//Reading the first line
			temp = strtok(graphLine, " \t\n");
			numVertices = atoi(temp);
			temp = strtok(NULL, " \t\n");
            state = 1;
		} else {
			temp = strtok(graphLine, " \t\n");
			while(temp != NULL) {
				temp = strtok(NULL, " \t\n");
				if ((temp != NULL) == 1){
					if(atoi(temp)>0)
					numEdges++;
				}  
			}
		}
	}
	if(numEdges>= 0)
	numEdges++;
	rewind(fp);
	//
	state = 0;
	while(fgets(graphLine, 1000000, fp) != NULL) {
	
		for(i = 0; i < (int) strlen(graphLine); i++) {
			char c = graphLine[i];
			if(c != ' ' && c != '\t' && c != '\n') {
				break;
			}
		}
		if(graphLine[0] == '#') {
			continue;
		} else if(state == 0) {
			//Reading the first line
				printf(graphLine);
			*pnov = numVertices;
			(*pxadj) = ( int*)malloc(sizeof( int) * (numVertices+1));
			(*pxadj)[0] = 0;
			(*pvwghts)  = (int*)malloc(sizeof(int) * numVertices);

			(*padj) = (int*)malloc(sizeof(int)  * numEdges);
			(*pewghts) = (double*)malloc(sizeof(double)   * numEdges);
            state = 1;
		} else {
			
			//Following 
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
			temp = strtok(NULL, " \t\n");
			while(temp != NULL && temp[0]!='#') {
				
				if(ecount == 2 * numEdges) {
					printf("Error: file contains more than %ld edges\n", (long int)numEdges);
					return -1;
				}
				(*padj)[ecount] = atoi(temp) - 1; /* ids start from 1 in the graph */
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
		//(*padj) = (int*)realloc((*padj), sizeof(int) * ecount);
		//(*pewghts) = (double*)realloc((*pewghts) , sizeof(double) * ecount);
	}
	for(jv = 0; jv < vcount; jv++) {
		qsort((*padj) + (*pxadj)[jv], (*pxadj)[jv+1] - (*pxadj)[jv], sizeof(int), cmp);
	}

	return 1;
}
int read_graph(char* filename,  int **xadj, int **adj,double **ewghts, int **vwghts,int* nov) {
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
    	  if(readingGraph(fp, xadj, adj, ewghts, vwghts, nov) == -1) {
    		  printf("error in reading the file\n");
    		  fclose(fp);
    	      return -1;
    	  }
      }else if(strcmp(dot,".gra") == 0){
    	  if(readingGra(fp, xadj, adj, ewghts, vwghts, nov) == -1) {
    		  printf("error in reading the file\n");
    		  fclose(fp);
    	      return -1;
    	  }
      }
      fclose(fp);
  return 1;
}


int main(int argc, char *argv[])
{
	using namespace std;
	int *row_ptr;
	int *col_ind;
	double *ewghts;
	int *vwghts;
	int nov;
    double vm, rss;
	// Graph reading
	if (argc < 2)
	{
		printf(" The input format is not correct \n");
	
		return 1;
	}
	if (read_graph(argv[1], &row_ptr, &col_ind, &ewghts, &vwghts, &nov) == -1)
	{
		printf(" graph reading error...\n");
		return 1;
	}

	
	return 0;
}
