#include "stdafx.h"
#include "Gage.h"



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
    while (queue.size() != 0 || currentTime < data.GetT()){
        job currentJob = queue.back();
        if (!isWithWindows){
            bool requiresReschedule = FindSchedule(currentJob, out);
        }
        // if there are tasks to schedule, set current time as time of first task in queue
        if (queue.size() != 0)
            currentTime = queue.back().first;
    }

    return res;
}

bool Gage::FindSchedule(job& currentJob, Schedule& out){
    int wfNum = currentJob.second.first, localPNum = currentJob.second.second;
    bool freeResourceWasFound = false, jobWasScheduled = false;
    for (const auto& resIndex: sortedResources){
        // check if this job can be executed on resource of this type 
        vector<int> pResTypes = data.Workflows(wfNum)[localPNum].GetResTypes();
        if (find(pResTypes.begin(), pResTypes.end(), resIndex + 1) == pResTypes.end())
            continue;
        // find actual starting time for the job
        double transferTimeEnd = FindDataTransferTimeEnd(wfNum, localPNum, resIndex, out);
        // find first resource of this type which isn't busy - check index
        for (int i = 0; i < data.Resources(resIndex).GetProcessorsCount(); i++) {
            if (data.Resources(resIndex).CanPlace(i, currentTime, 0)){
                freeResourceWasFound = true;

                break;
            }
        }
        // if schedule was found or there is a need to reschedule, then break
    }
    return true;
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
                int resType = data.GetResourceTypeIndex(proc);
                double datasize = data.Workflows(wfNum).GetTransfer(i, localPNum);
                double bandwidth = data.GetBandwidth(proc, resIndex);
                double transferTime = datasize / bandwidth;
                double currentEnd = sched.get<1>() + sched.get<3>() + transferTime;
                if (currentEnd > transferTimeEnd)
                    transferTimeEnd = currentEnd;
            }
        }
    }
    return transferTimeEnd;
}

bool sortCurrentTime(const job& first, const job& second){
    return first.first > second.first;
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
