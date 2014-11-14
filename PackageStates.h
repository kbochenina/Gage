#include "boost/tuple/tuple.hpp"
#include <vector>
#include <fstream>

using namespace boost;
using namespace std;

typedef tuples::tuple<int,int,double> PackageState; // (type, core, level)

#pragma once
class PackageStates
{
	// package number
	int pNum;
	vector <PackageState> fullStates;
public:
	PackageStates(int p, vector <PackageState> fS) : pNum(p), fullStates(fS) { };
	PackageStates(PackageStates&& rhs);
	const int& GetType(int stateNum) {return fullStates[stateNum].get<0>();}
	const int& GetCore(int stateNum) {return fullStates[stateNum].get<1>();}
	const double& GetLevel(int stateNum) {return fullStates[stateNum].get<2>();}
	const int GetStatesCount() const {return fullStates.size();}
	const PackageState& operator[] (int index) const { return fullStates[index]; }
	int GetNextStateNum(int currentState, pair<int,int> typeCore);
	void PrintStates(ofstream & out);
	~PackageStates(void);
};

