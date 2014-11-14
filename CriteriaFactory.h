#include "stdafx.h"
#include "DataInfo.h"
#include "CriteriaMethod.h"
#include <memory>

#pragma once
class CriteriaFactory
{
public:
	static unique_ptr<CriteriaMethod> GetMethod(DataInfo &d, int criteriaNum);
	CriteriaFactory(void);
	~CriteriaFactory(void);
};

