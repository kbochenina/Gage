#include "StdAfx.h"
#include "Scheduler.h"
#include "ScheduleToXML.h"
#include "SchedulingFactory.h"
#include "CriteriaFactory.h"
#include "direct.h"

using namespace std;

enum gage { GAGEWITHOUTWINDOWS = 1, GAGEWITHWINDOWS = 2 };

Scheduler::Scheduler( DataInfo& d ): data(d)
{
   methodsSet.resize(data.GetWFCount());
   maxEff = 0.0;
   xmlWriter = unique_ptr<ScheduleToXML>(new ScheduleToXML(data));
   data.SetWfPriorities();
   eff = unique_ptr<Efficiency>(new Efficiency(2.00 / data.GetprocessorsCount(), data.GetT()));
   maxPossible = 0.0;
   for (int i = 0; i < data.GetResourceCount(); i++){
      Intervals initIntervals;
      data.Resources(i).GetCurrentWindows(initIntervals);
      vector <BusyIntervals> current;
      current = initIntervals.GetCurrentIntervals();
      for (size_t j = 0; j < current.size(); j++){
         BusyIntervals & processorIntervals = current[j];
         for (auto it = processorIntervals.begin(); it != processorIntervals.end(); it++){
            for (size_t k = 0; k < it->second.size(); k++){
               maxPossible += eff->EfficiencyByPeriod(1, it->second[k].first, it->second[k].second);
            }
         }
      }
   }
   maxPossible = 1.00 - maxPossible;
}


Scheduler::~Scheduler(void)
{
}

void Scheduler::SetSchedulingStrategy(int strategyNumber)
{
   switch (strategyNumber)
   {
      // only Bellman
   case 1: for (unsigned int i = 0; i < methodsSet.size(); i++)
            methodsSet[i] = 1;
      break;
      // only Greedy
   case 2: for (unsigned int i = 0; i < methodsSet.size(); i++)
            methodsSet[i] = 2;
	   break;
	   // clustered scheme
   case 3: for (unsigned int i = 0; i < methodsSet.size(); i++)
            methodsSet[i] = 3;
       break;
       // gage algorithm
   case 4: for (unsigned int i = 0; i < methodsSet.size(); i++)
            methodsSet[i] = 4;
      break;
      // mixed
   }
}

