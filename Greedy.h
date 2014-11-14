#pragma once
#include "SchedulingMethod.h"
class Greedy :
	public SchedulingMethod
{
    vector <int> finished;
    vector <int> unscheduled;
 public:
	Greedy(DataInfo &d,int u, int w);
	double GetWFSchedule(Schedule &out);
	double GetFullSchedule(Schedule& out);
	double GetOneWFSchedule(Schedule& out);
    void FindSchedule(Schedule& out, double &eff, int pNum, bool forOneWf);
	~Greedy(void);
};

