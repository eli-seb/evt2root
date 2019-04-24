/*
CRawmQDCUnpacker.cpp

Class to parse through data coming from mqdc 

Most of the program's structure borrowed from http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html


Gordon M.
Nov 2018
*/
#define HAVE_HLESS_IOSTREAM  //May not be necessary. Placed here for the sake of #include<Iostream.h> misbehaving
#include "mQDCUnpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

//This is the main place where changes need to be made from one module to another; all else mostly name changes
//useful masks and shifts for mQDC:
//here we have to read as 0xf000 instead of 0xc000 due to some errors in evt files
static const uint16_t TYPE_MASK (0xf000);
static const uint16_t TYPE_HDR (0x4000);
static const uint16_t TYPE_DATA (0x0000);
static const uint16_t TYPE_TRAIL (0xc000);


//header-specific:
static const unsigned HDR_ID_SHIFT (0);
static const uint16_t HDR_ID_MASK (0x00ff);
static const unsigned HDR_COUNT_SHIFT (0);
static const uint16_t HDR_COUNT_MASK (0x0fff);


//data-specific:
static const unsigned DATA_CHANSHIFT (0);
static const uint16_t DATA_CHANMASK (0x001f);
static const uint16_t DATA_CONVMASK (0x0fff);

pair< uint16_t*, ParsedmQDCEvent> mQDCUnpacker::parse( uint16_t* begin,  uint16_t* end, vector<int> ids) {

  ParsedmQDCEvent event;

  auto iter = begin;
  int bad_flag = 0;
  int id_flag = 0;
  unpackHeader(iter, event);
  if (iter > end){
    bad_flag =1;
  }
  iter += 2;
 

  int nWords = (event.s_count-1)*2; //count includes the eoe 
  auto dataEnd = iter + nWords;
  for (unsigned int i=0; i<ids.size(); i++) {
    if(event.s_id == ids[i]) {
      id_flag = 1;
      break;
    }
  }
  if (!id_flag) {//If unexpected id, skip data; either bad event or bad stack
    bad_flag = 1;
    iter+=nWords;
    //Error testing
    //cout<<"Bad mQDC id: "<<event.s_id<<endl;
  } else {
    iter = unpackData(iter, dataEnd, event);
  }
  //Jump EOE buffer
  iter +=2;

  return make_pair(iter, event);

}

bool mQDCUnpacker::isHeader(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void mQDCUnpacker::unpackHeader(uint16_t* word, ParsedmQDCEvent& event) {
  //Error testing
  try {
    if (!isHeader(*(word+1))) {
      string errmsg("mQDCUnpacker::parseHeader() ");
      errmsg += "Found non-header word when expecting header. ";
      errmsg += "Word = ";
      unsigned short w = *(word+1);
      errmsg += to_string(w);
      throw errmsg;
    }

    event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
    word++;
    event.s_id = (*word&HDR_ID_MASK)>>HDR_ID_SHIFT;
  } catch (string errmsg) {
    event.s_count = 1;
    event.s_id = 99; //should NEVER match a valid id 
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    //cout<<errmsg<<endl; //only turn on if testing
  }
}

bool mQDCUnpacker::isData(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA );
}

void mQDCUnpacker::unpackDatum(uint16_t* word, ParsedmQDCEvent& event) {
  //Error handling: if not valid data, throw 0 in chan 0 
  try {
    if (!isData(*(word+1))) {
      string errmsg("mQDCUnpacker::unpackDatum() ");
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
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_id = 99; //should NEVER match a valid id
    event.s_data.push_back(chanData);
    //cout<<errmsg<<endl; //only turn on if testing
  }
  
}

 uint16_t* mQDCUnpacker::unpackData( uint16_t* begin, uint16_t* end, ParsedmQDCEvent& event) {

  event.s_data.reserve(event.s_count+1); //memory allocation
  auto iter = begin;
  while (iter<end) {
    unpackDatum(iter, event);
    iter = iter+2;
  }

  return iter;

}

bool mQDCUnpacker::isEOE(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}


