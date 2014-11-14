#include "stdafx.h"
#include "DataInfo.h"

#pragma once
class CriteriaMethod
{
protected:
	// reference on model data
	DataInfo& data;
public:
	CriteriaMethod(DataInfo &d) : data(d) {}
	// get criteria value for current schedule and init data
	virtual double GetCriteria(const Schedule &in) = 0;
	// get criteria value taling into account the average time of unscheduled tasks
	virtual double GetCriteria (const Schedule &in, const int &wfNum) = 0;
	// return true if criteria tends to minimum
	virtual bool TendsToMin() = 0;
	// return true if first value is better than second
	virtual bool IsBetter(double first, double second) = 0;
	// print info
	//virtual void printInfo() = 0;
	~CriteriaMethod(void);
};

