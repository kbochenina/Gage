// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include "UserException.h"
#include <boost/tuple/tuple.hpp>

using namespace boost;
using namespace std;

// (globalPackageNum, tBegin, vector<globalCoreNums>, execTime)
typedef tuples::tuple<int,int,vector<int>,double> PackageSchedule;
typedef vector <PackageSchedule> Schedule;

enum Criteries { RESERVED_TIME = 1, MAX_EFF = 2 };



// TODO: reference additional headers your program requires here
