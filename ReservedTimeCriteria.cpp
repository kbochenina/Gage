#include "stdafx.h"
#include "ReservedTimeCriteria.h"


ReservedTimeCriteria::ReservedTimeCriteria(DataInfo &d) : CriteriaMethod(d)
{
}

double ReservedTimeCriteria::GetCriteria(const Schedule &in){
	// replace in case of different deadlines
	double deadline = data.GetT();
	// max ending time
	double maxEndTime = 0;
	for (auto &sched: in){
		double currentEndTime = sched.get<1>() + sched.get<3>();
		if (maxEndTime < currentEndTime)
			maxEndTime = currentEndTime;
	}
	// if all tasks are unscheduled 
	if (maxEndTime == 0) return 0;
	return (deadline - maxEndTime);
}

// get criteria value taling into account the average time of unscheduled tasks
double ReservedTimeCriteria::GetCriteria (const Schedule &in, const int &wfNum){
	double requiredDeadline = data.GetDeadline(wfNum);
	// max ending time
	double maxEndTime = 0;
	for (auto &sched: in){
		double currentEndTime = sched.get<1>() + sched.get<3>();
		if (maxEndTime < currentEndTime)
			maxEndTime = currentEndTime;
	}
	// if all tasks are unscheduled 
	if (maxEndTime == 0) return 0;
	double deadline = requiredDeadline - maxEndTime;
	// if there are some unscheduled tasks
	int pCount = data.Workflows(wfNum).GetPackageCount();
	if (in.size() != pCount){
		vector <int> scheduled;
		for (const auto &sched : in){
			scheduled.push_back(sched.get_head());
		}
		for (int pNum = 0; pNum < pCount; pNum++){
			// if package is unscheduled, add it avg exec time to deadline
			if (find(scheduled.begin(), scheduled.end(), pNum) == scheduled.end())
			// we suppose that all unscheduled packages execute sequentially
			deadline += data.Workflows(wfNum).GetAvgExecTime(pNum);
		}
	}
	return deadline;
}


ReservedTimeCriteria::~ReservedTimeCriteria(void)
{
}
