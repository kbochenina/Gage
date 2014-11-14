#include "SchedulingMethod.h"
#include "Greedy.h"
#include "Clustered.h"
#include "Gage.h"
#include <memory>
#pragma once
class SchedulingFactory
{
public:
	static unique_ptr<SchedulingMethod> GetMethod(DataInfo &d, int uid, int wfNum);
	SchedulingFactory();
	~SchedulingFactory(void);
};

