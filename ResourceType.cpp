#include "StdAfx.h"
#include "ResourceType.h"


ResourceType::ResourceType(int u, int rC, int nC, double p, vector<BusyIntervals> bi, bool flag, ModelingContext& context)
{
	uid = u;
	numCoresPerOneRes = nC;
	resCount = rC;
	perf = p;
	windows.SetData(bi, context);
	canExecuteOnDiffResources = flag;
}


ResourceType::~ResourceType(void)
{
}

// fix busy intervals after scheduling
void ResourceType::FixBusyIntervals(){
	windows.Fix();
}

// set current busy intervals to fixed
void ResourceType::ResetBusyIntervals(){
	windows.Reset();
}

// set current and fixed busy intervals to init
void ResourceType::SetInitBusyIntervals(){
	windows.SetInit();
}
// get current intervals 
void ResourceType::GetCurrentIntervals(vector<BusyIntervals> &storedIntervals){
	storedIntervals = windows.GetCurrentIntervals();
}
// set current intervals 
void ResourceType::SetCurrentIntervals(vector<BusyIntervals> &storedIntervals){
	windows.SetCurrentIntervals(storedIntervals);
}

// find placement !for 1 processor for execTime
// tbegin and processor is out parameters
bool ResourceType::FindPlacement(const double &execTime, int &tbegin, int& processor, double& deadline) const {
	if (windows.FindPlacement(execTime, tbegin, processor, deadline)) return true;
	return false;
}
// add interval [tbegin; tbegin + execTime] to processor
void ResourceType::AddInterval(const double &execTime, const int &tbegin, const int& processor) {
	vector <int> processorNumber;
	processorNumber.push_back(processor);
	windows.AddDiaps(processorNumber, tbegin, execTime);
}

// check received interval for intersection with existing intervals
bool ResourceType::CanPlace(const int& num, const int& tBegin, const double& execTime){
	const int resNum = num / numCoresPerOneRes;
	const int procNum = num - resNum * numCoresPerOneRes + 1;
	return windows.CanPlace(resNum, procNum, tBegin, execTime);
}


