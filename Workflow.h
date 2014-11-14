#include "Package.h"

#pragma once
class Workflow
{
	// workflow number - from 1
	int uid;
	// start time
	int tstart;
   // deadline
   double deadline;
   // packages of WF
	vector <Package> packages;
	// connectivity matrix
	vector<vector<int>> matrix;
   // sub-deadlines
   vector <double> finishingTimes;
   // expected start times
   vector <double> startTimes;
   // list of packages priority
   vector <int> priorities;
   // data sizes 
   vector<vector<double>> transfer;
   // set start times after setting finishing times
   void SetStartTimes();
public:
	const Package& operator[] (int pNum) const { return packages[pNum]; } 
	// return package count
	inline int GetPackageCount() const { return packages.size(); } 
	// return execTime of package pNum on resource with type resType and coreCount
	double GetExecTime (unsigned int pNum, int resType, int coreCount) const ;
	// return vector with possible cores count for package pNum
	void GetProcessorsCount(int pNum, vector<int>&out) ;
	// return vector with possible resource types for package pNum
	void GetResTypes(int pNum, vector<int>&out) const;
	// return true if package pNum is init
	bool IsPackageInit(int pNum) const;
	// return true if first depends on second
	bool IsDepends(unsigned one, unsigned two) const;
	// return matrix[i][j]
	int GetMatrixValue(int i, int j) const { return matrix[i][j]; }
	// return vector with packages that depend from pNum
	void GetOutput(unsigned int pNum, vector<int>& out) const;
	// return vector with packages from which pNum depends 
	void GetInput(unsigned int pNum, vector<int>& in) const;
	// return true if the package pNum is last
	bool IsPackageLast(int pNum) const;
	// return average exectime of package pNume
	double GetAvgExecTime(int pNum) const;
	// return computational amount of package pNum
	double GetAmount(const int & pNum) const { return packages[pNum].GetAmount(); }
	// return all successors of package pNum
	void GetSuccessors(const unsigned int &pNum, vector<int>&out) const;
	Workflow(int u, vector <Package> p, vector<vector<int>> m, double d, vector<vector<double>> t, double start);
   // setting sub-deadlines for the workflow
   void SetFinishingTimes(double avgCalcPower);
   size_t GetFinishingTimesSize() {return finishingTimes.size();}
   // !dangerous
   double GetFinishingTime (int pNum) const {return finishingTimes[pNum];}
   // !dangerous
   double GetStartTime (int pNum) const {return startTimes[pNum];}
   void SetPriorities();
   int GetNextPackage (int pNum) const{return priorities[pNum];}
   const double GetDeadline() const {return deadline;}
   double GetTransfer(const int &in, const int &out) const;
   void SetTransfer(vector<vector<double>> t)  ;
   int GetLastPackagesCount() const;
   bool IsParent(int parent, int child) const { return matrix[parent][child] == 1? true : false;}
   // returns start time of workflow
   double GetStartTime() const {return tstart;}
   // get maximum exec time of package
   double GetMaximumExecTime(const int& pNum) const;
   void SetDeadline(double deadline) {this->deadline = deadline;}
   void SetTStart(double tstart){this->tstart = tstart;}
    Workflow(){ uid = 0; }
	~Workflow(void);
};

