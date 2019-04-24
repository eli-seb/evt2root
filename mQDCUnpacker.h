/*
CRawmQDCUnpacker.h

class to parse through data coming from mqdc

most of the program's structure borrowed from http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html

gordon m.
Nov 2018
*/

#ifndef mqdcunpacker_h
#define mqdcunpacker_h

#include <vector>
#include <utility>
#include <cstdint>

struct ParsedmQDCEvent {
  int s_id;
  int s_res;
  int s_count;
  int s_eventNumber;   
  std::vector<std::pair<int, std::uint16_t>> s_data;
};


class mQDCUnpacker {
  public:
    std::pair< uint16_t*, ParsedmQDCEvent> parse( uint16_t* begin,  uint16_t* end, std::vector<int> ids);
    bool isHeader(std::uint16_t word);

  private:
    bool isData(std::uint16_t word);
    bool isEOE(std::uint16_t word); 
   
    void unpackHeader(std::uint16_t* word, ParsedmQDCEvent& event);
    void unpackDatum(std::uint16_t* word, ParsedmQDCEvent& event); 
    uint16_t* unpackData( uint16_t* begin,  uint16_t* end, ParsedmQDCEvent& event); 
};


#endif
        
