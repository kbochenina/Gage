#include "stdafx.h"
#include "Test.h"

using namespace std;

// (globalPackageNum, tBegin, vector<globalCoreNums>, execTime)
//typedef vector <tuples::tuple<int,int,vector<int>,double>> Schedule;

bool Test::TestIntervals(){
	try{
	bool passed = true;
	// for each package
	for (auto & packageSched : sched){
		auto & globalCoreNums = packageSched.get<2>();
		if (globalCoreNums.size() < 1)
			throw UserException("Test::TestIntervals() error. Zero processor count");
		// get resource type number
		int resType = data.GetResourceTypeIndex(globalCoreNums[0]);
		// get local processor numbers
		vector <int> localCoreNums;
		int initIndex = data.GetInitResourceTypeIndex(resType);
		for (auto & num : globalCoreNums)
			localCoreNums.push_back(num - initIndex);
		// if interval is not busy 
		const int &tBegin = packageSched.get<1>();
		const int globalPackageNum = packageSched.get_head();
		int wfNum, localPackageNum;
		data.GetLocalNumbers(globalPackageNum, wfNum, localPackageNum);
		int wfBegin = data.Workflows(wfNum).GetStartTime();
		if (tBegin < wfBegin){
			cout << "Test Test::TestIntervals() is not passed. Package start time is less than wf start time. Global package number = " <<
				globalPackageNum << ", resource type = " << resType << ", processors ";
			for (const auto & num : localCoreNums)
				cout << num << " ";
			cout << "tBegin = " << tBegin << ", wfBegin = " << wfBegin << endl;
			return false;
		}

		const double &execTime = packageSched.get<3>();
		for (const auto & num : localCoreNums){
			// if interval isn't intersect existing intervals
			if (data.Resources(resType).CanPlace(num, tBegin, execTime))
				// add interval
				data.Resources(resType).AddInterval(execTime, tBegin, num);
			// else return false
			else {
				cout << "Test Test::TestIntervals() is not passed. Global package number = " <<
					globalPackageNum << ", resource type = " << resType << ", processors ";
				for (const auto & num : localCoreNums)
					cout << num << " ";
				cout << "tBegin = " << tBegin << ", execTime = " << execTime << endl;
				return false;
			}
		}
		
	}
	return passed;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

bool Test::TestWFLinks(){
	bool passed = true;
	// for each package
	for (auto & packageSched : sched){
		int globalPackageNum = packageSched.get_head();
		int localPackageNum = 0, wfNum = 0;
		// get wf number and local package number by global package number
		data.GetLocalNumbers(globalPackageNum, wfNum, localPackageNum);
		// get input package numbers
		vector <int> in;
		data.Workflows(wfNum).GetInput(localPackageNum, in);
		// if package is init, continue
		if (in.size() == 0)
			continue;
		// set local numbers to global
		int initPackageNum = data.GetInitPackageNumber(wfNum);
		for (auto &i : in)
			i += initPackageNum;
		// if package has at least one parent
		for (const auto& parent : in){
			bool parentFound = false;
			for (const auto& parentSched : sched){
				// if we find a parent's schedule
				if (parentSched.get_head() == parent){
					parentFound = true;
               double transfer = data.Workflows(wfNum).GetTransfer(localPackageNum, parent - initPackageNum);
               int packageResType = data.GetResourceTypeIndex(packageSched.get<2>()[0]);
               int parentResType = data.GetResourceTypeIndex(parentSched.get<2>()[0]);
               double transferTime = 0.0;
               // if package and its parent calculate on different resources types
               if (packageResType != parentResType){
                   transferTime = transfer / data.GetBandwidth(parentResType, packageResType);
               }
					if (parentSched.get<1>() + parentSched.get<3>() + transferTime > packageSched.get<1>()){
						cout << "Test Test::TestWFLinks() is not passed. Global package number = " <<
						packageSched.get_head() << ", wfNum = " << wfNum  << "localNum = " << localPackageNum << 
						", intersects with parent " << parent << "(global num)" << endl;
						return false;
					}
				}
			}
			if (parentFound == false){
				cout << "Test Test::TestWFLinks() is not passed. Global package number = " <<
				packageSched.get_head() << ", wfNum = " << wfNum  << "localNum = " << localPackageNum << 
				", cannot find schedule for parent " << parent << "(global num)" << endl;
				return false;

			}
		}
	}
	return passed;
}


