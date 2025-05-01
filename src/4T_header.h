#define ROW 6
#define COL 6
#define PIECES 8
#define MAX_PLIES 200
#define MAX_MOVES 32

#define RED 1
#define BLUE 2
#define UNKNOWN 3

#define USER 0
#define ENEMY 1

#define POS_NUM 1537019       //32*36^3 + 33*36^2 + 34*36 + 35
#define FEATURE_NUM 256       //4*4*4*4
#define TUPLE_NUM 61          //總共61個4-tuple

#define MAX_HAND 200        //max move 200

#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <chrono>
#include <random>
#include <time.h>
#ifdef _WIN32
    #include <direct.h>  // Windows directory handling
#else
    #include <sys/stat.h>  // POSIX directory handling (for Mac/Linux)
#endif
#include <algorithm>
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach/mach_time.h>  // For timing functions
    // Add other macOS-specific headers as needed
#endif

#include "pcg_random.hpp"
#include "4T_DATA.hpp"
#include "4T_GST.hpp"
