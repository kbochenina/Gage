#include "StdAfx.h"
#include "DataInfo.h"
#include <sstream> // istringstream
#include <boost/filesystem.hpp> // directory_iterator, path
#include <iterator>
#include "direct.h"
#include <windows.h> // for  GetModuleFileName


using namespace boost::filesystem; // directory_iterator, path

const int PACKS_MAX = 50;

DataInfo::~DataInfo(void)
{
}

DataInfo::DataInfo( string fSettings, double mL )
{
   minL = mL;
    Init(fSettings);
   _mkdir("Output");
   _chdir("Output");
    //SetTransferValues();
  
}

double DataInfo::GetAvgTransferFrom(const int& globalNum) const{
	// getting wfNum, localPackageNum
	int wfNum, localNum;
	GetLocalNumbers(globalNum, wfNum, localNum);
	// getting resTypes for globalNum
	vector <int> currentResTypes = Workflows(wfNum)[localNum].GetResTypes();
	vector <int> linked;
	Workflows(wfNum).GetOutput(localNum, linked);

	double maxAvgTransfer = 0.0;

	for (auto& linkedIt : linked){
		 double transfer = Workflows(wfNum).GetTransfer(localNum, linkedIt);
		 vector <int> linkedResTypes = Workflows(wfNum)[linkedIt].GetResTypes(); 
		 double transferTime = 0.0;

		 for (auto& currentIt : currentResTypes){
			 for (auto& linkedIt : linkedResTypes){
				 double band = GetBandwidth(currentIt - 1, linkedIt - 1); 
				 if (band!= 0) transferTime += transfer / band;
			}
		 }

		 double avgTransfer = transferTime / (currentResTypes.size() * linkedResTypes.size());
		 if (avgTransfer > maxAvgTransfer) maxAvgTransfer = avgTransfer;
	}
	return maxAvgTransfer;
}

double DataInfo::GetMaxLastTasksExecTime(int wfNum) const{
	double maxTime = 0.0;
	for (int i = 0; i < Workflows(wfNum).GetPackageCount(); i++){
		if (Workflows(wfNum).IsPackageLast(i)){
			double maxPackageTime = Workflows(wfNum).GetMaximumExecTime(i);
			if (maxPackageTime > maxTime)
				maxTime = maxPackageTime;
		}
	}
	return maxTime;
}

void  DataInfo::SetTransferValues(){
    ofstream file("transfer.txt");
    int wfNum = 0;
    double avgBandwidth = 0.0;
    for (size_t i = 0; i < bandwidth.size(); i++){
        int resBandwidth = 0;
        for (size_t j = 0; j < bandwidth.size(); j++)
            resBandwidth += bandwidth[i][j];
        resBandwidth *= resources[i].GetProcessorsCount();
        avgBandwidth += resBandwidth;
    }
    avgBandwidth /= processorsCount;

    for (const auto& wf : workflows){
        vector <vector<double>> transfer;
        int pCount = wf.GetPackageCount();
        transfer.resize(pCount);
        
        for (int i = 0; i < pCount; i++){
            for (int j = 0; j < pCount; j++)
                transfer[i].push_back(0);
        }
        
        if (context.GetCCR() != 0) {
            double avgResPerf = GetAvgPerf();
            double conseqAmount = 0.0;
            for (int i = 0; i < pCount; i++)
                conseqAmount += wf.GetAmount(i);
            double avgTime = conseqAmount / avgResPerf;
            double fullAmount = context.GetCCR() * avgTime / avgBandwidth;
            // in megabytes
            double avgAmount = fullAmount / (pCount - wf.GetLastPackagesCount());
            for (int i = 0; i < pCount; i++){
				double h = context.GetH(), outPackageAmount = 0.0;
				if (h == 0) 
					outPackageAmount = avgAmount;
				else
					outPackageAmount = rand()% static_cast<int>(2 * h * avgAmount) 
                    + avgAmount * (1 - h);
                vector <int> out;
                wf.GetOutput(i, out);
                double avgPackageAmount = outPackageAmount / out.size();
                for (size_t j = 0; j < out.size(); j++){
                    transfer[i][out[j]] = avgPackageAmount;
                }
            }
        }
        file << "Workflow # " << wfNum << endl;
        for (size_t i = 0; i < transfer.size(); i++){
            for (size_t j = 0; j < transfer.size(); j++)
                file << transfer[i][j] << " ";
            file << endl;
        }
        workflows[wfNum].SetTransfer(transfer);
        wfNum++;
    }
    file.close();
}

