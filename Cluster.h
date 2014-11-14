#pragma once
typedef boost::tuple<vector<int>, vector<double>, vector<double>,vector<double>> clusterInfo;

class Cluster
{
	vector<int> pNumbers;
	vector <double> pstarts;
	vector <double> pdeadlines;
	vector <double> pweights;
	vector <double> plengths;
	double weight;
	double length;
	double deadline;
	double tstart;
public:
	Cluster(void) {weight = 0; length = 0; deadline = 0; tstart = 0;}
	void Add(int pNum, double pWeight, double pDeadline, double pStart);
	double GetLength();
	void SetLength();
	double GetWeight() {return weight;}
	double GetDeadline() {return deadline;}
	double GetStart() {return tstart;}
	void GetInfo(clusterInfo &info);
	void PrintInfo(ofstream& file);
	int GetSize() {return pNumbers.size();}
	// !dangerous
	int GetPNum(int index){return pNumbers[index];};
	int GetWeight(int index){return pweights[index];}
	int GetDeadline(int index){return pdeadlines[index];}
	int GetPLength(int index){return plengths[index];}
	int GetPStart(int index){return pstarts[index];}
	void Change (int index1, int index2);
	void InsertAfter(int toInsert, int after);
	void SetStart(double i) {tstart = i;}
	void SetDeadline(double i){deadline = i;}
	void Delete(int index);
	~Cluster(void);
};

