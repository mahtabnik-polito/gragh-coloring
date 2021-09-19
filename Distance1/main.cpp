#include <iomanip>
#include <algorithm>
#include <iostream>
#include <string>
#include <omp.h>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif 
#include "mmio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __cplusplus
}
#endif 
//Struct for reporting performance analysis results
typedef	struct OutputData
{
	int colorCnt, mergeConflictCnt;
	double prepTime, execTime,vm,rss;
}OutputData;
//Number of colors
inline int ColorsNum(int nov, short colorList[])
{
	int Max = -1;

	for (int i = 0; i < nov; i++)
		if (colorList[i] > Max)
			Max = colorList[i];

	return Max + 1;
}
int conflicts( int *col, int *row, int nov, short colorList[], int out[], bool Detected[])
{
	 int index = 0;
	 int i = 0;
	do{
		int color = colorList[i];
		int beginCol = row[i], endCol = row[i + 1];
		for (int j = beginCol; j < endCol; j++)
		{
			if (colorList[col[j]] == color) 
			{
				int conflictIndex = i < col[j] ? i : col[j];
				if (!Detected[conflictIndex])
				{
					Detected[conflictIndex] = true;
					out[index++] = conflictIndex;
				}
			}
		}
		i++;
	}while(i < nov);
	i = 0;
	do{
	Detected[out[index]] = false;
	i++;
	}while( i < index);
	return index;
}
int MinColor(int vertex,  int *row, int *col, int nov, short colorList[], bool ExistedColor[], int maxEdgeCnt)
	{
		int beginCol, endCol;
		beginCol = row[vertex];
		endCol = row[vertex + 1];
		for (int i = beginCol; i < endCol; i++)
		{
			int c = colorList[col[i]];
			if (c >= 0)
				ExistedColor[c] = true;
		}
		int i = 0;
		do{
			if (!ExistedColor[i])
			{
				for (int j = beginCol; j < endCol; j++)
				{
					int c = colorList[col[j]];
					if (c >= 0)
						ExistedColor[c] = false;
				}
				return i;
			}
			 i++;
		}while(i < maxEdgeCnt + 1);
		return -1;
	}


