/*
SPSevt2root.cpp
Takes .evt files from nscldaq 11 and converts them into .root files.
This version is kept up to date for the Super-Enge Splitpole at FSU, and was built
using a framework devised by Nabin, ddc, KTM et. al. in Dec 2015
Gordon M. Feb. 2019
*/

/*
Revised for ResoNeut Feb. 2019 
Eli T.
*/

#include "evt2root.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

//constructor
evt2root::evt2root() {

  cout << "Enter evt list file: ";
  cin >> fileName;   

  adc1.resize(32); adc2.resize(32); adc3.resize(32); adc4.resize(32); adc5.resize(32);
  tdc1.resize(32); tdc2.resize(32); qdc1.resize(32); qdc2.resize(32); qdc3.resize(32);

  adc1_geo = 4; // Set geo addresses here 
  adc_geos.push_back(adc1_geo);
  adc2_geo = 5; // You can find these in daqconfig.tcl
  adc_geos.push_back(adc2_geo);
  adc3_geo = 7;
  adc_geos.push_back(adc3_geo);
  adc4_geo = 8;
  adc_geos.push_back(adc4_geo);
  adc5_geo = 12;
  adc_geos.push_back(adc5_geo);
  
  tdc1_geo = 14;
  adc_geos.push_back(tdc1_geo);
  tdc2_geo = 15;
  adc_geos.push_back(tdc2_geo);
  
  qdc1_geo = 3;
  qdc_geos.push_back(qdc1_geo);
  qdc2_geo = 10;
  qdc_geos.push_back(qdc2_geo);
  qdc3_geo = 17;
  qdc_geos.push_back(qdc3_geo);

}

//Destructor
evt2root::~evt2root() {
  delete rootFile;
  delete DataTree;
}

// Reset()
// Each event needs to be processed separetely
// This cleans all the variables

void evt2root::Reset(){
  for(int i = 0; i<32;i++){
    adc1[i] = 0;
    adc2[i] = 0;
    adc3[i] = 0;
    adc4[i] = 0;
    adc5[i] = 0;
    tdc1[i] = 0;
    tdc2[i] = 0;
    qdc1[i] = 0;
    qdc2[i] = 0;
    qdc3[i] = 0;
  }
}

// setParameters()
// Does the heavy lifting of setting all non-raw channel paramters.
// Copy from Parameters.cpp in SpecTclResoNeut 

void evt2root::setParameters() {
 
}

// unpack()
// This is where the file is actually parsed. Takes a short pointer and traverses the .evt file, 
// seaching for header buffers that match the modules. Once a valid header is found, the module
// unpacker is called. The end data is then sorted into the indivually addressed modules of a 
// given type. No knowledge of the stack order is necessary to unpack 
 
void evt2root::unpack(uint16_t* eventPointer) {

  uint16_t* iterPointer = eventPointer;
  uint32_t numWords = *iterPointer++;
  uint16_t* end =  eventPointer + numWords+1;
  vector<ParsedADCEvent> adcData;
  vector<ParsedmQDCEvent> qdcData;

  Reset(); // Wipe Variables

  while (iterPointer<end){
    //check to make sure it matches a header
    if (adc_unpacker.isHeader(*iterPointer) && *(iterPointer-1) != 0xffff) {
      auto adc = adc_unpacker.parse(iterPointer-1, end, adc_geos);
      adcData.push_back(adc.second);
      iterPointer = adc.first;
    } else if (mqdc_unpacker.isHeader(*iterPointer)) {
      auto mqdc = mqdc_unpacker.parse(iterPointer-1, end, qdc_geos);
      qdcData.push_back(mqdc.second);
      iterPointer = mqdc.first;
    } else iterPointer++;
  }

  // See daqconfig.tcl for stack config event
  for (auto& event : adcData) {
    for (auto& chanData : event.s_data) {
      if (event.s_geo == adc1_geo) adc1[chanData.first] = chanData.second;
      else if (event.s_geo == adc2_geo) adc2[chanData.first] = chanData.second;
      else if (event.s_geo == adc3_geo) adc3[chanData.first] = chanData.second;
      else if (event.s_geo == adc4_geo) adc4[chanData.first] = chanData.second;
      else if (event.s_geo == adc5_geo) adc5[chanData.first] = chanData.second;
      else if (event.s_geo == tdc1_geo) tdc1[chanData.first] = chanData.second;
      else if (event.s_geo == tdc2_geo) tdc2[chanData.first] = chanData.second;
    }
  }
  for (auto& event : qdcData) {
    for (auto& chanData : event.s_data) {
      if (event.s_id == qdc1_geo) qdc1[chanData.first] = chanData.second;
      else if (event.s_id == qdc2_geo) qdc2[chanData.first] = chanData.second;
      else if (event.s_id == qdc3_geo) qdc3[chanData.first] = chanData.second;
    }
  }
  
  setParameters();
  DataTree->Fill();
}

