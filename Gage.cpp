#include "stdafx.h"
#include "Gage.h"

bool sortCurrentTime(const job& first, const job& second){
    return first.first > second.first;
}


Gage::Gage(DataInfo &d, int param): SchedulingMethod(d,param)
{
    if (param == 1)
        isWithWindows = false;
    else if (param == 2)
        isWithWindows = true;
    currentTime = 0;
    AddInitJobsToQueue();
    if (!isWithWindows)
        CreateSortedResources();
}

bool sortPerfIndexes(const pair<int, double>& first, const pair<int, double>& second){
    return first.second > second.second;
}

void Gage::CreateSortedResources(){
    // (performance, resourceIndex)
    vector <pair<int, double>> perfIndexes;
    for (int i = 0; i < data.GetResourceCount(); i++)
        perfIndexes.push_back(make_pair(i, data.Resources(i).GetPerf()));
    sort(perfIndexes.begin(), perfIndexes.end(), sortPerfIndexes);
    for (const auto&i : perfIndexes){
        sortedResources.push_back(i.first);
    }
}

double Gage::GetWFSchedule(Schedule &out){
    double res = 0.0;
    // while we have jobs to schedule and simulation time is less that T
    while (queue.size() != 0 && currentTime < data.GetT()){
        job currentJob = queue.back();
        if (!isWithWindows){
            FindSchedule(currentJob, out);
        }
        // if there are tasks to schedule, set current time as time of first task in queue
        if (queue.size() != 0)
            currentTime = queue.back().first;
    }
    DeleteFakeIntervals();
    return res;
}

void Gage::DeleteFakeIntervals(){
    for (const auto& interval : fakeIntervals){
        int resIndex = interval.get_head(),
            procIndex = interval.get<1>(),
            tBegin = interval.get<2>(),
            execTime = interval.get<3>();
        data.Resources(resIndex).DeleteInterval(execTime, tBegin, procIndex);
    }
}

void Gage::FindSchedule(job& currentJob, Schedule& out){
	
    int wfNum = currentJob.second.first, localPNum = currentJob.second.second;
    int globalPNum = data.GetInitPackageNumber(wfNum) + localPNum;
    // find actual starting time for the job
    
    vector<int> pResTypes = data.Workflows(wfNum)[localPNum].GetResTypes();

    bool freeResourceWasFound = false, jobWasScheduled = false, jobWillBeRescheduled = false;
	 double newStartTime = data.GetT();
    for (const auto& resIndex: sortedResources){
        // check if this job can be executed on resource of this type 
        if (find(pResTypes.begin(), pResTypes.end(), resIndex + 1) == pResTypes.end())
            continue;
        double transferTimeEnd = FindDataTransferTimeEnd(wfNum, localPNum, resIndex, out);
        double possibleStartTime = transferTimeEnd > currentTime ? transferTimeEnd + 1 : currentTime;
        bool busyWithOtherJob = false;
        // find first resource of this type which isn't busy - check index
        for (int i = 0; i < data.Resources(resIndex).GetProcessorsCount(); i++) {
            // if processor is free at the time
            if (data.Resources(resIndex).CanPlace(i, possibleStartTime, 0)){
				    freeResourceWasFound = true;
				    double execTime = data.Workflows(wfNum)[localPNum].GetExecTime(resIndex + 1, 1);
                // if job can be placed on this resource
                if (data.Resources(resIndex).CanPlace(i, possibleStartTime, execTime)){
					    // fix busy intervals
					    data.Resources(resIndex).AddInterval(execTime, possibleStartTime, i);
					    // add information to schedule
					    vector<int> processors;
					    processors.push_back(data.GetGlobalProcessorIndex(resIndex, i));
					    out.push_back(make_tuple(globalPNum, possibleStartTime, processors, execTime));
					    // job will be scheduled or rescheduled
					    queue.pop_back();
					    // add ready successors to queue
					    AddReadySuccessors(wfNum, localPNum, out, possibleStartTime + execTime);
					    jobWasScheduled = true;
                   break;
				    }
                // if job cannot be placed on this resource
				    else {
					    double nextSchedTime = GetNextStartTime(resIndex, i, possibleStartTime, execTime, busyWithOtherJob);
                   // if job cannot be placed because of initial time windows, job will be rescheduled later
					    if (!busyWithOtherJob && nextSchedTime < data.GetT()){
						    currentJob.first = nextSchedTime;
						    queue.pop_back();
						    queue.push_back(currentJob);
						    sort(queue.begin(), queue.end(), sortCurrentTime);
						    jobWillBeRescheduled = true;
                      data.Resources(resIndex).AddInterval(nextSchedTime-possibleStartTime, possibleStartTime, i);
                      fakeIntervals.push_back(make_tuple(resIndex, i, possibleStartTime, nextSchedTime-possibleStartTime));
                      break;
					    }
				    }
            }
            // if processor is busy at the time, find next free start time on this processor
            else{
                double nextStartOnProc = FindNextFreeStart(resIndex, i, possibleStartTime);
                if (nextStartOnProc < newStartTime)
                    newStartTime = nextStartOnProc;
            }
        }
		if (jobWasScheduled || jobWillBeRescheduled)
			break;
    }

	if (!freeResourceWasFound) {
		cout << "There was no free resources for job " << localPNum << " of workflow " << wfNum << endl;
      if (newStartTime < data.GetT()){
		    queue.back().first = newStartTime;
		    sort(queue.begin(), queue.end(), sortCurrentTime);
	     }
	 }
	
	if (jobWasScheduled) 
		cout << "Job " << localPNum << " of workflow " << wfNum << " was successfully scheduled" <<  endl;

	// if all resources was busy and there is free window later, reschedule this job with new start time
	
		
	if (jobWillBeRescheduled) 
		cout << "Job " << localPNum << " of workflow " << wfNum << " will be rescheduled" <<  endl;
}


