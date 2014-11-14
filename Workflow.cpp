#include "StdAfx.h"
#include "Workflow.h"
#include <list>

Workflow::Workflow(int u, vector <Package> p, vector<vector<int>> m, double d, vector<vector<double>> t, double tstart) {
    uid = u; 
    packages = p; 
    matrix = m; 
    deadline = d; 
    transfer = t;
    this->tstart = tstart;
}

void Workflow::SetTransfer(vector<vector<double>> t) {
    try{
        if (t.size() != transfer.size())
            throw UserException("Workflow::SetTransfer() error. Wrong rows count");
        for (const auto&row: t){
            if (row.size() != transfer.size())
                 throw UserException("Workflow::SetTransfer() error. Wrong columns count");
        }
        for (size_t i = 0; i < transfer.size(); i++)
            for (size_t j = 0; j < transfer.size(); j++)
                transfer[i][j] = t[i][j];
    }
    catch (UserException& e){
        std::cout<<"error : " << e.what() <<endl;
        std::system("pause");
        exit(EXIT_FAILURE);
    }
}

int Workflow::GetLastPackagesCount() const{
    int val = 0;
    for (int i = 0; i < packages.size(); i++)
        if (IsPackageLast(i)) val++;
    return val;
}

double Workflow::GetExecTime ( unsigned int pNum, int type, int cores) const {
   try{
      if (pNum < 0 || pNum > packages.size()-1) 
         throw UserException("Workflow::GetExecTime() error. Wrong packageNum" + to_string(pNum));
      return packages[pNum].GetExecTime(type, cores);
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// return true if package pNum is init
bool Workflow::IsPackageInit(int pNum) const {
   for (const auto &i : matrix)
      if (i[pNum]!=0) return false;
   return true;
}

// return true if second depends on first
bool Workflow::IsDepends(unsigned one, unsigned two) const {
   try {
      string errorMsg = "Workflow::IsDepends() error. Workflow " + to_string(uid) + ", incorrect package num - ";
      if (one > packages.size()-1) throw errorMsg +  to_string(one);
      if (two > packages.size()-1) throw errorMsg +  to_string(two);
      if (matrix[one][two]==1) return true;
      // if dependency is indirect
      else {
         for (unsigned int i = 0; i < matrix[one].size(); i++){
            if (matrix[one][i]!=0) 
               if (IsDepends(i,two)) return true;
         }
         return false;
      }
   }
   catch(const string msg){
      cout << msg << endl;
      system("pause");
      exit(EXIT_FAILURE);
   }
}

Workflow::~Workflow(void)
{
}

// return vector with packages that depend from pNum
void Workflow::GetOutput(unsigned int pNum, vector<int>& out) const{
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetOutput() error. Wrong package number");
      for (size_t i = 0; i < matrix.size(); i++){
         if (matrix[pNum][i] == 1)
            out.push_back(i);
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}
// return vector with packages from which pNum depends 
void Workflow::GetInput(unsigned int pNum, vector<int>& in) const{
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetInput() error. Wrong package number");
      for (size_t i = 0; i < matrix.size(); i++){
         if (matrix[i][pNum]==1)
            in.push_back(i);
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// return true if the package pNum is last
bool Workflow::IsPackageLast(int pNum) const {
   bool isLast = true;
   for (size_t i = 0; i < matrix.size(); i++){
      if (matrix[pNum][i]==1){
         isLast = false;
         return isLast;
      }
   }
   return isLast;
}

// return average exectime of package pNume
double Workflow::GetAvgExecTime(int pNum) const{
   return packages[pNum].GetAvgExecTime();
}

double Workflow::GetMaximumExecTime(const int& pNum) const{
	return packages[pNum].GetMaxExecTime();
}

// return all successors of package pNum
void Workflow::GetSuccessors(const unsigned int &pNum, vector<int>&out) const {
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetInput() error. Wrong package number");
      out.clear();
      vector <int> successors;
      GetOutput(pNum, successors);
      out = successors;
      while (successors.size() != 0){
         vector<int>tmp = successors;
         for (auto &current: tmp){
            vector<int> currentSuccessors;
            GetOutput(current, currentSuccessors);
            for (const auto& i : currentSuccessors){
               if (find(out.begin(), out.end(), i) == out.end()){
                  out.push_back(i);
                  successors.push_back(i);
               }
            }
            successors.erase(find(successors.begin(), successors.end(), current));
         }
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}


// !!make up variant with communications
 // setting sub-deadlines for the workflow
 void Workflow::SetFinishingTimes(double avgCalcPower){
     // indexes of packages which finishing time was already setted up 
      vector <int> usedNums;
      double maxAmount = 0.0;
      int maxTask = -1;
      vector<double> amounts;
      int pCount = packages.size();
      amounts.resize(pCount);
      for (int i = 0; i < pCount; i++){
         if (IsPackageInit(i)){
            amounts[i] = packages[i].GetAmount();
            if (maxAmount < amounts[i]) {
               maxAmount = amounts[i];
               maxTask = i;
            }
            usedNums.push_back(i);
         }
      }
      // linked - packages linked with already calculated
      vector<int> linked;
      vector<int> out;
      for (size_t i = 0; i < usedNums.size(); i++){
         GetOutput(usedNums[i], out);
         for (size_t i = 0; i < out.size(); i++){
            if (find(linked.begin(), linked.end(), out[i]) == linked.end())
               linked.push_back(out[i]);
         }
      }
      // while we don't find all values
      while (linked.size()!=0){
         vector <int> in;
         // get first value from linked
         int current = linked.front();
         // get all inputs for linked
         GetInput(current, in);
         bool allUsed = true;
         for (size_t k = 0; k < in.size(); k++){
            if (find(usedNums.begin(),usedNums.end(),in[k]) == usedNums.end())
               allUsed = false;
         }
         // if all previous values were already calculated
         if (allUsed){
            double max = 0;
            for (size_t k = 0; k < in.size(); k++){
               if (amounts[in[k]] > max) max = amounts[in[k]];
            }
            // get latest "time" according to maximum of previous
            amounts[current] = max + packages[current].GetAmount();
            if (maxAmount < amounts[current]){
               maxAmount = amounts[current];
               maxTask = current;
            }
            // push current package to usedNums
            usedNums.push_back(current);
            out.clear();
            // add all outputs of current package to linked
            GetOutput(current, out);
            // erase current from linked
            linked.erase(linked.begin());
            for (size_t i = 0; i < out.size(); i++){
               if (find(linked.begin(), linked.end(), out[i]) == linked.end())
                  linked.push_back(out[i]);
            }
         }
         // if we can't calculate value for current on this step
         else {
            // we move current to the end
            linked.erase(linked.begin());
            linked.push_back(current);
         }
      }
      
      // calculating last finishing times for all tasks
            
      finishingTimes.resize(pCount);
           
      //finishingTimes[maxTask] = maxAmount/avgCalcPower;
      //double deadline = finishingTimes[maxTask];
	  finishingTimes[maxTask] = deadline + tstart;
      for (int i = 0; i < pCount; i++){
         if (i != maxTask){
            finishingTimes[i] = amounts[i]/maxAmount*deadline + tstart;
			if (finishingTimes[i] < 0)
				cout << "error" << endl;
         }
      }
	SetStartTimes();
 }

 void Workflow::SetStartTimes(){
	 vector<int> pred;
	 startTimes.resize(packages.size(),tstart);
	 for (size_t i = 0; i < packages.size(); i++){
		 GetInput(i,pred);
		 if (pred.size() == 0) {
			 startTimes[i] = GetStartTime();
			 continue;
		 }
		 double maxFinishingPred = 0, currDeadline = 0;
		 for (vector<int>::iterator it = pred.begin(); it!= pred.end(); it++){
			 currDeadline = finishingTimes[*it];
			 if (currDeadline > maxFinishingPred)
				 maxFinishingPred = currDeadline; 
		 }
		 startTimes[i] = maxFinishingPred;
		// if (startTimes[i]> finishingTimes[i])
		//	 cout << startTimes[i] << " " << finishingTimes[i] << endl;
		 pred.clear();
	 }
	// cout << endl;
 }

 // set different wf priorities
void Workflow::SetPriorities(){
      // get priority list for current wf
      list <pair<int, double>> priorityList;
      for (size_t j = 0; j < finishingTimes.size(); j++)
         priorityList.insert(priorityList.end(),make_pair(j, finishingTimes[j]));
      double minTime = std::numeric_limits<double>::infinity();
      int minIndex = 0;

      while (priorities.size() < packages.size()){
         list<pair<int, double>>::iterator position, dependPosition = priorityList.begin();
         for (list<pair<int, double>>::iterator it = priorityList.begin();
            it!= priorityList.end(); it++){
            if (it->second < minTime){
                  minTime = it->second;
                  minIndex = it->first;
                  position = it;
            }
         }
         vector<int> dependsOn;
         GetInput(minIndex, dependsOn);
         bool allPrioretized = true;
         for (size_t i = 0; i < dependsOn.size(); i++){
            if (find(priorities.begin(), priorities.end(), dependsOn[i]) == priorities.end()){
               allPrioretized = false;
               // find last depend position
               for (size_t i = 0; i < dependsOn.size(); i++){
                  auto foundPosition = 
                     find(priorityList.begin(), priorityList.end(),
                     make_pair(dependsOn[i], finishingTimes[dependsOn[i]]));

                  if (distance(foundPosition, dependPosition))
                     dependPosition = foundPosition;
               }
               priorityList.erase(position);
               priorityList.insert(dependPosition, make_pair(minIndex,minTime));
               break;
            }
         }

         if (allPrioretized) {
            priorities.push_back(minIndex);
            priorityList.erase(position);
         }
      
      
         minTime = std::numeric_limits<double>::infinity();
      }
      // smallest elements will be in the end of vector
      reverse(priorities.begin(), priorities.end());
    
 }

double Workflow::GetTransfer(const int &in, const int &out) const {
    try{
        if (in > transfer.size()-1 || in < 0 || out > transfer.size()-1 || out < 0)
            throw UserException("Workflow::GetTransfer() error. Wrong parameters");
        return transfer[in][out];
    }
    catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}