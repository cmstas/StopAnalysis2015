#include "Riostream.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TString.h"

#include <vector>

void mergeHadoopFiles(const TString& indir, const TString& sample, const TString& outpath) {

  // Print user commands
  cout << "Merging files from dir: " << indir << endl;
  cout << "Ouputting to: " << outpath << endl;

  // Declare new TChain
  TChain *chain = new TChain("t");
  chain->SetMaxTreeSize(5000000000LL); //default is 100000000000LL = 100Gb

  // Get number of files from input directory
  int nFiles_in = (int)chain->Add( Form("%s/%s*.root", indir.Data(), sample.Data() ) );

  // Merge chain
  int nFiles_out = chain->Merge(outpath, "fast");
  
  // Create list of names of histograms to add as well
  std::vector<TString> histNames;
  histNames.push_back( "h_counter" );

  // Get number of histograms
  const int nHistos = (int) histNames.size();

  // Get first output file
  TFile *f_output = new TFile(outpath, "update");

  // Initialize pointers for each histogram
  TH1D *histos[nHistos];
  for(int iHist=0; iHist<nHistos; iHist++) histos[iHist]=NULL;

  // Loop over input files, adding histograms
  bool firstFile=true;
  TObjArray *listOfFiles = chain->GetListOfFiles();
  TIter fileIter(listOfFiles);
  TFile *currentFile = 0;

  while ( (currentFile = (TFile*)fileIter.Next()) ) { 
    
    // Open input file
    TFile file(  currentFile->GetTitle() );
    
    // Check if input file exists
    if( file.IsZombie() ) continue;
    
    // Loop over histograms in file to add them
    for(int iHist=0; iHist<nHistos; iHist++){
      
      // If this is the first file, then clone the histos
      if( firstFile ){
	TH1D *h_temp = (TH1D*)file.Get( histNames[iHist] );
	histos[iHist] = (TH1D*)h_temp->Clone();
	histos[iHist]->SetDirectory(f_output);
      }
      // If not the first file, then add the histos
      else{
	TH1D *h_temp = (TH1D*)file.Get( histNames[iHist] );
	histos[iHist]->Add( h_temp );
      }
      
    } // end loop over histograms

    // Close input file
    file.Close();

    firstFile=false;

  } // end loop over input files

  f_output->Write();


  // Copy histograms to output files
  if(nFiles_out>1){

    TString outpath_base = outpath;
    outpath_base.ReplaceAll(".root","");
    
    for(int iFile=1; iFile<nFiles_out; iFile++){

      TString iOutPath = Form( "%s_%d.root", outpath_base.Data(), iFile );
      TFile *outFile = new TFile(iOutPath, "update");

      TH1D *h_temp[nHistos];
      for(int iHist=0; iHist<nHistos; iHist++) h_temp[iHist]=NULL;

      for(int iHist=0; iHist<nHistos; iHist++){
	h_temp[iHist] = (TH1D*)histos[iHist]->Clone();
	h_temp[iHist]->SetDirectory(outFile);
      } // end loop over histograms

      // Clean up outfile
      outFile->Write();
      outFile->Close();
      delete outFile;

    } // end loop over output files

  } // end if nFiles_out>1

  // Clean up
  f_output->Close();
  delete f_output;

  return;
}
