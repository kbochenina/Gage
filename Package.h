#include <map>

typedef map<pair<int,int>, double> times;

#pragma once
class Package
{
	// package local uid - from 1
	int uid;
	// supported resource types
	vector <int> resTypes;
	// supported core counts
	vector <int> processorsCounts;
	// exec times for pairs (resType, coreCount)
	times execTimes;
	// amount
	double amount;
   // alpha - the part of consequtive code in Amdal's law
   double alpha;
public:
	Package(int u, vector<int>r, vector<int>c, times e, double a, double al) {uid = u; resTypes = r; processorsCounts = c; execTimes = e; amount = a; alpha = al;}
	// copy constructor
	Package (const Package & p);
	// getting execTime for choosed type and core
	double GetExecTime(int type, int cores) const ;
	int GetResTypesCount () const {return resTypes.size();}
	int GetProcessorsCount () const {return processorsCounts.size();}
	const vector <int>& GetResTypes() const {return resTypes;}
	const vector <int>& GetprocessorsCounts() const {return processorsCounts;}
	const times& GetExecTimes() const {return execTimes;}
	const double GetAmount() const { return amount;} 
	// return average exectime of package pNum (on 1 processor)
	double GetAvgExecTime() const;
	// return maximum exectime of package pNum (on 1 processor)
	double GetMaxExecTime() const;
	~Package(void);
};

