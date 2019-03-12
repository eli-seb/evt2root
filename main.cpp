#include "evt2root.h"
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  TApplication app("app", &argc, argv); //if someone wants root graphics
  evt2root converter;
  cout<<"--------------- evt2root ---------------"<<endl;
  converter.run();
}
