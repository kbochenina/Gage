#include "StdAfx.h"
#include "ScheduleToXML.h"
#include <memory>

int ScheduleToXML::xmlCount = 0;

struct my_compare_op
{
    bool operator()(const PackageSchedule& a, const PackageSchedule& b) const
    {
        return boost::get<0>(a) < boost::get<0>(b);
    }
};

void ScheduleToXML::CreateXML(Schedule currentSchedule){
	//std::cout << "ScheduleToXML::CreateXML() was called\n";
	resFileName = xmlBaseName + to_string(xmlCount) + ".jed";
   schedFileName = xmlBaseName + to_string(xmlCount) + ".txt";
   ++xmlCount;
	ofstream f(resFileName);
	MetaXMLInfo(f);
	f << "\t<node_infos>\n";
	BusyToXML(f);
   sort(currentSchedule.begin(), currentSchedule.end(), my_compare_op());
  	FullScheduleToXML(f, currentSchedule);
   PrintSched(currentSchedule);
	f << "\t</node_infos>\n";
	f << "</grid_schedule>\n";
	f.close();
}

void ScheduleToXML::PrintSched(Schedule & sched){
   int wfNum = -1, currentWf = -1;
   ofstream s(schedFileName);
   // res << "WF " << wfNum << endl;
   for (Schedule::iterator it = sched.begin(); it!= sched.end(); it++){
        int globalPackageNumber = it->get<0>();
        int localNum = -1;
        data.GetLocalNumbers(globalPackageNumber, currentWf, localNum);
        if (currentWf != wfNum){
            s << "----------------------------------------------------------------------------------" << endl << "Workflow " << currentWf + 1 << endl;
            wfNum = currentWf;
        }
        s << "P" << ++localNum << "(" << globalPackageNumber + 1 << ") "<< " Processor numbers: ";
        for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
            s << *it2  << " ";
        s << "Res type: " << data.GetResourceType(it->get<2>()[0]);
        s << " Start time: " << it->get<1>() << " Exec time: " << it->get<3>() << " End time: " <<
            it->get<1>() + it->get<3>() << endl;
   }
   s << endl;
   s.close();
}

void ScheduleToXML::MetaXMLInfo(ofstream &f){
	f << "<grid_schedule>\n";
	f << "\t<meta_info>\n";
	f << "\t\t<meta name=\"alloc\" value=\"mcpa\"/>\n";
	f << "\t\t<meta name=\"pack\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"bf\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"ialloc\" value=\"0\"/>\n";
	f << "\t</meta_info>\n";
	f << "\t<grid_info>\n";
	f << "\t\t<info name=\"nb_clusters\" value=\"1\"/>\n";
	f << "\t\t<clusters>\n";
	f << "\t\t  <cluster id=\"0\" hosts=\"" << data.GetprocessorsCount() << "\" first_host=\"0\"/>\n";
	f << "\t\t</clusters>\n";
	f << "\t</grid_info>\n";
}

// busy intervals are produced from Intervals.fixed
void ScheduleToXML::BusyToXML(ofstream &f){
	int inc = 0;
	// for each resource type
	for (unsigned i = 0; i < data.resources.size(); i++){
		Intervals windows;
		// get intervals of this resource type
		data.resources[i].GetCurrentWindows(windows);
		// get fixed intervals 
		vector <BusyIntervals> fixed = windows.GetFixedIntervals();
		// for each resource
		for (unsigned j = 0; j < fixed.size(); j++){
			// get pointer to resource's intervals
			BusyIntervals::iterator bIt = fixed[j].begin();
			// loop on different cores
			for (;bIt != fixed[j].end(); bIt++){
				int coreNum = bIt->first-1;
				coreNum += inc;
				// loop on different intervals
				for (vector<pair<int,int>>::size_type k = 0; k < bIt->second.size(); k++){
					int tBegin = bIt->second[k].first;
					int tEnd = bIt->second[k].second;
					f << "\t\t<node_statistics>" << endl;
					f << "\t\t	<node_property name=\"id\" value=\""<< coreNum <<"\"/>" << endl;
					f << "\t\t	<node_property name=\"type\" value=\"busy\"/>" << endl;
					f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
					f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
					f << "\t\t	<configuration>" << endl;
					f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
					f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
					f << "\t\t	  <host_lists>" << endl;
					f << "\t\t	    <hosts start=\"" << coreNum << "\" nb=\"1\"/>" << endl;
					f << "\t\t	  </host_lists>" << endl;
					f << "\t\t	</configuration>" << endl;
					f << "\t\t</node_statistics>" << endl;
				}
			}
			inc += fixed[j].size();
			
		}
	}
}

void ScheduleToXML::FullScheduleToXML(ofstream&f, Schedule &currentSchedule){
	int currentWfPackage = 0;
   int currentWfNum = 0;
	for (Schedule::size_type i = 0; i < currentSchedule.size(); i++){
		int packageNum = currentSchedule[i].get<0>();
		int tBegin = currentSchedule[i].get<1>();
		int coresCount = currentSchedule[i].get<2>().size();
		vector <int> cores = currentSchedule[i].get<2>();
		int type = data.GetResourceType(cores[0]);
		int currBeginIndex = 0;
		// correct
		int localNum = 0;
		data.GetLocalNumbers(packageNum, currentWfNum, localNum);
		//cout << "type=" << type << " cores = " << coresCount << " package = " << packageNum << " ";
		double execTime = currentSchedule[i].get<3>();
		//cout << execTime << endl;
		int tEnd = tBegin + static_cast<int>(execTime);
		for (unsigned j = 0; j < cores.size(); j++){
			f << "\t\t<node_statistics>" << endl;
			f << "\t\t	<node_property name=\"id\" value=\"" << packageNum+1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"type\" value=\"wf"<< currentWfNum % 10 + 1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
			f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
			f << "\t\t	<configuration>" << endl;
			f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
			f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
			f << "\t\t	  <host_lists>" << endl;
			f << "\t\t	    <hosts start=\"" << cores[j] << "\" nb=\"1\"/>" << endl;
			f << "\t\t	  </host_lists>" << endl;
			f << "\t\t	</configuration>" << endl;
			f << "\t\t</node_statistics>" << endl;
		}
		currentWfPackage++;
	}
}

ScheduleToXML::~ScheduleToXML(void)
{
}
