#include "StdAfx.h"
#include "Intervals.h"

void Intervals::SetData(vector<BusyIntervals> i, ModelingContext& c){ 
   init = i; 
   context = c;
   if (init.size() > 0) 
      // numCoresPerOneRes * numResources
      numCores = init[0].size() * init.size(); 
   else numCores = 0; 
   SetInit();
 }


// find placement !for 1 processor for execTime
// tbegin is in/out parameter (in - earliest start time of package)
// processor is out parameters
// we can use non-full execution times in that case
// function return false and (-1, -1) as out parameters 
// if we have not time at all processors
bool Intervals::FindPlacement(const double &execTime, int &tbegin, int& processor, double&deadline) const{
   vector <double> tb(numCores, -1);
   int currentProcessor = 0;
   // cycle for resources
   for (size_t i = 0; i < current.size(); i++){
      const BusyIntervals & bi = current[i];
      // cycle for processors
      for (BusyIntervals::const_iterator j = bi.begin(); j != bi.end(); j++){
         double begin = tbegin;
         // if we have no busy intervals for this processor
         // we can start execution from tbegin
         if (j->second.size()==0){
            tb[currentProcessor++] = tbegin;
            continue;
         }
         //// cycle for intervals
         //for (size_t k = 0; k < j->second.size(); k++){
         //   if (j->second[k].second <= begin && k != j->second.size()-1 )
         //      continue;
         //   if (begin + execTime <= j->second[k].first){
         //      tb[currentProcessor] = begin;
         //      break;
         //   }
         //   else
         //      begin = j->second[k].second;
         //   if ( k == j->second.size()-1 )
         //      tb[currentProcessor] = begin;

         //}

		 int intIndex = 0;
		 int numIntervals = j->second.size();
		 while (intIndex < numIntervals 
			 && begin >= j->second[intIndex].second) ++intIndex;
		 // if begin > right busy border, begin = right border
		 if (intIndex == j->second.size()){
			 tb[currentProcessor] = j->second[numIntervals-1].second;
		 }
		 else {
			  while (true){
				 // if package can be placed into [intIndex-1; intIndex] interval
				 if (begin + execTime <= j->second[intIndex].first){
					 tb[currentProcessor] = begin;
					 break;
				 }
				 // if package cannot be placed between last busy interval and previous,
				 // right border is tbegin
				 if (intIndex == j->second.size()-1){
					 tb[currentProcessor] = j->second[intIndex].second;
					 break;
				 }
				// else we try next free diapason
				begin = j->second[intIndex].second;
				++intIndex;
			 }
		 }

         currentProcessor++;
      }
   }

   for (auto& elem : tb){
	   if (elem < tbegin)
		   elem = tbegin;
   }

   double trealbegin = deadline;
   // find resource with earliest finishing time
   for (size_t i = 0; i < tb.size(); i++){
      // if we haven't free window on this processor
      // we should miss it
      if (tb[i] == -1)
         continue;
      // if starting time is less than previous
      // and more than earliest finishing time
      if (tb[i] < trealbegin && tb[i]>=tbegin){
         trealbegin = tb[i];
         processor = i;
      }
   }
   tbegin = static_cast<int>(trealbegin);
   if (tbegin + 1 > deadline){
      processor = -1;
      tbegin = -1;
      return false;
   }
   return true;
}

// add busy intervals [tBegin; (tBegin + execTime) round to highest stage border] to cores in coreNumbers
void Intervals::AddDiaps(vector <int> coreNumbers, int tBegin, double execTime)  {
   for (size_t i = 0; i < coreNumbers.size(); i++){
      int processor = coreNumbers[i];
      int number = 0;
      for (size_t i = 0; i < current.size(); i++){
         for (BusyIntervals::iterator j = current[i].begin(); j != current[i].end(); j++){
            if (number != processor){
               number++;
               continue;
            }
            else {
               if (j->second.size() == 0 || tBegin < j->second[0].first){
                  pair<int,int> newPair = make_pair(tBegin, static_cast<int>(tBegin + execTime + 1));
                  j->second.insert(j->second.begin(), newPair);
                  return;
               }

               // cycle for intervals
               for (size_t k = 0; k < j->second.size(); k++){
                  // if tbegin > last tend
                  if (tBegin >= j->second[k].second )
                     // and this is last interval
                     // or we can insert this interval among two intervals
                     if (k == j->second.size() - 1 || tBegin + execTime <= j->second[k+1].second){
                     pair<int,int> newPair = make_pair(tBegin, static_cast<int>(tBegin + execTime + 1));
                     j->second.insert(j->second.begin() + k + 1, newPair);
                     return;
                  }
               }
            }
         }
      }
   }
}

// check received interval for intersection with existing intervals
bool Intervals::CanPlace(const int& resNum, const int &procNum, const int& tBegin, const double& execTime){
   
   const auto& intervals = current[resNum][procNum];
   
   for (const auto& interval : intervals){
      if (tBegin >= interval.first && tBegin < interval.second)
         return false;
      const double tEnd = tBegin + execTime;
      if (tEnd > interval.first && tEnd <= interval.second)
         return false;
   }
   return true;
}

Intervals::~Intervals(void)
{
}
