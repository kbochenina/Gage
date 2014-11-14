#include "DataInfo.h"

#pragma once
class ScheduleToXML
{
	// model data
	DataInfo& data;
	string resFileName;
   string schedFileName;
	string xmlBaseName;
	// current xml file count
	static int xmlCount;
	void MetaXMLInfo(ofstream &f);
	void BusyToXML(ofstream &f);
	void FullScheduleToXML(ofstream&f, Schedule &currentSchedule);
   void PrintSched(Schedule &sched);
public:
	ScheduleToXML(DataInfo &d): data(d) {}
	// create an XML file based on current data state (especially fixed intervals)
	// add packages from currentSchedule
	void CreateXML(Schedule currentSchedule);
	void SetXMLBaseName(string name) { xmlBaseName = name; }
	~ScheduleToXML(void);
};

