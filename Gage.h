#pragma once
#include "schedulingmethod.h"

typedef pair<int, pair<int,int>> job;
class Gage :
    public SchedulingMethod
{
    bool isWithWindows;
    // simulation time
    unsigned currentTime;
    // scheduling time, (wfNum, localPackageNum) - in order of decreasing current time
    vector <job> queue;
    // vector of indexes of resources sorted by their performances in ascending order
    vector <int> sortedResources;
    // add jobs without parents to the queue
    void AddInitJobsToQueue();
    // build sortedResources vector
    void CreateSortedResources();
    // find schedule or push job to the queue with updated time
    void FindSchedule(job&, Schedule&);
    // return data transfer time for current job
    double FindDataTransferTimeEnd(int, int, int, Schedule&);
	// add ready successors to queue
	void AddReadySuccessors(int, int, Schedule&, double);
	// get next scheduling time as start of next busy interval
	double GetNextStartTime(int, int, double, double, bool);
	// find next free interval (if there is no free interval, returning value is data.GetT())
	double FindNextFreeStart(int, int, double);
public:
    Gage(DataInfo &d, int param);
    double GetWFSchedule(Schedule &out);
    ~Gage(void);
};

