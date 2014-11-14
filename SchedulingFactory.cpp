#include "stdafx.h"
#include "SchedulingFactory.h"

enum Algorithms{ BELLMAN = 1, GREEDY = 2, CLUSTERED = 3, GAGE = 4 };

SchedulingFactory::SchedulingFactory(void)
{
}

SchedulingFactory::~SchedulingFactory(void)
{
}

unique_ptr<SchedulingMethod> SchedulingFactory::GetMethod(DataInfo &d, int uid, int wfNum){
	try{
		switch (uid){
		case BELLMAN:
			//return unique_ptr<SchedulingMethod> (new BellmanScheme(d, uid, wfNum));
		case GREEDY:
			return unique_ptr<SchedulingMethod>(new Greedy(d, uid, wfNum));
		case CLUSTERED:
			return unique_ptr<SchedulingMethod>(new Clustered(d,uid));
      case GAGE:
          return unique_ptr<SchedulingMethod>(new Gage(d,wfNum));
		default:
			throw UserException("SchedulingFactory::GetMethod() error. No valid algorithm found. Current algorithm uid = " + to_string(uid));
		}
	}
	catch (UserException& e){
		cout << "error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

