#include "DataInfo.h"
#include "Efficiency.h"
#include <memory>
#pragma once

// provides finding schedule for one WF
class SchedulingMethod
{
	// scheduling method uid
	int uid;
protected:
	// reference on model data
	DataInfo& data;
	// current wfNum
	int wfNum;
	// efficiency finder
	unique_ptr<Efficiency> eff;
public:
	SchedulingMethod();
	SchedulingMethod(DataInfo &d, int u, int w) : data(d), uid(u), wfNum(w) {
		eff = unique_ptr<Efficiency> ( new Efficiency(2.00 / data.GetprocessorsCount(), data.GetT()) );
	}
	SchedulingMethod(DataInfo &d, int u) : data(d), uid(u), wfNum(0){
		eff = unique_ptr<Efficiency> ( new Efficiency(2.00 / data.GetprocessorsCount(), data.GetT()) );
	}
	// get schedule for current method and WF
	// return value is efficiency
	// resulting schedule is stored in out parameter
	virtual double GetWFSchedule(Schedule &out) = 0;
	~SchedulingMethod(void);
};

