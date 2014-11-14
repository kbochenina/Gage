#include "StdAfx.h"
#include "PackageStates.h"
#include "UserException.h"
#include <iostream>

using namespace std;

// move constuctor
PackageStates::PackageStates(PackageStates&& rhs){
	//std::cout << "PackageState move constructor\n";
	this->pNum = rhs.pNum;
	this->fullStates = std::move(rhs.fullStates);
}

int PackageStates::GetNextStateNum(int currentState, pair<int,int> typeCore){
	try{
		string errCurrentStateNum = "PackageStates()::GetNextStateNum() error. Wrong current state num";
		if (currentState < 0 || currentState > fullStates.size()-1) 
			throw UserException(errCurrentStateNum); 
		tuples::tuple <int,int, double> fullState = fullStates[currentState];
		int currentType = fullState.get<0>();
		int currentCores = fullState.get<1>();
		if (currentType == -1) return currentState;
		if (currentType != 0) {
			// if we have next non-1 level for this (type, core)
			// then if we have next uncomplete level for this (type, core)
			// we should use it
			// otherwise task will be finished on next step
			// and it is last package state (by default) 
			if (fullStates[currentState+1].get<0>() == currentType && 
				fullStates[currentState+1].get<1>() == currentCores)
			return currentState + 1;
			else return fullStates.size()-1;
		}
		else {
			for (unsigned int i = currentState + 1; i < fullStates.size(); i++){
				if (typeCore.first == fullStates[i].get<0>() && typeCore.second == fullStates[i].get<1>())
					return i;
			}
			// case when execTime(type, core) > delta
		return fullStates.size()-1;
		//throw UserException("Package::GetNextStateNum() error. Current type or core cannot be found in PackageState");
		}
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

PackageStates::~PackageStates(void)
{
}

void PackageStates::PrintStates(ofstream & out){
	out << "Package # " << pNum << endl;
	int stateNum = 0;
	for (auto state : fullStates){
		out << "State # " << stateNum++ << "\t";
		out << state.get<0>() << " " << state.get<1>() << " " << state.get<2>() << endl;
	}
	out << endl;
}


