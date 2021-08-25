#include <iomanip>
#include <algorithm>
#include <iostream>
#include <string>
#include <omp.h>

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
	double prepTime, execTime;
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


	OutputData sequentialMethod( int *row, int *col, int nov, short colorList[], int maxEdgeCnt)
	{
		OutputData output;
		double startTime, endTime;
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
		return output;
	}

	OutputData parallelMethod( int *row, int *col, int nov, short colorList[], int maxEdgeCnt)
	{
		OutputData output;
		double TimeB, TimeE;
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
	printf(" Sequential  - number of colors %d- done in %10.10f s \n",
			 SequentialTest.colorCnt, SequentialTest.execTime);
		cout << "__________________________________________________________\n";
	for (int i = 0; i < 3; i++)
		printf(" Parallel  with  %d threads - number of colors %d- done in %10.10f s \n",
			 Numthreads[i], ParallelTest[i].colorCnt, ParallelTest[i].execTime);
			 	cout << "__________________________________________________________\n";
	cout << "\n";
	return 0;
}
