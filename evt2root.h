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
    ~evt2root();
    int run();
 
  private:
    void setParameters();
    void unpack(uint16_t* eventPointer);
    void Reset();
    const int BufferWords = 13328; //Left over from previous version?
    const int BufferBytes = BufferWords*2;
    static const int BufferLength = 26656; //the same value as buffer bytes?
    char buffer[BufferLength];
    string fileName;
    TFile *rootFile;
    TTree *DataTree;

    //ROOT branch parameters
    vector<Int_t> adc1, adc2, adc3, adc4, adc5,
                  tdc1, tdc2, qdc1, qdc2, qdc3;
 
    //geoaddresses
    int adc1_geo, adc2_geo, adc3_geo, adc4_geo, adc5_geo,
        tdc1_geo, tdc2_geo, qdc1_geo, qdc2_geo, qdc3_geo;

    vector<int> adc_geos, qdc_geos;
  
    //module unpackers
    ADCUnpacker adc_unpacker;
    mQDCUnpacker mqdc_unpacker;
};

#endif
