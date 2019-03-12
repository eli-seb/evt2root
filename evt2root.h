/*SPSevt2root.h
 *Takes .evt files from nscldaq 11 and converts them into .root files.
 *This version is kept up to date for the Super-Enge Splitpole at FSU, and was built
 *using a framework devised by Nabin, ddc, KTM et. al. in Dec 2015
 *
 *Gordon M. Feb. 2019
 */

#ifndef EVT2ROOT_H
#define EVT2ROOT_H

#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <vector>
#include <cstdint>
#include "ADCUnpacker.h"
#include "mTDCUnpacker.h"
#include "mQDCUnpacker.h" 

using namespace std;

class evt2root {

  public:
    evt2root();
    int run();
 
  private:
    void setParameters();
    void unpack(uint16_t* eventPointer);
    void Reset();
    const int BufferWords = 13328; //Left over from previous version?
    const int BufferBytes = BufferWords*2;
    static const int BufferLength = 26656; //the same value as buffer bytes?
    char buffer[BufferLength];
    Float_t nanos_per_chan = 0.0625; //ps->ns conv. for mtdc
    string fileName;
    TFile *rootFile;
    TTree *DataTree;

    //ROOT branch parameters
    Int_t adc1[32];
    Int_t adc2[32];
    Int_t adc3[32];
    Int_t adc4[32];
    Int_t adc5[32];

    Int_t tdc1[32];
    Int_t tdc2[32];

    Int_t qdc1[32];
    Int_t qdc2[32];
    Int_t qdc3[32];
 
    //geoaddresses
    int adc1_geo;
    int adc2_geo;
    int adc3_geo;
    int adc4_geo;
    int adc5_geo;

    int tdc1_geo;
    int tdc2_geo;

    int qdc1_geo;
    int qdc2_geo;
    int qdc3_geo;
  
    //module unpackers
    ADCUnpacker adc_unpacker;
    mTDCUnpacker mtdc_unpacker;
    mQDCUnpacker mqdc_unpacker;
};

#endif
