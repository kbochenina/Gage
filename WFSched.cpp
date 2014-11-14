// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataInfo.h"
#include "Scheduler.h"
#include <cstdlib>
#include <stdlib.h>
#include "direct.h"
#include "windows.h"


using namespace std;

enum SchedulingTypes { ONLY_BELLMAN = 1, ONLY_GREEDY = 2, CLUST = 3, GAGE = 4 };
enum SchedulingSchemes { STAGED = 1, EFF_ORDERED = 2, SIMPLE = 3, RESERVED_ORDERED = 4, CLUSTERED = 5, GAGEWITHOUTWINDOWS = 6, GAGEWITHWINDOWS = 7 };

int _tmain(int argc, wchar_t**argv)
{
	// fileSettings is a file with program settings
	// it is a first command line argument
	// if program is started without arguments, filename is "settings.txt"
	double minLInit = 20000;
   int periodsCount = 1, experCount = 1;
   _chdir("D:\\ITMO\\Degree\\Programs\\Clustered\\Clustered");
	wstring fileSettings;
	if (argc == 1 ) {
		fileSettings=L"settings.txt";
	}
	else {
		minLInit = _wtof(argv[1]);
      periodsCount = _wtoi(argv[2]);
      experCount = _wtoi(argv[3]);
		//cout << minLInit << endl;
	}
	fileSettings=L"settings.txt";
	string s(fileSettings.begin(),fileSettings.end());
	cout << "File settings name: " << s << endl;
	srand(time(NULL));
	
	double koeff = 0.125;
	for (int i = 0; i < periodsCount; i++){
		// set data
		double minLength = minLInit + koeff*i * minLInit;
		cout << "Minlength = " << minLength << endl;
     //system("pause");
      for (int j = 0; j < experCount; j++){
          _chdir("D:\\ITMO\\Degree\\Programs\\Clustered\\Clustered\\Output");
          string metricsFileName = "fullmetrics";
          metricsFileName.append(to_string(i+1));
          metricsFileName.append("-");
          metricsFileName.append(to_string(j+1));
          metricsFileName.append(".txt");
          ofstream m(metricsFileName, ios::trunc);
		    m.close();
          _chdir("D:\\ITMO\\Degree\\Programs\\Clustered\\Clustered");
		    DataInfo data(s,minLength);
		    Scheduler sched(data);
		    ofstream f("fullmetrics.txt", ios::trunc);
		    f.close();
		    sched.SetSchedulingStrategy(ONLY_GREEDY);	
		    sched.GetSchedule(SIMPLE);
		    sched.GetMetrics("simple_metrics.txt", "SimpleSched",metricsFileName);
		    sched.TestSchedule();
		    cout << "***************************************************" << endl;
		    sched.GetSchedule(RESERVED_ORDERED);
		    sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime",metricsFileName);
		    sched.TestSchedule();
		    cout << "***************************************************" << endl;
		    /*sched.GetSchedule(EFF_ORDERED);
		    sched.GetMetrics("eff_metrics.txt", "StagedEfficiency");
		    sched.TestSchedule();
		    cout << "***************************************************" << endl;*/
		    sched.SetSchedulingStrategy(CLUST);
		    sched.GetSchedule(CLUSTERED);
		    sched.GetMetrics("clustered.txt", "Clustered", metricsFileName);
		    sched.TestSchedule();
		    cout << "***************************************************" << endl;
          sched.SetSchedulingStrategy(GAGE);
		    sched.GetSchedule(GAGEWITHOUTWINDOWS);
		    sched.GetMetrics("gagewithoutwindows.txt", "GageWithoutWindows", metricsFileName);
		    sched.TestSchedule();
		    cout << "***************************************************" << endl;
     
		    _chdir("D:\\ITMO\\Degree\\Programs\\Clustered\\Clustered");
      }
      
	}
	/*sched.GetSchedule(STAGED);
	sched.GetMetrics("staged_metrics.txt");
	sched.TestSchedule();*/
	//system("pause");
	return 0;
}

