#include <iomanip>
#include <algorithm>
#include <iostream>
#include <string>
#include <omp.h>

#include <unistd.h>
#include <ios>
#include <fstream>
#ifdef __cplusplus
extern "C" {
#endif 
#include "IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int maxEdgeNum(int *col, int *row,  int nov)
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
		double startTime, endTime, vm, rss;;
		int maxColor = 0;
		bool *ExistedColor = new bool[maxEdgeCnt + 1]();

		startTime = omp_get_wtime();
		for (int i = 0; i < nov; i++)
		{
			int c = MinColor(i, row, col, nov, colorList, ExistedColor, maxEdgeCnt);
			colorList[i] = c;
			if (c > maxColor) maxColor = c;
		}
		endTime = omp_get_wtime();
		// clean up
		delete[] ExistedColor;
		output.colorCnt = maxColor + 1;
		output.execTime = endTime - startTime;
		output.prepTime = output.mergeConflictCnt = 0;
		process_mem_usage(vm, rss);
		output.vm= vm;
		output.rss=rss;
		return output;
	}

	OutputData parallelMethod( int *row, int *col, int nov, short colorList[], int maxEdgeCnt)
	{
		OutputData output;
		double TimeB, TimeE, vm, rss;
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
		TimeB = omp_get_wtime();
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
		TimeE = omp_get_wtime();

		// clean up
		delete[] DetectedVertex;
		delete[] VerticesConflicted;
#pragma omp parallel
		{
			delete[] ExistedColor;
		}
		output.execTime = TimeE - TimeB;
		output.mergeConflictCnt = mergeConflictCnt;
		output.colorCnt = ColorsNum(nov, colorList);
		output.prepTime = 0;
		process_mem_usage(vm, rss);
		output.vm= vm;
		output.rss=rss;
		return output;
	}


int main(int argc, char *argv[])
{
	using namespace std;
	 int *row_ptr;
	int *col_ind;
	double *ewghts;
	int *vwghts;
	int nov;
	// Graph reading
	if (argc < 2)
	{
		cout << " The input format is not correct \n";
		cout << "Press Enter to continue...";
		cin.get();
		return 1;
	}
	if (read_graph(argv[1], &row_ptr, &col_ind, &ewghts, &vwghts, &nov, 0) == -1)
	{
		cout << " graph reading error...\n";
		return 1;
	}

	// finding the maximum number of edges on a vertex
	int cntMaxEdge = maxEdgeNum( col_ind,row_ptr, nov);

	// Performance analysis
	OutputData SequentialTest, ParallelTest[5];

	short *colors = new short[nov];
	fill_n(colors, nov, -1);
	cout << "Starting... \n";
	cout << "__________________________________________________________\n";
	
	// Sequential
	SequentialTest = sequentialMethod(row_ptr, col_ind, nov, colors, cntMaxEdge);

	// Parallel
	int Numthreads[] = {1,2,4};
	using std::cout;
   using std::endl;

  
   
	for (int i = 0; i < 3; i++)
	{
		omp_set_num_threads(Numthreads[i]);
		ParallelTest[i] = parallelMethod(row_ptr, col_ind, nov, colors, cntMaxEdge);
#ifdef DEBUG
		cout << "Correctness checking...";
		s = !conflicts( nov,col_ind,  colors, out, isDetected) ? "correct\n" : "Not correct!\n";
		cout << s;
		fill_n(isDetected, outSize, false);
#endif // DEBUG
		fill_n(colors, nov, -1); // reinitialize
		
	}

	// Print results
	printf(" Sequential  - number of colors %d- done in %10.10f s  with rss %f KB and VM %f KB \n",
			 SequentialTest.colorCnt, SequentialTest.execTime, SequentialTest.rss, SequentialTest.vm);
		cout << "__________________________________________________________\n";
	for (int i = 0; i < 3; i++)
		printf(" Parallel  with  %d threads - number of colors %d- done in %10.10f s  with rss %f KB and VM %f KB\n",
			 Numthreads[i], ParallelTest[i].colorCnt, ParallelTest[i].execTime, ParallelTest[i].rss, ParallelTest[i].vm);
			 	cout << "__________________________________________________________\n";
	cout << "\n";
	return 0;
}