double Gage::FindNextFreeStart(int resIndex, int procIndex, double startTime){
	vector<BusyIntervals> currentIntervals;
	data.Resources(resIndex).GetCurrentIntervals(currentIntervals);
	for (const auto& currentInterval : currentIntervals[procIndex].begin()->second){
		if (currentInterval.second > startTime)
			return currentInterval.second;
	}
	return 0;
}

double Gage::GetNextStartTime(int resIndex, int procIndex, double possibleStartTime, double execTime, bool& busyWithOtherJob){
	vector<BusyIntervals> initIntervals, currentIntervals;
	data.Resources(resIndex).GetInitIntervals(initIntervals);
	data.Resources(resIndex).GetCurrentIntervals(currentIntervals);
	for (const auto& interval : initIntervals[procIndex].begin()->second){
		if (interval.first > possibleStartTime){
			for (const auto& currentInterval : currentIntervals[procIndex].begin()->second){
				if (currentInterval.first > possibleStartTime && 
					currentInterval.first != interval.first){
						busyWithOtherJob = true;
						return 0;
				}
			}
			if (!busyWithOtherJob)
				return interval.first;
		}
	}
	return 0;
}

void Gage::AddReadySuccessors(int wfNum, int localPNum, Schedule &out, double possibleStartTime){
	vector <int> childs;
	data.Workflows(wfNum).GetOutput(localPNum, childs);
	for (const auto& children: childs){
		// if children isn't in queue yet
		bool isChildrenInQueue = false;
		for  (const auto& elem : queue){
			if (elem.second.first == wfNum && elem.second.second == children)
				isChildrenInQueue = true;
		}
		if (isChildrenInQueue)
			continue;
		vector <int> parents;
		data.Workflows(wfNum).GetInput(children, parents);
		bool allParentsScheduled = true;
		for (const auto& parent : parents){
			bool isParentScheduled = false;
			int globalParentNum = data.GetInitPackageNumber(wfNum) + parent;
			for (const auto& sched : out){
				if (sched.get_head() == globalParentNum){
					isParentScheduled = true;
					break;
				}
			}
			if (!isParentScheduled){
				allParentsScheduled = false;
				break;
			}
		}
		if (allParentsScheduled){
			queue.push_back(make_pair(possibleStartTime + 1, make_pair(wfNum, children)));
		}
	}
	sort(queue.begin(), queue.end(), sortCurrentTime);
}

double Gage::FindDataTransferTimeEnd(int wfNum, int localPNum, int resIndex, Schedule& out){
    vector <int> parents;
    data.Workflows(wfNum).GetInput(localPNum, parents);
    double transferTimeEnd = 0.0;
    // transform local numbers to global numbers (necessary to find in schedule)
    for (const auto&i : parents){
        int globalParentNum = data.GetInitPackageNumber(wfNum) + i;
        for (const auto& sched: out){
            if (sched.get_head() == globalParentNum){
                int proc = sched.get<2>()[0];
				    int resFrom = data.GetResourceTypeIndex(proc);
                double transferTime = 0.0;
                if (resFrom != resIndex){
                    double datasize = data.Workflows(wfNum).GetTransfer(i, localPNum);
                    double bandwidth = data.GetBandwidth(resFrom, resIndex);
                    if (bandwidth != 0) 
                        transferTime = datasize / bandwidth;
                }
                double currentEnd = sched.get<1>() + sched.get<3>() + transferTime;
                if (currentEnd > transferTimeEnd)
                    transferTimeEnd = currentEnd;
            }
        }
    }
    return transferTimeEnd;
}



void Gage::AddInitJobsToQueue(){
    // for each workflow
    for (unsigned i = 0; i < data.GetWFCount(); i++){
        // for all packages (check if workflows are numbered from zero)
        for (unsigned j = 0; j < data.Workflows(i).GetPackageCount(); j++){
            if (data.Workflows(i).IsPackageInit(j)){
                queue.push_back(make_pair(data.Workflows(i).GetStartTime(), make_pair(i, j)));
            }
        }
    }
    sort(queue.begin(), queue.end(), sortCurrentTime);
}

Gage::~Gage(void)
{
}
