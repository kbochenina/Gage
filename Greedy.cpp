#include "stdafx.h"
#include "Greedy.h"

using namespace std;


Greedy::Greedy(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){ 
    finished.clear();
    unscheduled.clear();
}

double Greedy::GetWFSchedule(Schedule &out){
    double eff = 0.0;
    switch (wfNum){
    case -1: {
        eff = GetFullSchedule(out); break;
    }
    default:
        eff = GetOneWFSchedule(out); break;
    }
    return eff;
}


double Greedy::GetFullSchedule(Schedule& out){
    double efficiency = 0.0;
    data.SetPriorities();
    while (finished.size() != data.GetPackagesCount()){
        // get unscheduled package with minimum finishing count
        int current = data.GetNextPackage();
        FindSchedule(out, efficiency, current, false);
 
    }
    return efficiency;
}

double Greedy::GetOneWFSchedule(Schedule& out){
    
    double efficiency = 0.0;
    int index = data.Workflows(wfNum).GetPackageCount() - 1;
    // while all packages are not scheduled
    while (finished.size() != data.Workflows(wfNum).GetPackageCount()){
        // get unscheduled package with minimum finishing count
        int current = data.Workflows(wfNum).GetNextPackage(index--);
        if (find(unscheduled.begin(), unscheduled.end(), current) != unscheduled.end())
            continue;
        FindSchedule(out, efficiency, current, true);
    }
    return efficiency;
}

void Greedy::FindSchedule(Schedule& out, double &efficiency, int pNum, bool forOneWf){
    vector<int> dependsOn;    
    int localNum = 0, processor = 0;
    double deadline = 0.0;
	double tstart = 0.0;
    if (!forOneWf){
        // get wfNum and package local num
        data.GetLocalNumbers(pNum, wfNum, localNum);
		deadline = data.GetDeadline(wfNum);
         // should get latest finishing time of previous packages
        data.Workflows(wfNum).GetInput(localNum, dependsOn);
		tstart = data.Workflows(wfNum).GetStartTime();
		int parentsFound = 0;
		// check the existence of parents in the schedule
		for (Schedule::iterator it = out.begin(); it!= out.end(); it++){
			int currentPackage = it->get_head(), currentWf, currentLocal;
			data.GetLocalNumbers(currentPackage, currentWf, currentLocal);
			if (currentWf == wfNum){
				for (size_t i = 0; i < dependsOn.size(); i++){
					if (currentLocal == dependsOn[i]){
						parentsFound++;
						break;
					}
				}
			}
		}
		// cannot find schedule for all the parents of current task
		if (parentsFound != dependsOn.size()) return;
        //deadline = data.Workflows(wfNum).GetDeadline();
    }
    else {
			data.Workflows(wfNum).GetInput(pNum, dependsOn);
			tstart = data.Workflows(wfNum).GetStartTime();
			deadline = data.Workflows(wfNum).GetDeadline();
			localNum = pNum; 
    }
   
    // (resType, finishingTime, transferDataSize) for all input packages
    vector <boost::tuple<int, double, double>> commInfo;
      
    for (size_t i = 0; i < dependsOn.size(); i++){
        // set global numbers
        for (auto& j : out){
            int add = 0;
            if (!forOneWf) add += pNum - localNum;
            // for input package
            if (boost::get<0>(j) == dependsOn[i] + add)
            {
                int resType = data.GetResourceTypeIndex(boost::get<2>(j)[0]);
                double tEnd = boost::get<1>(j) + boost::get<3>(j) + 1;
                int initPackageNumber = data.GetInitPackageNumber(wfNum);
                double transfer = 0.0;
                if (!forOneWf)
                    transfer = data.Workflows(wfNum).GetTransfer(boost::get<0>(j) - initPackageNumber, 
                    pNum - initPackageNumber);
                else 
                    transfer = data.Workflows(wfNum).GetTransfer(boost::get<0>(j), pNum);
                commInfo.push_back(make_tuple(resType, tEnd, transfer));
                /*if (tEnd > expectedBegin)
                    expectedBegin = static_cast<int>(tEnd) + 1;*/

            }
        }
    }
   
    // getting possible types of resources
    vector <int> resTypes = data.Workflows(wfNum)[localNum].GetResTypes();
    vector <double> execTime;
    for (size_t i = 0; i < resTypes.size(); i++){
        execTime.push_back(data.Workflows(wfNum)[localNum].GetExecTime(resTypes[i],1));
    }
    vector<int> viewedResources;
    // processor, tbegin, tend, resIndex
    boost::tuple<int, double, double, int> savedPlan;
    double bestTimeEnd = std::numeric_limits<double>::infinity();
    bool planWasFound = false;
    for (auto &res : resTypes){
            // resources indexed from 1
            res -=1;
            //int tbegin = 0;
			int tbegin = tstart;
            // get the expected begin time for current resource type
            for (const auto& in : commInfo){
                double bandwidth = data.GetBandwidth(in.get_head(), res);
                double currResBegin = 0.0;
                if (bandwidth == 0)
                    currResBegin = in.get<1>();
                // currBegin = tEnd + transferSize/bandwidth
                else
                    currResBegin = in.get<1>() + in.get<2>()/bandwidth;
                if (currResBegin > tbegin){
                    // for rounding to upper int
                    currResBegin += 0.5;
                    tbegin = static_cast<int>(currResBegin);
                }
            }
			if (tbegin > deadline) continue;
            if (data.Resources(res).FindPlacement(execTime[res], tbegin, processor, deadline)){
                if (tbegin + execTime[res] < bestTimeEnd){
                    savedPlan.get<0>() = processor; 
                    savedPlan.get<1>() = tbegin;
                    savedPlan.get<2>() = tbegin + execTime[res]; 
                    savedPlan.get<3>() = res;
                    bestTimeEnd = tbegin + execTime[res];
                }
                planWasFound = true;
            }
    }
       
    if (planWasFound){
        int tbegin = static_cast<int>(savedPlan.get<1>());
        double execTime = savedPlan.get<2>()-tbegin;
        data.Resources(savedPlan.get<3>()).AddInterval(execTime, 
            tbegin, savedPlan.get<0>());
        finished.push_back(pNum);
        int procGlobalIndex = data.GetGlobalProcessorIndex(savedPlan.get<3>(), savedPlan.get<0>());
        vector <int> processors;
        processors.push_back(procGlobalIndex);
        out.push_back(make_tuple(pNum, tbegin, processors, execTime));
        // add efficiency value
        int tend = static_cast<int>((tbegin + execTime > deadline) ? deadline : tbegin + execTime);
        efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
        }
    else {
        //cout << "Can not find placement for package " << current << endl;
        finished.push_back(pNum);
        int initPackageNum = data.GetInitPackageNumber(wfNum);
        // get all successors of this package
        vector <int> successors;
        if (!forOneWf) 
            data.Workflows(wfNum).GetSuccessors(pNum - initPackageNum, successors);
        else data.Workflows(wfNum).GetSuccessors(pNum, successors);
            
        for (auto & i : successors){
            // transform local to global
            if (!forOneWf) i += initPackageNum;
            if (find(finished.begin(), finished.end(),i) == finished.end())
                finished.push_back(i);
            if (forOneWf){
                 if (find(unscheduled.begin(),unscheduled.end(), i) == unscheduled.end()) 
                    unscheduled.push_back(i);

            }
                               
        }
        // remove successor from priority queue
        if (!forOneWf) data.RemoveFromPriorities(successors);
    }

}

Greedy::~Greedy(void)
{
}
