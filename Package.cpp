#include "StdAfx.h"
#include "Package.h"


double Package::GetExecTime(int type, int cores) const {
	try{
		std::pair<int,int> typeCore = make_pair(type,cores);
		auto it = execTimes.find(typeCore);
		if (it==execTimes.end()) 
			throw UserException("Package::GetExecTime() : combination of type " + to_string(type) + 
			"  and cores " + to_string(cores) + " not found");
		return it->second;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

double Package::GetAvgExecTime() const{
	double sum = 0.0;
	int counter = 0;
	for (auto it = execTimes.begin(); it!= execTimes.end(); it++){
		// for 1 core
		if (it->first.second == 1){
			sum += it->second;
			counter ++;
		}
	}
	return sum/counter;
}

// return maximum exectime of package pNum (on 1 processor)
double Package::GetMaxExecTime() const{
	double max = 0.0;
	for (auto it = execTimes.begin(); it!= execTimes.end(); it++){
		// for 1 core
		if (it->first.second == 1){
			if (it->second > max)
				max = it->second;
		}
	}
	return max;
}

// copy constructor
Package::Package (const Package & p){
	uid = p.uid;
	resTypes.reserve(p.resTypes.size());
	processorsCounts.reserve(p.resTypes.size());
	copy(p.resTypes.begin(), p.resTypes.end(), back_inserter(resTypes));
	copy(p.processorsCounts.begin(), p.processorsCounts.end(), back_inserter(processorsCounts));
	execTimes.insert(p.execTimes.begin(), p.execTimes.end());
	amount = p.amount;

}

Package::~Package(void)
{
}