// implements staging scheme for finding the schedule for WFs set
// <PRE> 0 <= firstWfNum < data.workflows.size()
double Scheduler::StagedScheme(int firstWfNum){
   cout << "StagedScheme(int) was called\n";
   try{
      int wfCount = data.GetWFCount();
      if (firstWfNum < 0 || firstWfNum > wfCount) 
         throw UserException("Scheduler::StagedScheme(int) error. Wrong init workflow number");
      
      Schedule oneWFsched;
      xmlWriter->SetXMLBaseName("Staged_");
         
      vector <double> eff;
      // applying settings of scheduling method for initial WF
      unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[firstWfNum], firstWfNum);
      // getting schedule for first WF
      double oneStepStart = clock();
      eff.push_back(method->GetWFSchedule(oneWFsched));

      // set local to global packages
      int initNum = data.GetInitPackageNumber(firstWfNum);
      for (size_t i = 0; i < oneWFsched.size(); i++)
         oneWFsched[i].get<0>() += initNum;


      fullSchedule = oneWFsched;
      
      //cout << "Elapsed time: " << (clock()-oneStepStart)/1000.0 << " sec" << endl;
      scheduledWFs.push_back(firstWfNum);	
      //xmlWriter->CreateXML(oneWFsched, firstWfNum);
      // write result to XML
      data.FixBusyIntervals();

      
      // write result to res file
      //PrintOneWFSched(res, oneWFsched, firstWfNum);
      
      
      // we need to store current busy intervals
      // of schedule that give the best efficiency
      // current best schedule is stored in oneWFsched
      vector<vector <BusyIntervals>> storedIntervals;
      Schedule storedSched;

      while (scheduledWFs.size() != wfCount ){
         //cout << "Stage " << scheduledWFs.size() + 1 << endl;
         double stageMaxEff = numeric_limits<double>::infinity();
         int bestWfNum = -1;
         for (int i = 0; i < wfCount; i++){
            // if this WF wasn't scheduled yet
            if (find(scheduledWFs.begin(), scheduledWFs.end(), i) == scheduledWFs.end()){
               //cout << "CurrentWfNum = " << i << " ";
               oneStepStart = clock();
               method = SchedulingFactory::GetMethod(data, methodsSet[i], i);
               oneWFsched.clear();
               double currentEff = method->GetWFSchedule(oneWFsched);
               //cout << "Elapsed time: " << (clock()-oneStepStart)/1000.0 << " sec" << endl;
               /*ReadData(i);
               directBellman = false;
               BackBellmanProcedure();
               directBellman = true;
               double currentEff = DirectBellman(i);*/
               if (stageMaxEff > currentEff){
                  stageMaxEff = currentEff;
                  bestWfNum = i;
                  storedSched = oneWFsched;
                  storedIntervals.clear();
                  data.GetCurrentIntervals(storedIntervals);
                  //GetBestBusyIntervals(bestBusyIntervals);
               }
               data.ResetBusyIntervals(); // newfag in my program
               //states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
            }
         }
         // set local to global packages
         int initNum = data.GetInitPackageNumber(bestWfNum);
         for (size_t i = 0; i < storedSched.size(); i++)
            storedSched[i].get<0>() += initNum;

         copy(storedSched.begin(), storedSched.end(), back_inserter(fullSchedule));
         //copy(bestStagesCores.begin(), bestStagesCores.end(),back_inserter(allStagesCores));
         scheduledWFs.push_back(bestWfNum);
         //usedNums = scheduledWFs; ???
         //stagesCores = bestStagesCores;
         //currentWfNum = bestWfNum;
         eff.push_back(stageMaxEff);
         // set current intervals as stored intervals
         data.SetCurrentIntervals(storedIntervals);
         // write result to XML
         // xmlWriter->CreateXML(storedSched, bestWfNum);
         // write result to res file
      //	PrintOneWFSched(res, storedSched, bestWfNum);
           
         data.FixBusyIntervals();
         
         /*SetBestBusyIntervals(bestBusyIntervals);
         FixNewBusyIntervals();
         BellmanToXML(true);*/
         //std::system("pause");
      }
      /*usedNums = scheduledWFs; ???
      SetFirstBusyIntervals();
      stagesCores = allStagesCores;
      BellmanToXML(false);*/
      //PrintFooter(res, eff);
      double sumEff = 0.0;
      for (size_t i = 0; i < eff.size(); i++)
         sumEff += eff[i];
      
      data.SetInitBusyIntervals();
      //xmlWriter->CreateXML(fullSchedule, -1);
      //res.close();
      cout << "Max eff: " << sumEff/maxPossible << endl;
      //cout << "Elapsed time: " << (clock()-stagedT)/1000.0 << " sec" << endl;
      return sumEff/maxPossible ;
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void Scheduler::StagedScheme(vector<int>& order){
   cout << "StagedScheme(vector<int>&) was called\n";
}

void Scheduler::GetSchedule(int scheduleVariant){
   Schedule storedSched;
   data.SetInitBusyIntervals();
   fullSchedule.clear();
   int bestStage = 0;
   ofstream resTime("time.txt", ios::app);
   ofstream full("fullmetrics.txt", ios::app);
   double t = 0, end = 0, stagedTime = 0;
   
   switch (scheduleVariant)
   {
      case 1:
         resTime << "Staged scheme: " << endl;
         //data.SetWfPriorities();
         maxEff = 0;
         for (int i = 0; i < data.GetWFCount(); i++) {
            scheduledWFs.clear();
            t = clock();
            double eff = StagedScheme(i);
            stagedTime = (clock() - t)/1000.0;
            end += stagedTime;
            resTime << "Time of executing stage " << i+1 << " " << stagedTime << endl;
            if (eff > maxEff){
               maxEff = eff;
               storedSched = fullSchedule;
               bestStage= i;
            }
         }
         cout << "Time of executing staged scheme " << end << endl;
         resTime << "Time of executing staged scheme " << end << endl;
         cout << "Average time of stage " << end/data.GetWFCount() << endl;
         resTime << "Average time of stage " << end/data.GetWFCount() << endl;
         fullSchedule = storedSched;
         xmlWriter->CreateXML(fullSchedule);
         cout << "Best stage: " << bestStage << endl;
         break;
      case 2:
      {
         t = clock();
         EfficiencyOrdered();
         end = (clock() - t)/1000.0;
         cout << "Time of executing efficiency ordered scheme " << end << endl;
         resTime << "Time of executing efficiency ordered scheme " << end << endl;
         full << "Time of executing efficiency ordered scheme " << end << endl;
         full << "Efficiency ordered scheme eff: " << maxEff << endl;
         break;
      }
      case 3: 
         // simple sched
         t = clock();
         SimpleSched();
         end = (clock() - t)/1000.0;
         cout << "Time of executing simple scheduling algorithm " << end << endl;
         resTime << "Time of executing simple scheduling algorithm " << end << endl;
         full << "Time of executing simple scheduling algorithm " << end << endl;
         full << "Sumple sched eff: " << maxEff << endl;
         break;
      case 4:
         // reserved_ordered sched
         //data.SetWfPriorities();
         t = clock();
         OrderedScheme(1);
         end = (clock() - t)/1000.0;
         cout << "Time of executing reserved ordered scheme " << end << endl;
         resTime << "Time of executing reserved ordered scheme " << end << endl;
         full << "Time of executing reserved ordered scheme " << end << endl;
         full << "Reserved scheme eff: " << maxEff << endl;
         break;
	  case 5:
		  // clustered scheme
		 t = clock();
         Clustered();
         end = (clock() - t)/1000.0;
         cout << "Time of executing clustered scheme " << end << endl;
         resTime << "Time of executing clustered scheme " << end << endl;
         full << "Time of executing clustered scheme " << end << endl;
         full << "clustered eff: " << maxEff << endl;
         break;
      case 6:
		  // gage without windows
		   t = clock();
         Gage(GAGEWITHOUTWINDOWS);
         end = (clock() - t)/1000.0;
         cout << "Time of executing gage without windows " << end << endl;
         resTime << "Time of executing gage without windows " << end << endl;
         full << "Time of executing gage without windows " << end << endl;
         full << "gage without windows eff: " << maxEff << endl;
         break;
      case 7:
		  // gage with windows
		   t = clock();
         Gage(GAGEWITHWINDOWS);
         end = (clock() - t)/1000.0;
         cout << "Time of executing gage with windows " << end << endl;
         resTime << "Time of executing gage with windows " << end << endl;
         full << "Time of executing gage with windows " << end << endl;
         full << "gage with windows eff: " << maxEff << endl;
         break;
      default:
         break;
      }
   resTime.close();
   full.close();
}

void Scheduler::Gage(unsigned int param){
    unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[0], param);
	 // get full schedule
	 maxEff = method->GetWFSchedule(fullSchedule);
	 maxEff = maxEff/maxPossible;
	 cout << "Efficiency: " << maxEff << endl;
    //data.FixBusyIntervals()
    string s = "Gage";
    if (param == 6)
        s += "WithoutWindow_";
    else if (param == 7)
        s += "WithWindows";
    xmlWriter->SetXMLBaseName(s);
    // write result to XML
    xmlWriter->CreateXML(fullSchedule);
    data.SetInitBusyIntervals();
}

