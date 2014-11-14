#include "stdafx.h"
#include "Metrics.h"

void Metrics::GetMetrics(Schedule & s, string name, string metricsFileName){
   sched = s;
   avgFine = 0.0;
   full.open(metricsFileName, ios::app);
   full << name.c_str() << endl;
   AvgUnfinischedTime();
   AvgReservedTime();
   AvgCCR();
   IntegralCriterion();
   full << "***************************************************"  << endl;
   full.close();
   out.close();
}

void Metrics::AvgCCR(){

}

void Metrics::AvgUnfinischedTime(){
   // check
   int wfCount = data.GetWFCount();
   // fines
   vector <double> fines(wfCount,0);
   vector <double> unfinishedTimes(wfCount,0);
   vector <int> unfinishedTasks(wfCount,0);
   vector <double> wfEnds(wfCount,0);
   vector <int> unscheduledTasks(wfCount,0);
   vector <vector<int>> scheduledTasks;
   vector <double> execTimePerWf(wfCount,0);
   vector <double> commTimePerWf(wfCount,0);
   vector <vector<int>> resTypesPerWfPackages;
   vector<double> fineMaxWFs;
   resTypesPerWfPackages.resize(wfCount);
   for (int i = 0; i < wfCount; i++)
       resTypesPerWfPackages[i].resize(data.Workflows(i).GetPackageCount());
   scheduledTasks.resize(wfCount);
   double summUnfinishedTime = 0;
   int summUnfinishedTasks = 0;
   vector<int> violatedDeadlines;
   // for each p
   for (size_t i = 0; i < sched.size(); i++){
      int pNum = sched[i].get<0>();
      int tBegin = sched[i].get<1>();
      double execTime = sched[i].get<3>();
      int processor = sched[i].get<2>()[0];
      int type = data.GetResourceTypeIndex(processor) ; // !
      int wfNum, localNum;
      data.GetLocalNumbers(pNum, wfNum, localNum);
      resTypesPerWfPackages[wfNum][localNum] = type;
      scheduledTasks[wfNum].push_back(localNum);
      execTimePerWf[wfNum] += execTime;
      vector <int> in;
      data.Workflows(wfNum).GetInput(localNum, in);
      for (size_t j = 0; j < in.size(); j++){
          double amount = data.Workflows(wfNum).GetTransfer(in[j], localNum);
          double commRate = data.GetBandwidth(resTypesPerWfPackages[wfNum][in[j]], type);
          if (commRate) 
              commTimePerWf[wfNum] += amount/commRate;
      }
	  double taskEnd = tBegin + execTime;
	  if (taskEnd > wfEnds[wfNum]) wfEnds[wfNum] = taskEnd;
      // DEADLINES FEATURE
      if ( taskEnd > data.GetDeadline(wfNum) ){
		 if (find(violatedDeadlines.begin(),violatedDeadlines.end(),wfNum)==violatedDeadlines.end())
			 violatedDeadlines.push_back(wfNum);
         unfinishedTimes[wfNum] += taskEnd - data.GetDeadline(wfNum);
		 reservedTime[wfNum] = 0;
         unfinishedTasks[wfNum]++;
         summUnfinishedTasks++;
      }
   }
   //cout << "Average unfinished times and tasks" << endl << "***************************************************"  << endl;
   //cout << "Unfinished times: " << endl;
    vector<double> unschTimes(data.GetWFCount(), 0);

   for (size_t i = 0; i < unfinishedTimes.size(); i++){
      // if we have some unscheduled tasks
      if (scheduledTasks[i].size() != data.Workflows(i).GetPackageCount()){
         if (find(violatedDeadlines.begin(),violatedDeadlines.end(),i)==violatedDeadlines.end())
			 violatedDeadlines.push_back(i);
         for (int j = 0; j < data.Workflows(i).GetPackageCount(); j++ ){
            // if package was not scheduled
            if (find(scheduledTasks[i].begin(), scheduledTasks[i].end(), j) == scheduledTasks[i].end()){
			   int globalNum = data.GetInitPackageNumber(i)+j;
			   unschTimes[i] += data.Workflows(i).GetAvgExecTime(j) + data.GetAvgTransferFrom(globalNum);
               //unfinishedTimes[i] += unschTimes[i];
			   unscheduledTasks[i]++;
               reservedTime[i] = 0.0;
            }
         }
      }
	  //summUnfinishedTime += unfinishedTimes[i];
      /*cout << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
      << " unscheduled tasks: " << unscheduledTasks[i] 
      << " scheduled tasks: " << scheduledTasks[i].size() << endl;*/
   }
   cout << "Workflows count: " << data.GetWFCount() << endl;

   for (int i = 0; i < data.GetWFCount(); i++){
	   // calculating fineMax
	  double fineMax = 0;
	  for (int j = 0; j < data.Workflows(i).GetPackageCount(); j++){
				fineMax += data.Workflows(i).GetAvgExecTime(j);
				int globalNum = data.GetInitPackageNumber(i) + j;
				fineMax += data.GetAvgTransferFrom(globalNum);
	  }
	  // if all tasks are scheduled
	  if (unscheduledTasks[i] == 0){
		  // if schedule is partly admissible
		  //fineMax = data.GetMaxLastTasksExecTime(i);
		  if (unfinishedTasks[i] != 0)
			fines[i] = wfEnds[i] - data.GetDeadline(i);
		 
		  //cout << "Partly admissible " << endl; 
      }
	  // if schedule is incomplete
	  else {
		  double deadline = data.GetDeadline(i);
		  // specific case when part of tasks were finished before deadline,
		  // and another part was not scheduled
		  if (wfEnds[i] < deadline)
			  fines[i] = unschTimes[i];
		  else
		  fines[i] = unschTimes[i] + unfinishedTimes[i];
		  if (fines[i] < 0)
			  cout << "tend " << wfEnds[i] << "unshTimes " << unschTimes[i] << "deadline " << deadline << endl;
		  //cout << "Incomplete " << endl;
	  }
	  out << "Workflow " << i+1 << " Fine: " << fines[i] << " Finemax: " << fineMax;
	  cout << "Workflow " << i+1 << " Fine: " << fines[i] << " Finemax: " << fineMax << endl;
	  fineMaxWFs.push_back(fineMax);
	  if (fineMax != 0) {
		  //fines[i] /= fineMax;
		  out << " Ratio: " << fines[i]/fineMax << " \n\t\tTstart: " << data.Workflows(i).GetStartTime() << " Deadline: " 
			  << data.Workflows(i).GetDeadline() << " Tend: " << wfEnds[i];
		 // cout << " Ratio: " << fines[i];
	  }
	  out << endl;
	 // cout << endl;
   }

   /*cout << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   cout << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;*/
   out << "Deadlines violated: " << violatedDeadlines.size() << endl;
   out << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   cout << "Deadlines violated: " << violatedDeadlines.size() << endl;
   cout << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   full << "Deadlines violated: " << violatedDeadlines.size() << endl;
   full << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   out << "Average unfinished times and tasks" << endl << "***************************************************" << endl;
   out << "Unfinished times: " << endl;
   for (size_t i = 0; i < unfinishedTimes.size(); i++){
      out << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
      << " unscheduled tasks: " << unscheduledTasks[i] 
      << " scheduled tasks: " << scheduledTasks[i].size() << endl;
   }
   out << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   out << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;

   cout << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   cout << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;
   full << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   full << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;
   int sumUnsched = 0;
   for (size_t i = 0; i < unscheduledTasks.size(); i++)
      sumUnsched += unscheduledTasks[i];

   out << "Unscheduled tasks count: " << sumUnsched << endl;
   out << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;
   cout << "Unscheduled tasks count: " << sumUnsched << endl;
   cout << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;
   full << "Unscheduled tasks count: " << sumUnsched << endl;
   full << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;

   avgFine = 0.0;
   double maxFine = 0.0;
    for (int i = 0; i < fineMaxWFs.size(); i++) {
	     if (fineMaxWFs[i] > maxFine)
			   maxFine = fineMaxWFs[i];
	  }
   
   int count = 0;
   for (auto& i: fines) {
	   avgFine += i / fineMaxWFs[count++];
   }
   avgFine /= data.GetWFCount();

  // double maxAvg = maxFine / count;
   //cout << "MaxAvgFine = " << maxAvg << endl;

   // avg fine diff
   count = 0;
   double sum = 0.0;
   for (int i = 0; i < fines.size(); i++) {
	   for (int j = 0; j < fines.size(); j++){
		   if (i!=j){
			if (abs(fines[i]-fines[j]) > sum)
				sum = abs(fines[i]-fines[j]);
			//maxFine += max(fines[i], fines[j]);
			count++;
		   }
	   }
   }

  // sum /= count;
   
   //cout << "Avg diff fine :" << sum << endl;

   //if (maxAvg > 0) fairness = sum / maxAvg;
   //else fairness = 0;
   fairness = sum / maxFine;
   cout << "maxFine " << sum << " max possible fine " << maxFine << endl;
   fairness = 1 - fairness;
  // cout << "Avg diff " << sum << " MaxAvgDiff " << maxAvg << endl;
   cout << "Fairness: " << fairness << endl;

   out << "Average fine: " << avgFine << endl;
   cout << "Average fine: " << avgFine << endl;
   full << "Average fine: " << avgFine << endl;



   double avgCCR = 0.0;

   for (size_t i = 0; i < wfCount; i++){
       double currCCR = commTimePerWf[i]/execTimePerWf[i];
       avgCCR += currCCR;
       
       /*out << "Workflow " << i + 1 << endl;
       out << "Execution time:" << execTimePerWf[i] << endl;
       out << "Communication time:" << commTimePerWf[i] << endl;
       out << "CCR: " << currCCR << endl;
      
       full << "Workflow " << i + 1 << endl;
       full << "Execution time:" << execTimePerWf[i] << endl;
       full << "Communication time:" << commTimePerWf[i] << endl;
       full << "CCR: " << currCCR << endl;*/
   }

   avgCCR /= wfCount;
   out << "Average CCR: " << avgCCR << endl;
   full << "Average CCR: " << avgCCR << endl;
}


