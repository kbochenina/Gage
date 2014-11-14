#include "StdAfx.h"
#include "ModelingContext.h"


ModelingContext::ModelingContext(void)
{
	T = 0; 
   CCR = 0.0;
   h = 0.0;
}



ModelingContext::~ModelingContext(void)
{
}

void ModelingContext::SetContext( int T,  double CCR, double h, double minLength )
{
	this->T = T;
	this->CCR = CCR;
    this->h = h;
	this->minLength = minLength;
}