void Scheduler::Clustered()
{
	 unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[0], -1);
	 // get full schedule
	 maxEff = method->GetWFSchedule(fullSchedule);
	 maxEff = maxEff/maxPossible;
	 cout << "Efficiency: " << maxEff << endl;
     //data.FixBusyIntervals();
     xmlWriter->SetXMLBaseName("Clustered_");
     // write result to XML
     xmlWriter->CreateXML(fullSchedule);
     data.SetInitBusyIntervals();
}

void Scheduler::EfficiencyOrdered(){
   try{
      maxEff = 0.0;
      // unscheduled WF numbers
      vector <int> unscheduled;
      for (int i = 0; i < data.GetWFCount(); i++)
         unscheduled.push_back(i);
      int stage = 0;
      // while we have unscheduled WFs
      while (unscheduled.size() != 0){
         //if (stage % 10 == 0) cout << "Stage " << stage << endl;
         stage++;
         // best schedule (from the prioretization criteria point of view)
         Schedule best;
         // and "best" wf number
         int bestWFNum = -1;
         // max eff (on this iteration)
         double currentBestEff = numeric_limits<double>::infinity();
         // busy intervals for best schedule
         vector<vector <BusyIntervals>> storedIntervals;
         // for each unscheduled WF
         for (auto &wfNum : unscheduled){
            Schedule current;
            unique_ptr <SchedulingMethod> method = 
               SchedulingFactory::GetMethod(data, methodsSet[wfNum], wfNum);
            // get current schedule in current variable
            double currentEff = method->GetWFSchedule(current);
         //	file << "wfnum = " << wfNum << " current eff = " << currentEff << endl;
            if (currentEff < currentBestEff){
               best = current;
               bestWFNum = wfNum;
               currentBestEff = currentEff;
               storedIntervals.clear();
               data.GetCurrentIntervals(storedIntervals);
            }
            data.ResetBusyIntervals();
         }

         // set local to global packages
         int initNum = data.GetInitPackageNumber(bestWFNum);
         for (size_t i = 0; i < best.size(); i++)
            best[i].get<0>() += initNum;
         // add best schedule to full schedule
         copy(best.begin(), best.end(), back_inserter(fullSchedule));

         data.SetCurrentIntervals(storedIntervals);
         data.FixBusyIntervals();

         maxEff += currentBestEff;

         //file << "Best wf num: " << bestWFNum << " bestEff: " << currentBestEff << endl;
         //cout << "Best wf num: " << bestWFNum << " bestEff: " << currentBestEff << endl;
         if (bestWFNum == -1) { 
            cout << "unscheduled.size() == " << unscheduled.size() << endl;
            break;
         }
         auto idx = find(unscheduled.begin(), unscheduled.end(), bestWFNum);
         if (idx == unscheduled.end()) {
            cout << bestWFNum << endl;
            throw UserException("Scheduler::EfficiencyOrdered(int) error. Best wf number was not found");
         }
         unscheduled.erase(idx);
      }
      data.SetInitBusyIntervals();
      maxEff /= maxPossible;
      cout << "Efficiency ordered scheme eff: " << maxEff << endl;
      
      xmlWriter->SetXMLBaseName("Efficiency_");
      // write result to XML
      xmlWriter->CreateXML(fullSchedule);
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// scheduling ordered due to prioretization criteria
void Scheduler::OrderedScheme(int criteriaNumber){
   try{
      maxEff = 0.0;
      // get pointer to criteria 
      unique_ptr<CriteriaMethod> criteria = CriteriaFactory::GetMethod(data,criteriaNumber);
      bool tendsToMin = criteria->TendsToMin();
      // unscheduled WF numbers
      vector <int> unscheduled;
      for (int i = 0; i < data.GetWFCount(); i++)
         unscheduled.push_back(i);
      int stage = 0;
      // while we have unscheduled WFs
      while (unscheduled.size() != 0){
         //if (stage % 10 == 0) cout << "Stage " << stage << endl;
         stage++;
         // best schedule (from the prioretization criteria point of view)
         Schedule best;
         // and "best" wf number
         int bestWFNum = -1;
         // max eff (on this iteration)
         double currentBestEff = 0.0;
         // current best criteria value
         double bestCriteria = tendsToMin ? numeric_limits<double>::max() : 
            -1 * numeric_limits<double>::max();
         // busy intervals for best schedule

         vector<vector <BusyIntervals>> storedIntervals;
         // for each unscheduled WF
         for (auto &wfNum : unscheduled){
            Schedule current;
            unique_ptr <SchedulingMethod> method = 
               SchedulingFactory::GetMethod(data, methodsSet[wfNum], wfNum);
            // get current schedule in current variable
            double currentEff = method->GetWFSchedule(current);
            // get current criteria
            double currentCriteria = criteria->GetCriteria(current, wfNum);
            if (criteria->IsBetter(currentCriteria, bestCriteria)){
               best = current;
               bestWFNum = wfNum;
               currentBestEff = currentEff;
               bestCriteria = currentCriteria;
               storedIntervals.clear();
               data.GetCurrentIntervals(storedIntervals);
            }
            data.ResetBusyIntervals();
         }

         // set local to global packages
         int initNum = data.GetInitPackageNumber(bestWFNum);
         for (size_t i = 0; i < best.size(); i++)
            best[i].get<0>() += initNum;
         // add best schedule to full schedule
         copy(best.begin(), best.end(), back_inserter(fullSchedule));

         data.SetCurrentIntervals(storedIntervals);
         data.FixBusyIntervals();

         maxEff += currentBestEff;

         //cout << "Best wf num: " << bestWFNum << " bestCriteria: " << bestCriteria << endl;
         if (bestWFNum == -1) { 
            cout << "unscheduled.size() == " << unscheduled.size() << endl;
            break;
         }
         auto idx = find(unscheduled.begin(), unscheduled.end(), bestWFNum);
         if (idx == unscheduled.end()) 
            throw UserException("Scheduler::OrderedScheme(int) error. Best wf number was not found");
         unscheduled.erase(idx);
      }
      data.SetInitBusyIntervals();
      maxEff /= maxPossible;
      cout << "Ordered scheme eff: " << maxEff << endl;
      xmlWriter->SetXMLBaseName("Ordered_");
      // write result to XML
      xmlWriter->CreateXML(fullSchedule);
 
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void Scheduler::SimpleSched(){
   // third parameter = -1 means that we will find the schedule for whole big WF
   unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[0], -1);
   // get full schedule
   maxEff = method->GetWFSchedule(fullSchedule);
   maxEff = maxEff/maxPossible;
   cout << "Efficiency: " << maxEff << endl;
   //data.FixBusyIntervals();
   xmlWriter->SetXMLBaseName("Simple_");
   // write result to XML
   xmlWriter->CreateXML(fullSchedule);
   data.SetInitBusyIntervals();
 }

// add to file info about schedule
void Scheduler::PrintOneWFSched(ofstream & res, Schedule & sched, int wfNum){
   res << "WF " << wfNum << endl;
   for (Schedule::iterator it = sched.begin(); it!= sched.end(); it++){
      res << "(" << it->get<0>() << " " << it->get<1>() << " " << it->get<3>() << " ";
      for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
         res << *it2 ;
      res << "))";
   }
   res << endl;
}

// add to res file additional schedule information
void Scheduler::PrintFooter(ofstream & res, vector<double>&eff){
   res << "Workflow order: " ;
      for (vector<int>::size_type i = 0; i < scheduledWFs.size(); i++){
         res << scheduledWFs[i] << " ";
      }
   res << endl << "Efficiencies: " ;
   for (vector<int>::size_type i = 0; i < eff.size(); i++){
      res << eff[i] << " ";
   }
   res << endl << "Max eff: " << maxEff << endl << endl;
}

void Scheduler::GetMetrics(string filename, string name, string metricsFileName){
   Metrics m(data, filename, maxEff);
   m.GetMetrics(fullSchedule, name, metricsFileName);
   ofstream out(filename, ios::app);
   out << "Efficiency: " << maxEff << endl;
}

void Scheduler::TestSchedule(){
   Test t(data, fullSchedule);
   if (t.TestIntervals())
      cout << "Test intervals passed" << endl;
   if (t.TestWFLinks())
      cout << "Test wf links passed" << endl;
}