#include "Intervals.h"
#include "ModelingContext.h"


#pragma once

typedef vector<pair<double,int>> TimeCore;

class ResourceType
{
	// resource number - from 1
	int uid;
	// cores count per one resource of this type
	int numCoresPerOneRes;
	// resource count
	int resCount;
	// performance (GFlop/s)
	double perf;
	// busy time windows
	Intervals windows;
	// if one package can execute of different resources, variable is true
	bool canExecuteOnDiffResources;
public:
	ResourceType(int u, int nC, int rC, double p, vector<BusyIntervals> i, bool flag, ModelingContext& context);
	// check if elements of vector timeCores (execTime, coreCount) can be placed in the moment tbegin
	// to a resources of this type (also according to checkType). Possible placement will be stored
	// to out variable. If placement is impossible, function will return false
	bool Check(const vector<TimeCore>& timeCores, int tBegin, bool checkType, vector<vector<int>>&out);
	inline int GetProcessorsCount() const {return numCoresPerOneRes * resCount; }
	inline double GetPerf() const {return perf;}
	void FixBusyIntervals();
	void ResetBusyIntervals();
	void SetInitBusyIntervals();
	void GetCurrentIntervals(vector<BusyIntervals> &storedIntervals);
	void SetCurrentIntervals(vector<BusyIntervals> &storedIntervals);
	void GetCurrentWindows(Intervals & out) const { out = windows; }
	// find placement !for 1 processor for execTime
	// tbegin and processor is out parameters
	bool FindPlacement(const double &execTime, int &tbegin, int& processor, double &deadline) const;
	// add interval [tbegin; tbegin + execTime] to processor
	void AddInterval(const double &execTime, const int &tbegin, const int& processor);
	// check received interval for intersection with existing intervals
	bool CanPlace(const int& num, const int& tBegin, const double& execTime);
	//void PrintIntervals(int processor) {windows.PrintIntervals(processor);}
	~ResourceType(void);
};

