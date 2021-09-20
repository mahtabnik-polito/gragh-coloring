# Graph Coloring Project (Q1)
In this project we chose 6 algorithms to implement on CPU in multi-threaded and greedy modes. 
The algorithms are: Sequential Greedy, Luby ( one approach for the Maximal Independent Set ) , Distance one , Smallest degree last , Jones- Plassmann and Largest degree first
We implemented and tested the Distance one algorithm on the Ubuntu Machine and the other algorithms were implemented and tested on both MAC OS and Windows 10.
Instruction to run the Luby, Smallest degree last, Jones Plassmann and Largest degree first on Windows on Clion:
1. Clone the repository
2. Each class are in their folders, when you want to run the project, you need to add the path of the class in the execution part of the CMakeLists like the image below:

![CMake Lists](https://github.com/mahtabnik-polito/gragh-coloring/blob/main/images/cmake.JPG?raw=true "CMake Lists")

4. each program takes a path of the graph as an input, in the Clion we define the input arguments and every time we select the input graph

![defile the input argument](https://github.com/mahtabnik-polito/gragh-coloring/blob/main/images/files.JPG?raw=true "defile the input argument")

5.In order to check the Virtual Memory and Physical Memory usage, we used the following lines of code in Windows 10:

    #include <windows.h>
    #include <stdio.h>
    #include <psapi.h>

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    cout<< "Total virtual Memory: " <<totalVirtualMem<<"B"<<endl;

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    cout<< "Used virtual Memory: " <<virtualMemUsedByMe <<"B"<<endl;
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    cout<< "Used Physical Memory By current Process: " <<physMemUsedByMe <<"B"<<endl;
    
If the classes had any errors related to the Mutex library, please import the library using:

    #include <mutex>
    
The multi threading part is implemented using ThreadPool and Mutex in every algorithms except Distance one. the critical areas and parallelization parts were derived form the following paper:
https://www.researchgate.net/publication/2296563_A_Comparison_of_Parallel_Graph_Coloring_Algorithms

in order to test the algorithm with different number of threads, we set the value NTHREAD in the main method of the programs equal to 1,2,4,8 and analyzed the related result.
    

