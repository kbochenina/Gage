#include "stdafx.h"
#include "CriteriaFactory.h"
#include "ReservedTimeCriteria.h"

CriteriaFactory::CriteriaFactory(void)
{
}


CriteriaFactory::~CriteriaFactory(void)
{
}

unique_ptr<CriteriaMethod> CriteriaFactory::GetMethod(DataInfo &d, int criteriaNum){
	try{
		switch (criteriaNum){
			case RESERVED_TIME:
			return unique_ptr<CriteriaMethod>(new ReservedTimeCriteria(d));
		default:
			throw UserException("CriteriaFactory::GetMethod() error. No valid algorithm found. Current criteria uid = " + to_string(criteriaNum));
		}
	}
	catch (UserException& e){
		cout << "error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}