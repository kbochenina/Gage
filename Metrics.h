#include "DataInfo.h"

#pragma once
class Metrics
{
	// reference on dataInfo
	DataInfo &data;
	Schedule sched;
	ofstream out;
	ofstream full;
	string filename;
	double avgFine;
	double fairness;
	double eff;
	vector<double> reservedTime;
	void AvgUnfinischedTime();
	void AvgReservedTime();
    void AvgCCR();
	void IntegralCriterion();
public:
	Metrics(DataInfo& md, string filename, double e) : data(md){   eff = e; out.open(filename); reservedTime.resize(data.GetWFCount());
														for (size_t i = 0; i < reservedTime.size(); i++) reservedTime[i] = numeric_limits<double>::infinity(); }
	void GetMetrics(Schedule & sched, string schemeName, string metricsFileName);
	~Metrics(void);
};

