#include "Efficiency.h"
#include "Metrics.h"
#include "Test.h"
#include <memory>

#pragma once


class Scheduler 
{
	// ATTRIBUTES
	// codes of scheduling methods for all WFs
	vector<int> methodsSet;
	// schedule for all WFs after optimizing
	Schedule fullSchedule;
	// reference on dataInfo
	DataInfo &data;
	// pointer to xml writer
	unique_ptr <ScheduleToXML> xmlWriter;
	// efficiency of fullSchedule
	double maxEff;
	// maximum possible efficiency
	double maxPossible;
	// numbers of already scheduled WFs
	vector<int> scheduledWFs;
	// efficiency function member
	unique_ptr<Efficiency> eff;
	// OPERATIONS
	// classical staged scheme
	double StagedScheme(int initWfNum);
	// staged scheme with known WF order
	void StagedScheme(vector <int>& wfOrder);
	// scheduling like one big WF
	void SimpleSched();
	// scheduling ordered due to prioretization criteria
	void OrderedScheme(int criteriaNumber);
	// efficiency ordered staged scheme
	void EfficiencyOrdered();
	// clustered scheme
	void Clustered();
   // gage algoritm
   void Gage(unsigned int);
	// add to file info about schedule
	void PrintOneWFSched(ofstream & out, Schedule & sched, int wfNum);
	// add to res file additional schedule information
	void PrintFooter(ofstream & out, vector<double>&eff);
public:
	Scheduler(DataInfo& d);
	// fulfill the variable methodsSet according to nonexistent algorithm
	void SetSchedulingStrategy(int strategyNumber);
	// get schedule according to scheduleVariant parameter
	// 1 - classical staged scheme
	// 2 - staged scheme with known order
	void GetSchedule(int scheduleVariant);
	// get metrics
	void GetMetrics(string filename, string name, string metricsFileName);
	// test full schedule
	void TestSchedule();
	~Scheduler(void);
};

