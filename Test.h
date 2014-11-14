#include "stdafx.h"
#include "DataInfo.h"

#pragma once
class Test
{
	DataInfo &data;
	const Schedule &sched;
public:
	Test(DataInfo& d, const Schedule& s) : data(d), sched(s){ data.SetInitBusyIntervals(); }
	bool TestIntervals();
	bool TestWFLinks();
	~Test(void) { data.SetInitBusyIntervals(); } 
};

