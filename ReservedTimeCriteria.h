#pragma once
#include "CriteriaMethod.h"
#include "DataInfo.h"
class ReservedTimeCriteria :
	public CriteriaMethod
{
public:
	ReservedTimeCriteria(DataInfo &d);
	double GetCriteria(const Schedule &in);
	// get criteria value taling into account the average time of unscheduled tasks
	double GetCriteria (const Schedule &in, const int &wfNum);
	bool TendsToMin() {return true;}
	// return true if first value is better than second
	bool IsBetter(double first, double second){ return (first < second ? true : false); }
	~ReservedTimeCriteria(void);
};

