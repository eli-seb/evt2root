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
   
  //Initalize vector size
  adc1.resize(32); adc2.resize(32); adc3.resize(32); adc4.resize(32); adc5.resize(32);
  tdc1.resize(32); tdc2.resize(32); qdc1.resize(32); qdc2.resize(32); qdc3.resize(32);
  neut_min.resize(12); neut_max.resize(12); neut_pos_sum.resize(12); neut_max_min.resize(12);
  tdc2_diff.resize(12);

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

  rand = new TRandom3();

}

//Destructor
evt2root::~evt2root() {
  delete rootFile;
  delete DataTree;
  delete rand;
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
  for(int i = 12; i<12;i++){
    neut_min[i] = 0;
    neut_max[i] = 0;
    neut_pos_sum[i] = 0;
    neut_max_min[i] = 0;
    tdc2_diff[i] = 0;
  }
  e_s1=0.0;
  e_s2=0.0;
  tdc2_all=0.0;

}

// setParameters()
// Does the heavy lifting of setting all non-raw channel paramters.
// Copy from Parameters.cpp in SpecTclResoNeut 

void evt2root::setParameters() { 
     
  int i,j,k;
  Int_t tp_max[4];
  Int_t tp_min[4];
  //Make Random number for bin uncertainty
     // + rand->Rndm();
  
  // Loop Over Each Detector/Crystal
  for (i=0;i<12;i++){
    for (j=0;j<4;j++){
      if (i<8){
	if (qdc2[j+4*i]>0 && qdc2[j+4*i]<4000){
	  tp_max[j] = (Int_t)((Float_t)qdc2[j+4*i]+rand->Rndm());
	}
	else{
	  tp_max[j]=0;
	}
      } 
      else {
	if (qdc1[j+4*(i-8)]>0 && qdc1[j+4*(i-8)]<4000){
	  tp_max[j] = (Int_t)((Float_t)qdc1[j+4*(i-8)]+rand->Rndm()); 
	}
	else{
	  tp_max[j]=0;
	}
      }
    }
     
    // find segment with minimum & maximum charge 
    for (j=0;j<4;j++){
      if (i<8){
	if (qdc2[j+4*i]>0 && qdc2[j+4*i]<4000){
	  tp_min[j] = (Int_t)((Float_t)qdc2[j+4*i]+rand->Rndm());
	}
	else{
	  tp_min[j]=0;
	}
      } 
      else {
	if (qdc1[j+4*(i-8)]>0 && qdc1[j+4*(i-8)]<4000){
	  tp_min[j] = (Int_t)((Float_t)qdc1[j+4*(i-8)]+rand->Rndm()); 
	}
	else{
	  tp_min[j]=0;
	}
      }
    }
    neut_pos_sum[i] = 0;
    neut_max[i] = 0;
    neut_min[i] = tp_min[0];
    
    for (j=0;j<4;j++){
      neut_pos_sum[i] = neut_pos_sum[i] + tp_max[j];
      if (tp_max[j]>neut_max[i]){
	neut_max[i] = tp_max[j];
      }
      if (tp_min[j] <= tp_min[0]){
	neut_min[i] = tp_min[j];
      }
    }
    neut_max_min[i] = neut_max[i] - neut_min[i];
  }

  // TDC
  int first_hit = 4096;

  for (j=0;j<12;j++){
    if (first_hit > tdc2[j+16] && tdc2[j+16] > 0){
      first_hit = tdc2[j+16];
    }
  }
  
  tdc2_all = (Float_t)first_hit;

  for (j=0;j<12;j++){ 
    tdc2_diff[j]=(Int_t)((Float_t)tdc2[j+16]-(Float_t)first_hit + 1000.0 + rand->Rndm());
  }


  // Si 
  float si_demax = 100;
  float si_emax = 100;

  for (k=0; k<16; k++){
    if (si_demax < adc1[k]){
      si_demax = adc1[k];
      e_s1 = si_demax;
    } 
    if (si_emax < adc2[k]){
      si_emax = adc2[k];
      e_s2 = si_emax;
    }
  }

 

}

/*unpack()
This is where the file is actually parsed. Takes a short pointer and traverses the .evt file, 
seaching for header buffers that match the modules. Once a valid header is found, the module
unpacker is called. The end data is then sorted into the indivually addressed modules of a 
given type. No knowledge of the stack order is necessary to unpack 
*/ 
void evt2root::unpack(uint16_t* eventPointer) {

  uint16_t* iterPointer = eventPointer;
  uint32_t numWords = *iterPointer++;
  uint16_t* end =  eventPointer+numWords+1;
  /*vector<ParsedmTDCEvent> tdcData; mTDC & mQDC use save event upacker */
  vector<ParsedADCEvent> adcData;
  vector<ParsedmQDCEvent> qdcData;

  Reset(); // Wipe Variables

  while (iterPointer<end){
    //check to make sure it matches a header
    if (adc_unpacker.isHeader(*iterPointer) && *(iterPointer-1) != 0xffff) {
      auto adc = adc_unpacker.parse(iterPointer-1, end, adc_geos);
      adcData.push_back(adc.second);
      iterPointer = adc.first;
    } else if (mqdc_unpacker.isHeader(*iterPointer) && *(iterPointer-1) != 0xffff) {
      auto mqdc = mqdc_unpacker.parse(iterPointer-1, end, qdc_geos, numWords);
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

  DataTree->Branch("neut_max", &neut_max);
  DataTree->Branch("neut_min", &neut_min);
  DataTree->Branch("neut_max_min", &neut_max_min);
  DataTree->Branch("neut_pos_sum", &neut_pos_sum);

  DataTree->Branch("e_s1", &e_s1, "e_s1/F");
  DataTree->Branch("e_s2", &e_s2, "e_s2/F");

  DataTree->Branch("tdc2_all", &tdc2_all,"tdc2_all/F");
  DataTree->Branch("tdc2_diff",&tdc2_diff);

  string evtName; 

  while (evtListFile >> evtName) {
    ifstream evtFile;
    evtFile.clear(); //make sure that evtFile is always empty before trying a new one
    evtFile.open(evtName.c_str(), ios::binary);
    int physBuffers = 0; //can report number of event buffers; consistency check with spectcl
    if (evtFile.is_open()) {
      cout<<"evt file: "<<evtName<<endl;
      char buffer[8];
      while (evtFile.read(buffer, 8)) {
        uint32_t ringSize = *(uint32_t*)buffer - 8; //first buffer contains size of ring
        char ringBuffer[ringSize];
	evtFile.read(ringBuffer, ringSize);//pull a ringBuffer to read
	uint32_t bodyheader_size = *(uint32_t*)ringBuffer; //pull the bodyheader size 
        uint16_t *eventPointer;
        if (bodyheader_size != 0) {
          eventPointer = ((uint16_t*)ringBuffer)+bodyheader_size/2;//start point if bodyhead
        } else {
          eventPointer = ((uint16_t*)ringBuffer)+2;//otherwise just skip bodyhead_size
        }
        auto bufferType = *(unsigned int*)(buffer+4);//determine what type of ring
        int runNum;
        switch (bufferType) {
          case 30: //Physics event buffer
             unpack(eventPointer);
             physBuffers++;
             cout<<"\rNumber of physics buffers: "<<physBuffers<<flush;
             break;
          case 1: //start of run buffer
            runNum = *(eventPointer);
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
    cout<<endl;
    evtFile.close();
    evtFile.clear();
  }
  evtListFile.close();
  DataTree->Write();
  rootFile->Close();
  return 1;
}
