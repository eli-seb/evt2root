/*
ADCUnpacker.cpp

Class to parse through data coming from ADC 
Most of the program's structure borrowed from http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html
Ken H. & Gordon M.
Oct 2018

This particular version is designed to work with SPSevt2root and uses 16-bit integer pointers to
traverse, instead of the spectcl 32-bit translator pointers. Possible area for improvement?
Gordon M. Feb 2019

NOTE: This version is specifically designed to unpack data that come in reverse!!
*/
#include "ADCUnpacker.h"
#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;

//This is where most chagnes need to be made for each module; most else is just name changes
//useful masks and shifts for ADC:
static const uint16_t TYPE_MASK (0x0700);
static const uint16_t TYPE_HDR (0x0200);
static const uint16_t TYPE_DATA (0x0000);
static const uint16_t TYPE_TRAIL (0x0400);

static const unsigned GEO_SHIFT (11);
static const uint16_t GEO_MASK (0xf800);

//header-specific:
static const unsigned HDR_COUNT_SHIFT (8);
static const uint16_t HDR_COUNT_MASK (0x3f00);
static const unsigned HDR_CRATE_SHIFT (16);
static const uint16_t HDR_CRATE_MASK (0x00ff);

//data-specific:
static const unsigned DATA_CHANSHIFT (0);
static const uint16_t DATA_CHANMASK (0x001f);
static const uint16_t DATA_CONVMASK (0x3fff);


pair< uint16_t*, ParsedADCEvent> ADCUnpacker::parse( uint16_t* begin,  uint16_t* end, vector<int> geos) {

  ParsedADCEvent event;
  int bad_flag = 0;
  int geo_flag = 0;

  auto iter = begin;

  unpackHeader(iter, event);
  if (iter>end)  {
    bad_flag = 1;
  }
  iter+=2;
  int nWords = event.s_count*2;
  auto dataEnd = iter + nWords;
  for (unsigned int i=0; i<geos.size(); i++) {
    if(event.s_geo == geos[i]) {
      geo_flag = 1;
      break;
    }
  }
  if (!geo_flag){//If unexpected geo, skip all data words; either bad event or bad stack
    bad_flag = 1;
    iter+=nWords;
    //Error testing
    //cout<<"Bad ADC geo: "<<event.s_geo<<endl;  
  } else {
    iter = unpackData(iter, dataEnd, event);
  }
  if (iter>end || bad_flag || !isEOE(*(iter+1))){
    //Error testing
    //cout<<"ADCUnpacker::parse() ";
    //cout<<"Unable to unpack event"<<endl;
  }
  

  iter+=2;

  return make_pair(iter, event);

}

bool ADCUnpacker::isHeader(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void ADCUnpacker::unpackHeader(uint16_t* word, ParsedADCEvent& event) {

  //Error handling: if not valid header, throw 0 in chan 0 for invalid geo 
  try {  
    if (!isHeader(*(word+1))) {
      string errmsg("ADCUnpacker::parseHeader() ");
      errmsg += "Found non-header word when expecting header. ";
      errmsg += "Word = ";
      unsigned short w = *(word+1);
      errmsg += to_string(w);
      throw errmsg;
    }

    event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
    ++word;
    event.s_geo = (*word&GEO_MASK)>>GEO_SHIFT;
    event.s_crate = (*word&HDR_CRATE_MASK) >> HDR_CRATE_SHIFT;
  } catch (string errmsg) {
    event.s_count = 0;
    event.s_geo = 99; //should NEVER match a valid geo
    event.s_crate = 0;
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    //cout<<errmsg<<endl; //only turn on when testing
  }

}

bool ADCUnpacker::isData(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA);
}

void ADCUnpacker::unpackDatum(uint16_t* word, ParsedADCEvent& event) {
  
  //Error handling: if not valid data throw 0 in chan 0, make geo invalid 
  try {
    if (!isData(*(word+1))) {
      string errmsg ("ADCUnpacker::unpackDatum() ");
      errmsg += "Found non-data word when expecting data.";
      throw errmsg;
    }

    uint16_t data = *word&DATA_CONVMASK;
    ++word;
    int channel = (*word&DATA_CHANMASK) >> DATA_CHANSHIFT;

    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
  } catch (string errmsg) {
    uint16_t data = 0;
    uint16_t channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    event.s_geo = 99; //should NEVER match valid geo
    //cout<<errmsg<<endl; //only turn on for error testing
  }
  
}

 uint16_t* ADCUnpacker::unpackData( uint16_t* begin,uint16_t* end, ParsedADCEvent& event) {

  event.s_data.reserve(event.s_count); //memory allocation

  auto iter = begin;
  while (iter!=end) {
    unpackDatum(iter, event);
    iter = iter+2;
  }

  return iter;

}

bool ADCUnpacker::isEOE(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}


