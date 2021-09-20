The project includes a make file. For compiling codes we should run it like below.
**$ make**

After executing this command the main file will generate. This file is used for coloring the input graph by Distance-1 algorithm in parallel and sequential.
The input file could be .gra or .graph formats. Also, the number of threads in the third argument in input.
For example for coloring new.graph file with 4 threads we should do like below.
$ ./main ./new.graph 4
In the output, we could see the number of colors used for coloring, execution time in millisecond, usage of physical and virtual memory.
