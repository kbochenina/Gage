#pragma once
class Efficiency
{
	double koeff;
	double T;
public:
	Efficiency(){}
	Efficiency(double k, double period): koeff(k), T(period){}
	inline double GetKoeff() {return koeff; }
	// return value of efficiency function in a point
	double EfficiencyFunction(double x);
	// return value of efficiency by period
	double EfficiencyByPeriod(int busyCores, double tBegin, double tEnd);
	~Efficiency(void);
};