double DataInfo::GetAvgPerf(){
    double perf = 0.0;
    for (const auto& res: resources){
        perf += res.GetPerf() * res.GetProcessorsCount();
    }
    return perf/processorsCount;
}

const double DataInfo::GetDeadline(int wfNum){
    return workflows[wfNum].GetDeadline();
}

void DataInfo::Init(string settingsFile){
   try{
      char second[21]; 
      ifstream file(settingsFile, ifstream::in);
      string errOpen = "File " + settingsFile + " was not open";
      string errWrongFormat = "Wrong format in file " + settingsFile + " at line ";
      string errWrongValue = "Wrong value of parameter ";
      string errWrongFormatFull = errWrongFormat;
      string errEarlyEnd = "Unexpected end of file " + settingsFile;

      wchar_t buffer[100];
      GetModuleFileName( NULL, buffer, MAX_PATH );
      cout << "Current dir: " ;
      wcout << buffer;
      cout << endl;

      if (file.fail()) 
          throw UserException(errOpen);
           
      unsigned int line = 0;
      string s, trim;
      int T = 0, delta = 0;
	   double mL = 0.0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "canExecuteOnDiffResources=";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int flag = atoi(s.c_str());
      bool canExecuteOnDiffResources;
      if (flag==1) canExecuteOnDiffResources = true;
      else if (flag == 0) canExecuteOnDiffResources = false;
      else throw UserException(errWrongValue + "canExecuteOnDiffResources");
      // InputFolderPath="FolderName"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "InputFolderPath=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      path dir = s;
      // DebugInfoFile="debugInfoFileName"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "DebugInfoFile=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      string openErr = " DebugInfoFile cannot be open";
      ofstream ex;
      ex.open(s, ios::app);
      if (ex.fail()) throw UserException(openErr);
      // ResultFile="result.txt"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "ResultFile=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      string resFileName = s;
      // T=Tvalue
      getline(file,s);
      ++line;
      trim = "T=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      T = stoi(s);
	  // minLength=minLengthValue
	  getline(file,s);
      ++line;
      trim = "minLength=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      mL = stof(s);
      
	  double koeff = 0.0;
	  // minLength=minLengthValue
	  getline(file,s);
      ++line;
      trim = "koeff=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      koeff = stof(s);

      double CCR = 0.0;
      
      getline(file,s);
      ++line;
      trim = "CCR=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      CCR = stof(s);

      double h = 0.0;
      getline(file,s);
      ++line;
      trim = "h=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      h = stof(s);

	  
      
      
      // read all filenames from path
      string resourcesFileName;
      string bandwidthFileName;
      vector <string> WFFileNames;
      bool isResourcesFileWasFound = false;
      for (directory_iterator it(dir), end; it != end; ++it) 
      {
         std::cout << "File processed - ";
         std::cout << *it << std::endl;
         string filename = it->path().string();
         if (filename.find("b") != string::npos) {
             bandwidthFileName = filename;
             continue;
         }
         if (filename.find("res")==string::npos && filename.find("n")==string::npos) continue;
         if (filename.find("res")!=string::npos ){
            resourcesFileName = filename;
            if (isResourcesFileWasFound == true)
                throw UserException("InputFiles folder contains more than 1 file with resource description");
            isResourcesFileWasFound = true;
         }
         else 
            WFFileNames.push_back(filename);
      }
      InitResources(resourcesFileName, canExecuteOnDiffResources);
      InitBandwidth(bandwidthFileName);
      for (vector<string>::iterator it = WFFileNames.begin(); it!= WFFileNames.end(); it++)
         //InitWorkflowsFromDAX(*it);
         InitWorkflows(*it);

	 /* mL = this->minL;

	  T = mL + workflows.size() * koeff * mL;*/
     cout << "T = " << T + T * koeff << endl;
	  context.SetContext(T, CCR, h, mL);	

	  for (int i = 0; i < workflows.size(); i++){
       /* double deadline = rand() / static_cast<double>(RAND_MAX) * (T - mL) + mL;
		  double tstart = rand() / static_cast<double>(RAND_MAX) * (deadline-mL);*/
		  //double deadline = rand() / static_cast<double>(RAND_MAX) * (T - mL) + mL;
		  double tstart = 0;
		  //double tstart =  0;
		  double deadline = T;
		  workflows[i].SetDeadline(deadline);
		  workflows[i].SetTStart(tstart);
		  cout << "WfNum: " << i <<" Tstart:" << tstart << " Deadline " << deadline << endl;
	  }
	  // setting deadlines and tstarts
	  //double singleLength = 20000;
	  //T = singleLength;
	  //context.SetT(T);
	  //double currentLength = singleLength + (workflows.size()-1) * singleLength / GetT() * singleLength / 2;
	  //cout << currentLength << endl;
	  //double maxLength = rand()%50000 + 30000;//GetT() * pacs.size()/50;
	  //for (int i = 0; i < workflows.size(); i++){
		 // double tstart = (GetT() == currentLength) ? 0.00 : (rand() / static_cast<double>(RAND_MAX) * (GetT() - currentLength));
		 // double deadline = tstart + currentLength;
		 // //tstart = 0;
		 // //deadline = T;
		 // workflows[i].SetDeadline(deadline);
		 // workflows[i].SetTStart(tstart);
		 // cout << "Tstart:" << tstart << " Deadline " << deadline << endl;
	  //}
      int initNum = 0;
      initPackageNumbers.resize(workflows.size());
      for (size_t i = 0; i < workflows.size(); i++){
         initPackageNumbers[i] = initNum;
         initNum += workflows[i].GetPackageCount();
      }
      ofstream resTime("Output/time.txt");
      cout << "Start to init finishing times..." << endl;
      double t = clock();
      InitFinishingTimes();
      double end = (clock()-t)/1000.0 ;
      cout << "Time of init finishing times " << end << endl;
      resTime << "Time of init finishing times " << end << endl;
      resTime.close();
      ex.close();
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
   catch (std::exception& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void DataInfo::InitWorkflowsFromDAX(string fname){
	try{
		ifstream file(fname, ifstream::in);
		string errOpen = "File " + fname + " was not open";
		string errEarlyEnd = "Unexpected end of file " + fname;
		string errWrongFormat = "Wrong format in file " + fname + " at line ";
		string errConnMatrix = "Wrong value in connectivity matrix";
		string errJobsCount = "Can not find jobs count value in file " + fname;
		string errWrongFormatFull = errWrongFormat;
		if (file.fail()) throw UserException(errOpen);
		double maxPerf = GetMaxPerf();
		bool jobsCountFound = false;
		int jobsCount = 0;
		string s;
		while (!jobsCountFound){
			getline(file,s);
			size_t found = s.find("jobCount=\"");
			if (found != std::string::npos){
				
				s.erase(0,found+10);
				istringstream iss(s);
				iss >> jobsCount;
				jobsCountFound = true;
			}
			if (file.eof()) throw UserException(errJobsCount);
		}
		//cout << jobsCount << endl;
		vector<vector<pair<string, double>>> inputFiles;
		vector<vector<pair<string,double>>> outputFiles;
		vector <vector<double>> transfer;
		vector <vector <int>> connectMatrix;
		vector<string> jobNames;
		inputFiles.resize(jobsCount);
		outputFiles.resize(jobsCount);
		transfer.resize(jobsCount);
		connectMatrix.resize(jobsCount);
		jobNames.resize(jobsCount);
		for (auto &row: transfer)
			row.resize(jobsCount);
		for (auto &row: connectMatrix)
			row.resize(jobsCount);
		vector <Package> pacs;
		getline(file,s);
		getline(file,s);
		for (int i = 0; i < jobsCount; i++){
			getline(file,s);
			if (s.find("job ")!= std::string::npos){
				size_t foundFirst = s.find("\"");
				size_t foundSecond = s.find("\"",foundFirst);
				string jobName = s.substr(foundFirst, foundSecond);
				//cout << jobName << endl;
				size_t runtimePos = s.find("runtime=\"");
				s.erase(0,runtimePos+9);
				double runTime = 0.0;
				istringstream iss(s);
				iss >> runTime;
				//cout << runTime << endl;
				double amount = maxPerf / runTime * 60;
				//cout << amount << endl;
				map <pair <int,int>, double> execTime;
				vector<int> types;
				vector<int> cCount;
				cCount.push_back(1);
				for (int j = 0; j < resources.size(); j++){
					double currentTime = amount / (resources[j].GetPerf() / maxPerf);
					execTime.insert(make_pair(make_pair(j+1, 1), currentTime));
					//cout << currentTime << endl;
					types.push_back(j+1);
				}
				Package p(i ,types, cCount, execTime, amount, 0);
				pacs.push_back(p);
				bool isNextJob = false;
				while (!isNextJob){
					getline(file,s);
					if (s.find("</job>") != std::string::npos) {
						isNextJob = true;
						//std::system("pause");
						continue;
					}
					if (s.find("<!--") != std::string::npos){
						isNextJob = true;
						continue;
					}
					size_t foundFirst = s.find("\"");
					size_t foundSecond = s.find("\"",foundFirst+1);
					string fileName = s.substr(foundFirst+1, foundSecond-foundFirst-1);
					foundFirst = s.find("\"", foundSecond+1);
					foundSecond = s.find("\"",foundFirst+1);
					string direction = s.substr(foundFirst+1, foundSecond-foundFirst-1);
					foundFirst = s.find("size=\"", foundSecond);
					s.erase(0,foundFirst+6);
					double size;
					istringstream iss(s);
					iss >> size;
					size /= 1048576;
					if (direction=="input"){
						inputFiles[i].push_back(make_pair(fileName,size));
					}
					else {
						outputFiles[i].push_back(make_pair(fileName,size));
					}
					//cout << fileName << " " << direction << " " << size << endl;
				} 
			}

		}
		// set dependencies
		for (int i = 0; i < jobsCount; i++){
			for (int j = 0; j < jobsCount; j++){
				for (auto & output: outputFiles[i]){
					string fileName = output.first;
					for (auto& input: inputFiles[j]){
						if (fileName == input.first){
							connectMatrix[i][j] = 1;
							transfer[i][j] = output.second;
						}
					}
				}
			}
		}

	/*	for (int i = 0; i < jobsCount; i++){
			for (int j = 0; j < jobsCount; j++){
				cout << connectMatrix[i][j] << " ";
			}
			cout << endl;
		}

		for (int i = 0; i < jobsCount; i++){
			for (int j = 0; j < jobsCount; j++){
				cout << transfer[i][j] << " ";
			}
			cout << endl;
		}*/
		//double tstart = 0.0;
		//double deadline = GetT();
		double maxLength = rand()%10000 + 10000;//GetT() * pacs.size()/50;
		double tstart = (GetT() == maxLength) ? 0.00 : (rand() / static_cast<double>(RAND_MAX) * (GetT() - maxLength));
		//double tstart = rand() / static_cast<double>(RAND_MAX) * GetT() / 3;
		double deadline = tstart + maxLength;
		/*if (workflows.size()==0){
			tstart = 0;
		    deadline = 5000;
		}
		if (workflows.size()==1){
			tstart = 2500;
			deadline = 6500;
		}
		if (workflows.size()==2) {
			tstart = 5000;
			deadline = 9500;
		}*/
		//double deadline = tstart - 1;
		//double deadline = tstart + maxLength;
		Workflow w(workflows.size() + 1, pacs,connectMatrix, deadline, transfer, tstart);
		//cout << "Tstart:" << tstart << " Deadline " << deadline << endl;
		workflows.push_back(w);
		//std::system("pause");
	}
	catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
   catch (std::exception& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }

}

double DataInfo::GetMaxPerf(){
	double res = 0.0;
	for (int i = 0; i < resources.size(); i++){
		double currentPerf = resources[i].GetPerf();
		if (currentPerf > res)
			res = currentPerf;
	}
	return res;
}

// NOTE: type numbers are counted from 1!
void DataInfo::InitWorkflows(string f){
   try{
      char second[21]; // enough to hold all numbers up to 64-bits
      ifstream file(f, ifstream::in);
      string errOpen = "File " + f + " was not open";
      string errEarlyEnd = "Unexpected end of file " + f;
      string errWrongFormat = "Wrong format in file " + f + " at line ";
      string errPackagesCount = "Packages count cannot be less than 1";
      string errCoresCount = "Cores count cannot be less than 1";
      string errConnMatrix = "Wrong value in connectivity matrix";
      string errWrongFormatFull = errWrongFormat;
      if (file.fail()) throw UserException(errOpen);

      string s, trim; int line = 0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "Workflows count = ";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int workflowsCount = stoi(s);
      vector <int> types; map <pair <int,int>, double> execTime; vector <Package> pacs; 
      vector <int> cCount; 
      vector <vector <int>> connectMatrix;
      int fullPackagesCount = 0;
      for (int i = 0; i < workflowsCount; i++){
         int packagesCount = 0;
         getline(file,s);
         ++line;
         if (file.eof()) throw UserException(errEarlyEnd);

         if ((found = s.find("(")) == std::string::npos){
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,found+1);
         istringstream iss(s);
         iss >> packagesCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         if (packagesCount < 1) {
            sprintf_s(second, "%d", i+1);
            string beginStr = "Workflow ";
            beginStr += second;
            beginStr += " - ";
            beginStr += errPackagesCount;
            throw UserException(beginStr);
         }
         vector <vector<double>> transfer;
         transfer.resize(packagesCount);
         for (int j = 0; j < packagesCount; j++){
             transfer[j].resize(packagesCount);
             for (int k = 0; k < packagesCount; k++)
                 transfer[j][k] = 0.0;
         }
         for (int j = 0; j < packagesCount; j++){
            double alpha = 0.0; // part of consequentually executed code
            ++fullPackagesCount;
            // Package [packageNumber]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            // Alpha: [alpha value]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Alpha: ";
            size_t found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            iss >> alpha;
            if (iss.fail()) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            // Resource types: [resource types values]. -1 means all possible resources
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Resources types: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            int typeNumber = 0;
            string comma;
            do{
               comma = "";
               iss >> typeNumber;
               // if package can execute on all possible resources
               if (typeNumber == -1){
                  for (unsigned int i = 0; i < resources.size(); i++)
                     types.push_back(i+1);
                  break;
               }
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               types.push_back(typeNumber);
               iss >> comma;
            } while (comma==",");



            // Cores count: [cores count values]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Processors count: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            int coresCount = 0;
            iss.str(s);
            iss.clear();
            int coreCount = 0;
            do{
               comma = "";
               iss >> coreCount;
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               cCount.push_back(coreCount);
               iss >> comma;
            } while (comma==",");

            if (cCount.size() < 1) {
               sprintf_s(second, "%d", i+1);
               string beginStr = "Workflow ";
               beginStr += second;
               beginStr += " - ";
               beginStr += errCoresCount;
               throw UserException(beginStr);
            }

            // Computational amount: [amount value]
            long int amount = 0;
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "GFlop: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            iss >> amount;
            if (iss.fail()) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }

            // Transfer (packages in a file are numbered from 1)
            unsigned index = 0;
            double transferVal = 0.0;
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Transfer: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            string current;
            int outPackageNumber = -1;
            double dataSize = 0.0;
            bool isPackageNumber = true;
            while (iss >> current){
                   if (iss.fail()) {
                   sprintf_s(second, "%d", line);
                   errWrongFormatFull += second;
                   throw UserException(errWrongFormatFull);
                }
                if (isPackageNumber){
                    current.erase(0,1);
                    outPackageNumber = atoi(current.c_str());
                }
                else {
                    found = s.find("Mb");
                    s.erase(found,2);
                    dataSize = atof(current.c_str());
                }
                if (!isPackageNumber){
					if (fullPackagesCount - 1 > transfer.size() - 1 || outPackageNumber - 1 > transfer[0].size() - 1)
						throw UserException("DataInfo::InitWorkflows() : wrong input file, package transfer number is out of range");
                    transfer[fullPackagesCount-1][outPackageNumber-1] = dataSize;
				}
                isPackageNumber = !isPackageNumber;
            }

            for (unsigned int k = 0; k < types.size(); k++){
               for (unsigned int l = 0; l < cCount.size(); l++){
                  // assume that the core numbers are in ascending order (else continue)
                  if (resources[k].GetProcessorsCount() < cCount[l]) break; 
                  // Amdal's law
                  double acc = (double) 1.00 / (alpha + (1-alpha)/(l+1));
                  // execTime = amount / (perf * acc)
                  double exTime = amount / (resources[k].GetPerf() * acc);
                  execTime.insert(make_pair(make_pair(types[k], cCount[l]), exTime));
               }
            }
            Package p(fullPackagesCount,types,cCount, execTime, amount, alpha);
            pacs.push_back(p);
            types.clear();
            execTime.clear();
            cCount.clear();
         }
         getline(file,s);
         if (file.eof()) throw UserException(errEarlyEnd);
         ++line;
         for (int j = 0; j < packagesCount; j++){
            vector <int> row;
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            iss.str(s);
            iss.clear();
            for (int k = 0; k < packagesCount; k++){
               int val = 0;
               iss >> val;
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               if (val!=0  && val!=1){
                  sprintf_s(second, "%d", i+1);
                  string beginStr = "Workflow ";
                  beginStr += second;
                  beginStr += " - ";
                  beginStr += errConnMatrix;
                  throw UserException(beginStr);
               }
               row.push_back(val);
            }
            connectMatrix.push_back(row);
         }


		 double maxLength = 20000;//GetT() * pacs.size()/50;
		//double deadline = GetT();
		//double tstart = 0.00;
		//cout << "maxTstart " << GetT() - maxLength << endl;
		
		 double tstart = (GetT() == maxLength) ? 0.00 : (rand() / static_cast<double>(RAND_MAX) * (GetT() - maxLength));
		//double tstart = rand() / static_cast<double>(RAND_MAX) * GetT() / 2;
		//double deadline = tstart - 1;
		 double deadline = tstart + maxLength;
		//while (deadline < tstart)
		//	deadline = rand() / static_cast<double>(RAND_MAX) * GetT();
		
	     //double deadline = GetT(), tstart = 0;
         Workflow w(workflows.size() + i+1, pacs,connectMatrix, deadline, transfer, tstart);
		  // cout << "Tstart:" << tstart << " Deadline " << deadline << endl;
         workflows.push_back(w);
         pacs.clear();
         connectMatrix.clear();
      }

   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void DataInfo::InitResources(string f, bool canExecuteOnDiffResources){
   try{
      processorsCount = 0;
      map <int, vector<pair <int,int>>> busyIntervals;
      char second[21]; // enough to hold all numbers up to 64-bits
      ifstream file(f.c_str(), ifstream::in);
      string errOpen = "File " + f + " was not open";
      string errEarlyEnd = "Unexpected end of file " + f;
      string errWrongFormat = "Wrong format in file " + f + " at line ";
      string errWrongFormatFull = errWrongFormat;
      //string errStageBorders = "InitResources(): stageBorders vector is empty!";
      //if (stageBorders.size()==0) throw UserException(errStageBorders);
      if (file.fail()) 
         throw UserException(errOpen);
      string s, trim; int line = 0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "Resources count = ";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int allResourcesCount = stoi(s);

      trim = "Resources types count = ";
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int typesCount = stoi(s);
      int resourcesCount = 0, coresCount = 0;

      for (int i = 0; i < typesCount; i++)
      {
         vector <BusyIntervals> typeBI;
         istringstream iss(s);
         getline(file,s);
         ++line;
         if (file.eof()) throw UserException(errEarlyEnd);
         sprintf_s(second, "%d", i+1);
         string first = "Type ";
         trim = first + second;
         found = s.find(trim);
         if (found != 0) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,trim.size()+2);
         iss.str(s);
         iss.clear();
         iss >> resourcesCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         found = s.find(",");
         s.erase(0,found+2);
         iss.str(s);
         iss.clear();
         iss >> coresCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         double perf = 0.0;
         getline(file,s);
         trim = "Performance (GFlops): ";
         found = s.find(trim);
         if (found != 0) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,trim.size());
         perf = atof(s.c_str());

         for (int j = 0; j < resourcesCount; j++){
            getline(file,s);
            ++line;
            if (file.eof()) throw UserException(errEarlyEnd);
            busyIntervals.clear();
            for (int k = 0; k < coresCount; k++){
               getline(file,s);
               ++line;
               if (file.eof()) throw UserException(errEarlyEnd);
               sprintf_s(second, "%d", k+1);
               first = "Core ";
               trim = first + second;
               found = s.find(trim);
               if (found != 0) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               s.erase(0,trim.size()+1);
               int diapCount = stoi(s);
               vector<pair<int,int>> oneResDiaps;
               for (int l = 0; l < diapCount; l++){
                  if (file.eof()) throw UserException(errEarlyEnd);
                  getline(file,s);
                  ++line;
                  iss.str(s);
                  iss.clear();
                  int one,two;
                  iss >> one;
                  if (iss.fail()) {
                     sprintf_s(second, "%d", line);
                     errWrongFormatFull += second;
                     throw UserException(errWrongFormatFull);
                  }
                  iss >> two;
                  if (iss.fail()) {
                     sprintf_s(second, "%d", line);
                     errWrongFormatFull += second;
                     throw UserException(errWrongFormatFull);
                  }
                  oneResDiaps.push_back(make_pair(one,two));
               }
               busyIntervals.insert(make_pair(k+1, oneResDiaps));
			   // !!!
			  //busyIntervals.clear();
            }
            // add busyIntervals for current resource to a vector <BusyIntervals>
            typeBI.push_back(busyIntervals);
         }
		
         ResourceType r(i+1,resourcesCount, coresCount, perf, typeBI, canExecuteOnDiffResources, context);
         resources.push_back(r);
         processorsCount += coresCount * resourcesCount;
      }

        file.close();
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// init bandwidth speed
// without any verification
void DataInfo::InitBandwidth(string fName){
    try{
        ifstream file(fName, ifstream::in);
        string s;
        getline(file,s);
        string trim = "Types count: ";
        size_t found = s.find(trim);
        s.erase(0,trim.size());
        int resTypes = atoi(s.c_str());
        if (resTypes != resources.size())
            throw UserException("DataInfo::InitBandwidth() error. Wrong resource type count");
        // min bandwidth
        getline(file,s);
        // max bandwidth
        getline(file,s);
        // koeff
        getline(file,s);
        // matrix header
        getline(file,s);
        bandwidth.resize(resources.size());
        for (size_t i = 0; i < resources.size(); i++){
            getline(file,s);
            istringstream iss(s);
            bandwidth[i].resize(resources.size());
            for (size_t j = 0; j < resources.size(); j++){
                double val = 0.0;
                iss >> val;
                bandwidth[i][j] = val;
            }
        }
    }
        catch (std::exception& e){
        cout<<"error : " << e.what() <<endl;
        std::system("pause");
        exit(EXIT_FAILURE);
   }

}

void DataInfo::FixBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->FixBusyIntervals();
   }
}

void DataInfo::ResetBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->ResetBusyIntervals();
   }
}