void Metrics::IntegralCriterion(){
	//double crit = 0.5 * eff + 0.5 * (1 - avgFine);
	double crit = 0.5* (1 - avgFine) + 0.5 * fairness ;
	out << "Integral criterion: " << crit << endl;
	cout << "Integral criterion: " << crit << endl;
	full << "Integral criterion: " << crit << endl;
}

void Metrics::AvgReservedTime(){
   double summReserved = 0.0;
   for (size_t i = 0; i < sched.size(); i++){
      int pNum = sched[i].get<0>();
      int tBegin = sched[i].get<1>();
      double execTime = sched[i].get<3>();
      int wfNum, localNum;
      data.GetLocalNumbers(pNum, wfNum, localNum);
      if (reservedTime[wfNum]!=0){
         // if this is the last package of WF
         if (data.Workflows(wfNum).IsPackageLast(localNum)){
            // get its reserved time
            double packageReservedTime = data.GetDeadline(wfNum) - (tBegin + execTime);
            // if this time is positive
            if ( packageReservedTime > 0 ){
               // if it is first positive reserve, save it
               if (reservedTime[wfNum] == numeric_limits<double>::infinity())
                  reservedTime[wfNum] = packageReservedTime;
               // otherwise if saved reserve > than new reserve
               // we should save new reserve
               else if (packageReservedTime < reservedTime[wfNum])
                  reservedTime[wfNum] = packageReservedTime;
            }
            else reservedTime[wfNum] = 0;
         }
      }
   }
   //cout << "Reserved times" << endl << "***************************************************"  << endl;
   for (size_t i = 0; i < reservedTime.size(); i++){
      //cout << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
      summReserved += reservedTime[i];
   }
   cout << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;

   out << "Reserved times" << endl << "***************************************************"  << endl;
   for (size_t i = 0; i < reservedTime.size(); i++){
      out << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
   }
   out << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   //cout << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   full << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   out << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
  // cout << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
   full << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
}

Metrics::~Metrics(void)
{
}
