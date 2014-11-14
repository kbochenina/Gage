#include "stdafx.h"
#include "Cluster.h"

void Cluster::PrintInfo(ofstream& file){
	file << "Total weight: " << weight << endl;
	file << "Total start: " << tstart << endl;
	file << "Total deadline: " << deadline << endl;
	file << "Total length: " << length << endl;
	file << "Packages " << endl;
	for (size_t i = 0; i < pNumbers.size(); i++){
		file << "(" << pNumbers[i] << " " << pweights[i] << " "
			<< pstarts[i] << " " << pdeadlines[i] << " " << plengths[i] << ")" << endl;
	}
}

void Cluster::Add(int pNum, double pWeight, double pDeadline, double pStart){
	pNumbers.push_back(pNum);
	pstarts.push_back(pStart);
	pweights.push_back(pWeight);
	pdeadlines.push_back(pDeadline);
	plengths.push_back(pDeadline - pStart);
	weight += pWeight;
	if (pDeadline > deadline)
		deadline = pDeadline;
	if (pNumbers.size() == 1) tstart = pStart;
	else if (pStart < tstart) tstart = pStart;
	length = deadline - tstart;
}

void Cluster::Change(int index1, int index2){
	int val = pNumbers[index1];
	pNumbers[index1] = pNumbers[index2];
	pNumbers[index2] = val;
	double res = pweights[index1];
	pweights[index1] = pweights[index2];
	pweights[index2] = res;
	res = pdeadlines[index1];
	pdeadlines[index1] = pdeadlines[index2];
	pdeadlines[index2] = res;
	res = plengths[index1];
	plengths[index1] = plengths[index2];
	plengths[index2] = res;
	res = pstarts[index1];
	pstarts[index1] = pstarts[index2];
	pstarts[index2] = res;
}

void Cluster::InsertAfter(int toInsert, int after){
	pNumbers.insert(pNumbers.begin() + after + 1, pNumbers[toInsert]);
	pNumbers.erase(pNumbers.begin() + toInsert);
	pweights.insert(pweights.begin() + after + 1, pweights[toInsert]);
	pweights.erase(pweights.begin() + toInsert);
	plengths.insert(plengths.begin() + after + 1, plengths[toInsert]);
	plengths.erase(plengths.begin() + toInsert);
	pdeadlines.insert(pdeadlines.begin() + after + 1, pdeadlines[toInsert]);
	pdeadlines.erase(pdeadlines.begin() + toInsert);
	pstarts.insert(pstarts.begin() + after + 1, pstarts[toInsert]);
	pstarts.erase(pstarts.begin() + toInsert);
}

void Cluster::Delete(int index){
	double weight = pweights[index], tstart = pstarts[index], deadline = pdeadlines[index];
	pNumbers.erase(pNumbers.begin() + index);
	pweights.erase(pweights.begin() + index);
	plengths.erase(plengths.begin() + index);
	pdeadlines.erase(pdeadlines.begin() + index);
	pstarts.erase(pstarts.begin() + index);
	this->weight -= weight;
	int newStart = this->deadline, newDeadline = 0;
	if (tstart == this->tstart) {
		for (size_t i = 0; i < pNumbers.size(); i++)
			if (pstarts[i]<newStart)
				newStart = pstarts[i];
	}
	if (deadline == this->deadline) {
		for (size_t i = 0; i < pNumbers.size(); i++)
			if (pdeadlines[i] > newDeadline)
				newDeadline = pdeadlines[i];
	}
	this->deadline = newDeadline;
	this->tstart = newStart;
	this->length = deadline - tstart;
	if (newDeadline == newStart){
		cout << "error" << endl;
		system("pause");
	}
}

void Cluster::SetLength(){
	try{
        if (tstart > deadline)
            throw UserException("Cluster::GetLength(). Negative cluster length");
       length =  deadline - tstart;
    }
      catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

double Cluster::GetLength(){
	return length;
	
}

void Cluster::GetInfo(clusterInfo &info){
	for (size_t i = 0; i < pNumbers.size(); i++ ) info.get<0>().push_back(pNumbers[i]);
	for (size_t i = 0; i < pstarts.size(); i++ ) info.get<1>().push_back(pstarts[i]);
	for (size_t i = 0; i < pdeadlines.size(); i++ ) info.get<2>().push_back(pdeadlines[i]);
	for (size_t i = 0; i < pweights.size(); i++ ) info.get<3>().push_back(pweights[i]);

}

Cluster::~Cluster(void)
{
}
