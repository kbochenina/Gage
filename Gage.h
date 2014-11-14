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
    // returns true if job requires rescheduling (it cannot finish execution before start of window)
    bool FindSchedule(job&, Schedule&);
    // return data transfer time for current job
    double FindDataTransferTimeEnd(int, int, int, Schedule&);
public:
    Gage(DataInfo &d, int param);
    double GetWFSchedule(Schedule &out);
    ~Gage(void);
};