void DataInfo::SetInitBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->SetInitBusyIntervals();
   }
}

void DataInfo::GetCurrentIntervals(vector<vector<BusyIntervals>> &out){
   out.resize(resources.size());
   for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
      resources[res].GetCurrentIntervals(out[res]);
   }
}

void DataInfo::SetCurrentIntervals(vector<vector<BusyIntervals>> &out){
   out.resize(resources.size());
   for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
      resources[res].SetCurrentIntervals(out[res]);
   }
}

int DataInfo::GetResourceType(int number){
   try{
      if (number < 0 || number > processorsCount-1) 
         throw UserException("DataInfo::GetResourceType() error. Wrong coreNumber");
      int current = 0;
      for (vector <ResourceType>::iterator it = resources.begin(); it!= resources.end(); it++){
         int currentCoreCount = it->GetProcessorsCount();
         if (number >= current && number < current + currentCoreCount) 
            return distance(resources.begin(), it);
         current+=currentCoreCount;
      }
      return -1;
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// get initial core index of resource type
int DataInfo::GetInitResourceTypeIndex(int type){
   int index = 0;
   for (int i = 0; i < type; i++)
      index += resources[i].GetProcessorsCount();
   return index;
}


// PRE: wfNum >=0 && wfNum < workflows.size()
const Workflow& DataInfo::Workflows(int wfNum) const {
   try{
      if ( wfNum < 0 || static_cast<unsigned>(wfNum) >= workflows.size() )
         throw UserException("DataInfo::Workflows() error. Index out of range");
      return workflows[wfNum]; 
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// PRE: resNum >=0 && resNum < resources.size()
ResourceType& DataInfo::Resources(int resNum)  {
   try {
      if ( resNum < 0 || static_cast<unsigned>(resNum) >= resources.size() )
         throw UserException("DataInfo::Resources() error. Index out of range");
      return resources[resNum]; 
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}


void DataInfo::InitFinishingTimes(){
   int wfIndex = 0;
   double avgCalcPower = 0.0;
   for (size_t i = 0; i < resources.size(); i++)
      avgCalcPower += resources[i].GetPerf();
   avgCalcPower /= resources.size();
   // for each workflow
   for (Workflow& wf: workflows){
      wf.SetFinishingTimes(avgCalcPower);
   }
}

// get all packages count
unsigned int DataInfo::GetPackagesCount(){
   unsigned int count = 0;
   for (size_t i = 0; i < workflows.size(); i++)
      count += workflows[i].GetPackageCount();
   return count;
}

// set priorities to whole packages (packages numbered from zero according to wf order)
void DataInfo::SetPriorities(){
   // constructing list of pairs (package number, finishing time)
   list <pair<int, double>> priorityList;
   int pNumber = 0;
   for (size_t i = 0; i < workflows.size(); i++){
      for (size_t j = 0; j < workflows[i].GetFinishingTimesSize(); j++)
         priorityList.insert(priorityList.end(),make_pair(pNumber++, workflows[i].GetFinishingTime(j)));
   }
   double minTime = std::numeric_limits<double>::infinity();
   int minIndex = 0;

   while (priorities.size() < GetPackagesCount()){
      list<pair<int, double>>::iterator position, dependPosition = priorityList.begin();
      for (list<pair<int, double>>::iterator it = priorityList.begin();
         it!= priorityList.end(); it++){
         if (it->second < minTime){
                  minTime = it->second;
                  minIndex = it->first;
                  position = it;
         }
      }
      int wfIndex, localPackage;
      GetLocalNumbers(minIndex, wfIndex, localPackage);
      vector<int> dependsOn;
      workflows[wfIndex].GetInput(localPackage, dependsOn);
      bool allPrioretized = true;
      for (size_t i = 0; i < dependsOn.size(); i++){
         dependsOn[i] += (minIndex - localPackage);
                  
         if (find(priorities.begin(), priorities.end(), dependsOn[i]) == priorities.end()){
            allPrioretized = false;
            for (size_t j = i+1; j < dependsOn.size(); j++)
               dependsOn[j] += (minIndex - localPackage);
            // find last depend position
            for (size_t i = 0; i < dependsOn.size(); i++){
               auto foundPosition = 
                  find(priorityList.begin(), priorityList.end(),
                  make_pair(dependsOn[i], workflows[wfIndex].GetFinishingTime(localPackage)));

               if (distance(foundPosition, dependPosition))
                  dependPosition = foundPosition;
            }
            priorityList.erase(position);
            priorityList.insert(dependPosition, make_pair(minIndex,minTime));
            break;
         }
      }

      if (allPrioretized) {
         priorities.push_back(minIndex);
         priorityList.erase(position);
      }
      
      
      minTime = std::numeric_limits<double>::infinity();
   }
   // smallest elements will be in the end of vector
   reverse(priorities.begin(), priorities.end());
}

// set different wf priorities
void DataInfo::SetWfPriorities(){
   // constructing list of pairs (package number, finishing time)
   for (size_t i = 0; i < workflows.size(); i++){
      workflows[i].SetPriorities();
   }
   //SetT(maxDeadline);
   cout << "Max deadline = " << GetT() << endl;

}

// get resource type by global index
int DataInfo::GetResourceTypeIndex(int globalIndex){
   int index = 0; 
   int border = resources[index].GetProcessorsCount();
   while (1){
      if (globalIndex < border){
         return index;
      }
      index++;
      border += resources[index].GetProcessorsCount();
   }
}

// getting next package with smallest finishing time
int DataInfo::GetNextPackage(){
   try{
      if (priorities.size() == 0)
         throw UserException("DataInfo::GetNextPackage() error. Priority is empty");
      int next = priorities.back();
      priorities.pop_back();
      return next;
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// get wfNum and local package number for global package number
void DataInfo::GetLocalNumbers (const int & current, int &wfNum, int &localNum) const{
   int aggregated = 0;
   for (size_t i = 0; i < workflows.size(); i++){
      if (current >= aggregated && current < aggregated + workflows[i].GetPackageCount()){
         wfNum = i;
         localNum = current - aggregated;
         return;
      }
      aggregated += workflows[i].GetPackageCount();
   }
}

// get global processor index 
int DataInfo::GetGlobalProcessorIndex(int resource, int local){
   int global = 0;
   for (int i = 0; i < resource; i++)
      global += resources[i].GetProcessorsCount();
   global += local;
   return global;
}

double DataInfo::GetDeadline(){
     double maxDeadline = 0.0; 
     for (size_t i = 0; i < workflows.size(); i++) {
         double d = workflows[i].GetDeadline() ;
         if (d > maxDeadline) 
             maxDeadline = d; 
     }
     return maxDeadline;
}

// remove some numbers from priorities
void DataInfo::RemoveFromPriorities(const vector<int>& toRemove){
   for (const auto& val : toRemove){
      auto & it = find(priorities.begin(), priorities.end(), val);
      if (it != priorities.end()) priorities.erase(it);
   }
}

double DataInfo::GetBandwidth(const int& from, const int& to) const {
    try{
        if (from > bandwidth.size()-1 || from < 0 || to > bandwidth.size()-1 || to < 0)
            throw UserException("DataInfo::GetBandwidth() error. Wrong parameters");
        return bandwidth[from][to];
    }
      catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}