// run()
// function to be called at exectuion. Takes the list of evt files and opens them one at a time,
// calls unpack() to unpack them, and then either completes or moves on to the next evt file.
// If a condition is not met, returns 0.
int evt2root::run() {
  
  DataTree = new TTree("DataTree","DataTree");
  
  ifstream evtListFile;
  evtListFile.open(fileName.c_str());
  if (evtListFile.is_open()) {
   cout << "Successfully opened: "<< fileName << endl;
  } else { 
   cout<<"Unable to open evt list input file"<<endl;
   return 0;
  }

  string temp;
  evtListFile>>temp;
  char rootName[temp.size()+1];
  strcpy(rootName, temp.c_str());
  
  rootFile = new TFile(rootName, "RECREATE");
  cout<<"ROOT File: "<<temp<<endl;
    
  //Add branches here
  DataTree->Branch("adc1", &adc1);
  DataTree->Branch("adc2", &adc2);
  DataTree->Branch("adc3", &adc3);
  DataTree->Branch("adc4", &adc4);
  DataTree->Branch("adc5", &adc5);
  
  DataTree->Branch("tdc1", &tdc1);
  DataTree->Branch("tdc2", &tdc2);
 
  DataTree->Branch("qdc1", &qdc1);
  DataTree->Branch("qdc2", &qdc2);
  DataTree->Branch("qdc3", &qdc3);
  
  string evtName; 
  evtListFile >> evtName;

  int physBuffers = 0; //can report number of event buffers; consistency check with spectcl
        
  while (!evtListFile.eof()) {
    ifstream evtFile;
    evtFile.clear(); //make sure that evtFile is always empty before trying a new one
    evtFile.open(evtName.c_str(), ios::binary);
    if (evtFile.is_open()) {
      cout<<"evt file: "<<evtName<<endl;
      while (!evtFile.eof()) {
        evtFile.read(buffer, 8);//take first 8 characters
	evtFile.read(buffer+8, *(uint32_t*)buffer-8);//read the remainder
	uint32_t subheader = *(uint32_t*)(buffer+8); //pull the subheader

        if (subheader>0) {
          cout <<"Unexpected subheader: " << subheader << endl; //relic from old version
        }

        auto eventPointer = ((uint16_t*)buffer)+6;//where we try to start a phys event
        auto bufferType = *(unsigned int*)(buffer+4);//determine what part of the file we're at
        int runNum;
        switch (bufferType) {
          case 30: //Physics event buffer
             unpack(eventPointer);
             physBuffers++;
             break;
          case 1: //start of run buffer
            runNum = *(eventPointer+8);
            cout <<"Run number = "<<runNum<<endl;
            cout <<"Should match with file name: " <<evtName<<endl;
            break;
        }
      }
    } else{
      cout<<"Unable to open evt file: "<<evtName<<endl;
      rootFile->Close();
      return 0;
    }
    cout<<"Number of physics buffers: "<<physBuffers<<endl;
    evtFile.close();
    evtFile.clear();
    evtListFile >> evtName;
  }
  evtListFile.close();
  DataTree->Write();
  rootFile->Close();
  return 1;
}