// finding the maximum number of edges on a vertex
int maxEdgeNum( int *row,  int nov)
{
	int Max = -1;
	for (int i = 0; i < nov; ++i)
	{
		int num = row[i + 1] - row[i];
		if (num > Max)
			Max = num;
	}
	return Max;
}
void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

	OutputData sequentialMethod( int *row, int *col, int nov, short colorList[], int maxEdgeCnt)
	{
		OutputData output;
		double vm, rss;
		int maxColor = 0;
		bool *ExistedColor = new bool[maxEdgeCnt + 1]();
		for (int i = 0; i < nov; i++)
		{
			int c = MinColor(i, row, col, nov, colorList, ExistedColor, maxEdgeCnt);
			colorList[i] = c;
			if (c > maxColor) maxColor = c;
		}
		// clean up
		delete[] ExistedColor;
		output.colorCnt = maxColor + 1;
		output.prepTime = output.mergeConflictCnt = 0;
		process_mem_usage(vm, rss);
		output.vm= vm;
		output.rss=rss;
		return output;
	}

	OutputData parallelMethod( int *row, int *col, int nov, short colorList[], int maxEdgeCnt)
	{
		OutputData output;
		double vm, rss;
		int mergeConflictCnt = -1;
		int confArrSize = nov / 2 + 1;
		int *VerticesConflicted = new int[confArrSize]();
		bool *DetectedVertex = new bool[nov]();
		static bool *ExistedColor;
#pragma omp threadprivate(ExistedColor)

#pragma omp parallel
		{
			ExistedColor = new bool[maxEdgeCnt + 1]();
		}

		// start coloring
#pragma omp parallel for
		for (int i = 0; i < nov; i++)
		{
			int c = MinColor(i, row, col, nov, colorList, ExistedColor, maxEdgeCnt);
			colorList[i] = c;
		}

		int confCnt = 0;
		// fix conflict
		do
		{
			confCnt = conflicts( col,row, nov, colorList, VerticesConflicted, DetectedVertex);
#pragma omp for
			for (int i = 0; i < confCnt; i++)
			{
				int c = MinColor(VerticesConflicted[i], row, col, nov, colorList, ExistedColor, maxEdgeCnt);
				colorList[VerticesConflicted[i]] = c;
			}
			++mergeConflictCnt;
		} while (confCnt > 0);

		// clean up
		delete[] DetectedVertex;
		delete[] VerticesConflicted;
#pragma omp parallel
		{
			delete[] ExistedColor;
		}
		//output.mergeConflictCnt = mergeConflictCnt;
		output.colorCnt = ColorsNum(nov, colorList);
		output.prepTime = 0;
		process_mem_usage(vm, rss);
		output.vm= vm;
		output.rss=rss;
		return output;
	}
	//
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
	char *temp, *graphLine =  (char *) malloc(sizeof(char)*100000+1);
	while(fgets(graphLine, 100000, fp) != NULL) {
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
	if(ecount != numEdges) {
		printf("number of edges do not match %ld %ld \n", (long int)ecount, (long int)(2 * numEdges));
		
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
	
using namespace std::chrono;
  

	int *row;
	int *col;
	double *ewghts;
	int *vwghts;
	int nov;
	// Graph reading
	if (argc < 3)
	{
		cout << " The input format is not correct \n";
		cout << "Press Enter to continue...";
		return 1;
	}
	if (read_graph(argv[1], &row, &col, &ewghts, &vwghts, &nov) == -1)
	{
		cout << " graph reading error...\n";
		return 1;
	}

	// finding the maximum number of edges on a vertex
	int cntMaxEdge = maxEdgeNum(row, nov);
	OutputData SequentialTest, ParallelTest;

	short *colors = new short[nov];
	fill_n(colors, nov, -1);
	cout << "Starting... \n";
	cout << "__________________________________________________________\n";
	auto start = high_resolution_clock::now();
	// Sequential
	SequentialTest = sequentialMethod(row, col, nov, colors, cntMaxEdge);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start);
  
	// To get the value of duration use the count()
	// member function on the duration object
	SequentialTest.execTime = duration.count();
	// Parallel
	int Numthread  = atoi(argv[2]);
	using std::cout;
    using std::endl;
	
		// Use auto keyword to avoid typing long
// type definitions to get the timepoint
// at this instant use function now()
 start = high_resolution_clock::now();
//The omp_set_num_threads routine affects the number of threads to be used for subsequent parallel regions that do not 
//specify a num_threads clause, by setting the value of the first element of the nthreads-var ICV of the current task.
		omp_set_num_threads(Numthread);
		ParallelTest = parallelMethod(row, col, nov, colors, cntMaxEdge);
#ifdef DEBUG
		cout << "Correctness checking...";
		s = !conflicts( nov,col,  colors, out, isDetected) ? "correct\n" : "Not correct!\n";
		cout << s;
		//Assigns the given value to the first count elements in the range beginning at first if count > 0. Does nothing otherwise.
		//void fill_n( OutputIt first, Size count, const T& value );
		fill_n(isDetected, outSize, false);
#endif // DEBUG
		fill_n(colors, nov, -1); // reinitialize
		// After function call
 stop = high_resolution_clock::now();
 duration = duration_cast<milliseconds>(stop - start);
  
// To get the value of duration use the count()
// member function on the duration object
ParallelTest.execTime = duration.count();
	

	// Print results
	printf(" Sequential  - number of colors %d- done in %10.10f ms  with rss %f KB and VM %f KB \n",
			 SequentialTest.colorCnt, SequentialTest.execTime, SequentialTest.rss, SequentialTest.vm);
		cout << "__________________________________________________________\n";

		printf(" Parallel  with  %d threads - number of colors %d- done in %10.10f ms  with rss %f KB and VM %f KB\n",
			 Numthread, ParallelTest.colorCnt, ParallelTest.execTime, ParallelTest.rss, ParallelTest.vm);
			 	cout << "__________________________________________________________\n";
	cout << "\n";
	return 0;
}
