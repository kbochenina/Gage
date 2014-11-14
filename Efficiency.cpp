#include "StdAfx.h"
#include "Efficiency.h"

// returning value is the efficiency by period
double Efficiency::EfficiencyByPeriod(int busyCores, double t1, double t2){
	if (t1 > t2) 
		throw UserException("Efficiency::EfficiencyByPeriod error. tend < tbegin");
	return ( busyCores * (double)(t2-t1)/(double)T * (EfficiencyFunction((double)t1/(double)T) + EfficiencyFunction((double)t2/(double)T)) / 2  );
}

double Efficiency::EfficiencyFunction(double x) { return (koeff*(1-x)); }
	

Efficiency::~Efficiency(void)
{
}
