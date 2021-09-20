The project includes a make file. For compiling codes we should run it like below.<br /><br />
```$ make```<br />

After executing this command the main file will generate. This file is used for coloring the input graph by Distance-1 algorithm in parallel and sequential.<br />
The input file could be .gra or .graph formats. Also, the number of threads in the third argument in input.<br />
For example for coloring new.graph file with 4 threads we should do like below.<br /><br />
```$ ./main ./new.graph 4```<br /><br />
In the output, we could see the number of colors used for coloring, execution time in millisecond, usage of physical and virtual memory.<br />
