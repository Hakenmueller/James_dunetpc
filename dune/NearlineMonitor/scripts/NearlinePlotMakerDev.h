#ifndef _NLPlotMkr
#define _NLPlotMkr

#include <fstream>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TDatime.h>
#include <TMultiGraph.h>
#include <TGaxis.h>
#include <TPaveStats.h>

// REMOVE includes below to be removed when I add partition to the OnMon header
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLStatement.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>

// NOTE:  Go through ALL of the above and remove what isn't necessary!!!
#include <sstream>


using namespace std;

void histogramZoom(TH1* hist, double n_sigma);
void graphZoom(TGraph* gr, double n_sigma);

//
// Define global variables to be used in NearlinePlotMaker.C
//

ifstream inFile;

TDatime *TNowGMT; // current GMT time
TDatime *TNow;    // current local time
TDatime *SRtime;
Int_t GMToffset;

unsigned int run;
unsigned int subrun;

UInt_t LastRun;
UInt_t LastSR;


struct NearlinePlotEnables{

  bool fMakeMetricTimeGraph;
  bool fMake2DHisto;
  bool fMakeBinByBinPlots;
  NearlinePlotEnables(bool make_metric_time_graph=true, bool make_2d_histo=true, bool make_bin_by_bin_plots=false){
    fMakeMetricTimeGraph=make_metric_time_graph;
    fMake2DHisto=make_2d_histo;
    fMakeBinByBinPlots=make_bin_by_bin_plots;
  }
};

struct NearlinePlotInfo{

  std::string fMetricName;
  int fChannel;
  int fNumDays;
  std::string fFileExtension;
  std::string fMetricDetails;

  void AddMetricDetails(std::string metric_details);
  std::string GetMetricMeanTimeGraphName(bool zoom=false);
  std::string GetMetricRmsTimeGraphName(bool zoom=false);
  std::string GetBinByBinTimeGraphName(unsigned int bin);
  

  NearlinePlotInfo(){};

  NearlinePlotInfo(std::string metric_name, int channel, int num_days, std::string file_extension, std::string metric_details=""){
    fMetricName = metric_name;
    fChannel = channel;
    fNumDays = num_days;
    fFileExtension = file_extension;
    fMetricDetails = metric_details;
  }
  
  std::string GetHistOutputName(){
    char name[256];
    if(fChannel >= 0) sprintf(name, "%sChan%04i_%.3i_days.%s", fMetricName.c_str(), fChannel, fNumDays, fFileExtension.c_str());
    else sprintf(name, "%s_%.3i_days.%s", fMetricName.c_str(), fNumDays, fFileExtension.c_str());
    return std::string(name);
  }

  std::string GetHist2DOutputName(){
    char name[256];
    if(fChannel >=0) sprintf(name, "%sVsTimeChan%04i_%.3i_days.%s", fMetricName.c_str(), fChannel, fNumDays, fFileExtension.c_str());
    else sprintf(name, "%sVsTime_%.3i_days.%s", fMetricName.c_str(), fNumDays, fFileExtension.c_str());
    return std::string(name);
  }

};

void NearlinePlotInfo::AddMetricDetails(std::string metric_details){
  fMetricDetails = metric_details;
}

std::string NearlinePlotInfo::GetMetricMeanTimeGraphName(bool zoom){
  char name[256];

  if(fChannel >= 0) sprintf(name, "%sMeanTimeChan%04i_%.3i_days", fMetricName.c_str(), fChannel, fNumDays);
  else  sprintf(name, "%sMeanTime_%.3i_days", fMetricName.c_str(), fNumDays);

  if(zoom) sprintf(name, "%s_zoom.%s", name, fFileExtension.c_str());
  else sprintf(name, "%s.%s", name, fFileExtension.c_str());

  return std::string(name);
}

std::string NearlinePlotInfo::GetMetricRmsTimeGraphName(bool zoom){
  char name[256];
  if(fChannel >= 0) sprintf(name, "%sRmsTimeChan%04i_%.3i_days", fMetricName.c_str(), fChannel, fNumDays);
  else  sprintf(name, "%sRmsTime_%.3i_days", fMetricName.c_str(), fNumDays);

  if(zoom) sprintf(name, "%s_zoom.%s", name, fFileExtension.c_str());
  else sprintf(name, "%s.%s", name, fFileExtension.c_str());

  return std::string(name);
}

std::string NearlinePlotInfo::GetBinByBinTimeGraphName(unsigned int bin){
  char name[256];
  if(fChannel>=0) sprintf(name, "%sTimeChan%04iBin%u_%.3i_days.%s", fMetricName.c_str(), fChannel, bin, fNumDays, fFileExtension.c_str());
  else sprintf(name, "%sTimeBin%u_%.3i_days.%s", fMetricName.c_str(), bin, fNumDays, fFileExtension.c_str());
  return std::string(name);
}




struct NearlinePlot{

  NearlinePlotEnables fPlotEnables;

  TH1F* fHistogram;
  std::string fHistName;
  NearlinePlotInfo fPlotInfo;
  std::string fHistTitle;

  TH2F* fHistogram2D;
  TH1F* fHistogram2DNormalisation;
  bool fNormalised;

  TGraph* fGraphMetricTime;
  TGraph* fGraphMetricRmsTime;
  std::vector<float> fMetricVec;
  std::vector<float> fMetricRmsVec;
  std::vector<float> fTimeVec;
  int fPlotCount;

  std::vector<TGraph*> fBinByBinGraphMetricTime;
  std::vector<std::vector<float>> fBinByBinMetricVec;
  std::vector<std::vector<float>> fBinByBinMetricErrorVec;

  NearlinePlot(std::string this_hist_name, NearlinePlotInfo this_plot_info, bool make_metric_time_graph=true, bool make_2d_histo=true, bool make_bin_by_bin_plots=false);


  bool AddHistogram(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago);
  bool AddHistogram1D(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago);
  bool AddHistogram2D(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago);
  TCanvas* makeHistoCanvas(TPaveText* updateText, int width=1200, int height=800);
  TCanvas* makeHisto2DCanvas(TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  TCanvas* makeGraphMetricTimeCanvas(TPaveText* updateText, int time_ago, int XNow, bool rms=false, bool zoom=false, int width=1200, int height=800, std::string taxis_labels="");
  TCanvas* makeBinByBinGraphTime(unsigned int bin, TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  void printPlots(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  void printHistogram1D(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  void printHistogram2D(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  void printGraphs(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width=1200, int height=800, std::string taxis_labels="");
  void setMetricDetails(std::string metric_details);

  void setHistTitle(std::string hist_title);
  void setPlotInfo(NearlinePlotInfo this_plot_info);

};


NearlinePlot::NearlinePlot(std::string this_hist_name, NearlinePlotInfo this_plot_info, bool make_metric_time_graph, bool make_2d_histo, bool make_bin_by_bin_plots)
{

  fHistogram=0;
  fHistName = this_hist_name;  
  fPlotInfo = this_plot_info;
  fHistTitle = "";

  fPlotEnables.fMake2DHisto=make_2d_histo;
  fHistogram2D=0;
  fHistogram2DNormalisation=0;
  fNormalised = false;

  fPlotEnables.fMakeMetricTimeGraph=make_metric_time_graph;
  fMetricVec.resize(0);
  fMetricRmsVec.resize(0);
  fTimeVec.resize(0);
  fPlotCount = 0;
  fGraphMetricTime = 0;
  fGraphMetricRmsTime = 0;

  fPlotEnables.fMakeBinByBinPlots=make_bin_by_bin_plots;
  fBinByBinGraphMetricTime.resize(0);
  fBinByBinMetricVec.resize(0);
  fBinByBinMetricErrorVec.resize(0);

}

void NearlinePlot::setHistTitle(std::string hist_title){
  fHistTitle = hist_title;
}

void NearlinePlot::setPlotInfo(NearlinePlotInfo this_plot_info){
  fPlotInfo = this_plot_info;
}

void NearlinePlot::setMetricDetails(std::string metric_details){
  fPlotInfo.AddMetricDetails(metric_details);
}


bool NearlinePlot::AddHistogram(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago){

  bool result;
  result = AddHistogram1D(file, header, Xsrtime, XNow, GMToffset, time_ago);
  if(fPlotEnables.fMake2DHisto) result = result & AddHistogram2D(file, header, Xsrtime, XNow, GMToffset, time_ago);
  return result;

}

bool NearlinePlot::AddHistogram1D(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago){

  TH1F *hist_temp = (TH1F*)file.FindObjectAny(fHistName.c_str());    
  if(hist_temp != 0){
    if(fHistogram == 0){
      fHistogram = (TH1F*) hist_temp->Clone((fHistName + "_temp").c_str());
      if(fHistTitle!="") fHistogram->SetTitle(fHistTitle.c_str());
      else fHistTitle = fHistogram->GetTitle();
      
      fHistogram->SetDirectory(0);
    }
    else fHistogram->Add(hist_temp,1.0);
  }    
  else{
    std::cerr << "NearlinePlot::AddHistogram - Failed to find histogram - " << fHistName << std::endl;
    return false;
  }
  if(hist_temp != 0 && header != 0 && Xsrtime != XNow - GMToffset) {
    fTimeVec.push_back(Xsrtime);
    fMetricVec.push_back(hist_temp->GetMean(1));
    fMetricRmsVec.push_back(hist_temp->GetRMS(1));

    if(fPlotEnables.fMakeBinByBinPlots){
      //Initiliase the binbybin vector of vectors
      if(fBinByBinMetricVec.size()==0){
        fBinByBinMetricVec.resize(hist_temp->GetNbinsX());
        fBinByBinMetricErrorVec.resize(hist_temp->GetNbinsX());
      }
      for(int bin=1; bin<=hist_temp->GetNbinsX();bin++){
        fBinByBinMetricVec.at(bin-1).push_back(hist_temp->GetBinContent(bin));
        fBinByBinMetricErrorVec.at(bin-1).push_back(hist_temp->GetBinError(bin));
      }
    }//fMakeBinByBinPlots

    fPlotCount++;
    //    std::cerr << "NearlinePlot::AddHistogram - Success" << std::endl;
    return true;
  }

  std::cerr << "NearlinePlot::AddHistogram - Failed - Shouldn't get here" << std::endl;
  return false;

}

bool NearlinePlot::AddHistogram2D(TFile const & file, TTree* header, int Xsrtime, int XNow, int GMToffset, int time_ago){

  TH1F *hist_temp = (TH1F*)file.FindObjectAny(fHistName.c_str());    
  if(hist_temp != 0){
    if(fHistogram2D == 0){
      int nbinsx = hist_temp->GetNbinsX();
      double xmin = hist_temp->GetBinLowEdge(1);
      double xmax = hist_temp->GetBinLowEdge(1+nbinsx);
      int nbinst = 72;//FIXME
      double tmin = time_ago;
      double tmax = XNow;
      fHistogram2D = new TH2F((fHistName + "_vs_time").c_str(), (std::string(hist_temp->GetTitle()) + " - vs Time").c_str(), nbinst, tmin, tmax, nbinsx, xmin, xmax);
      //      fHistogram2D->GetXaxis()->SetTitle("x axis");
      fHistogram2D->GetYaxis()->SetTitle(hist_temp->GetXaxis()->GetTitle());
      fHistogram2D->GetZaxis()->SetTitle(hist_temp->GetYaxis()->GetTitle());

      //Also make the normalisation histogram
      fHistogram2DNormalisation = new TH1F((fHistName + "_normalisation").c_str(), (std::string(hist_temp->GetName()) + " Normalisation").c_str(), nbinst, tmin, tmax);

      //Make sure we keep the histograms in memory
      fHistogram2D->SetDirectory(0);
      fHistogram2DNormalisation->SetDirectory(0);

    }//make TH2
  }//Got TH1
  else{
    std::cerr << "NearlinePlot::AddHistogram2D - Failed to find histogram - " << fHistName << std::endl;
    return false;
  }
  if(hist_temp != 0 && header != 0 && Xsrtime != XNow - GMToffset) {  
    //Now add this th1 to the th2
    int nbins = hist_temp->GetNbinsX();
    for(int i=0;i<nbins;i++){
      double content = hist_temp->GetBinContent(i);
      double center = hist_temp->GetBinCenter(i);
      if(content<=0.0) continue;
      fHistogram2D->Fill(Xsrtime, center, content);
    }//loops over the histogram bins
    
    //Add Entry to our normalisation
    fHistogram2DNormalisation->Fill(Xsrtime);
    return true;
  }
  std::cerr << "NearlinePlot::AddHistogram2D - Failed - Shouldn't get here" << std::endl;  
  return false;
}


TCanvas* NearlinePlot::makeHistoCanvas(TPaveText* updateText, int width, int height){

  std::string can_name = fHistName + "_can";
  std::string can_title = fHistName + "_can";

  TCanvas *can = new TCanvas(can_name.c_str(), can_title.c_str(), width, height);
  can->cd();
  gPad->SetGridx();
  gPad->SetGridy();
  can->SetLogy();
  gStyle->SetOptStat(111111);
  fHistogram->SetLineWidth(2);
  fHistogram->SetLineColor(kRed);
  fHistogram->SetName(fHistName.c_str());
  fHistogram->Draw();
  updateText->Draw();


  return can;
}

TCanvas* NearlinePlot::makeHisto2DCanvas(TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){

  std::string can_name = fHistName + "_vs_time_can";
  std::string can_title = fHistName + "_vs_time_can";

  TCanvas *can = new TCanvas(can_name.c_str(), can_title.c_str(), width, height);
  can->cd();
  can->SetLogz();
  can->SetRightMargin(0.15);
  gStyle->SetOptStat(0);
  gPad->SetGridx();
  gPad->SetGridy();
  if(taxis_labels==""){
    if(fPlotInfo.fNumDays <= 2) taxis_labels = "%H:%M";
    else taxis_labels = "%m/%d";
  }

  if(fNormalised!=true){
    for(int xbin=1;xbin<=fHistogram2D->GetNbinsX();xbin++){
      double norm = fHistogram2DNormalisation->GetBinContent(xbin);
      if(norm <= 0.0) continue;
      for(int ybin=1;ybin<=fHistogram2D->GetNbinsY();ybin++){
        int global_bin = fHistogram2D->GetBin(xbin, ybin);
        double content = fHistogram2D->GetBinContent(global_bin);
	if(content <= 0.0) continue;
        fHistogram2D->SetBinContent(global_bin, content/norm);
      }//ybin
    }//xbin
    fNormalised=true;
  }//Normalise


  fHistogram2D->GetXaxis()->SetTimeFormat(taxis_labels.c_str());
  fHistogram2D->GetXaxis()->SetLimits(time_ago,XNow);
  fHistogram2D->GetXaxis()->SetTitle("(central time)");
  fHistogram2D->GetXaxis()->SetTimeDisplay(1);
  fHistogram2D->GetXaxis()->SetLabelSize(0.03);
  fHistogram2D->Draw("COLZ");
  updateText->Draw();


  return can;
}

TCanvas* NearlinePlot::makeBinByBinGraphTime(unsigned int bin, TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){


  if(bin >= fBinByBinMetricVec.size()) return NULL;

  std::string can_name = fHistName + "_time_bin_" + std::to_string(bin);
  std::string can_title = "Mean of " + fHistTitle + " Bin " + std::to_string(bin);

  TCanvas* can = new TCanvas(can_name.c_str(), can_title.c_str(), width, height);
  can->cd();
  gPad->SetGridx();

  TGraph* gr = new TGraph(fPlotCount);
  for(int i=0;i<fPlotCount;i++) gr->SetPoint(i, fTimeVec.at(i), fBinByBinMetricVec.at(bin).at(i));//FIXME add errors
  
  gr->SetMarkerColor(kBlue);
  gr->GetXaxis()->SetTimeDisplay(1);
  gr->GetXaxis()->SetLabelSize(0.03);

  if(taxis_labels==""){
    if(fPlotInfo.fNumDays <= 2) taxis_labels = "%H:%M";
    else taxis_labels = "%m/%d";
  }

  gr->GetXaxis()->SetTimeFormat(taxis_labels.c_str());
  gr->GetXaxis()->SetLimits(time_ago,XNow);
  gr->GetXaxis()->SetTitle("(central time)");
  gr->Draw("A*");

  updateText->Draw();

  int maxtime = 0;
  double max  = 0.0, ave = 0.0;
  TPaveText *LastPoint = new TPaveText(0.3,0.88,0.93,0.93,"NDC");
  LastPoint->SetLineColor(1);
  LastPoint->SetFillColor(0);
  LastPoint->SetBorderSize(1);
  char lptext[128];
  maxtime = 0;
  max = 0.0;
  ave = 0.0;
  for(int i = 0; i < fPlotCount; ++i) {
    ave += (double)fMetricVec.at(i);
    if(fTimeVec.at(i) > maxtime) {
      maxtime = fTimeVec.at(i);
      max     = fMetricVec.at(i);
    }
  }
  if(fPlotCount > 0) ave = ave/(double)fPlotCount;
  sprintf(lptext,"Last Point = %f  /  Average = %f",max,ave);
  LastPoint->Clear();
  LastPoint->AddText(lptext);


  LastPoint->Draw();

  return can;
  

}

TCanvas* NearlinePlot::makeGraphMetricTimeCanvas(TPaveText* updateText, int time_ago, int XNow, bool rms, bool zoom, int width, int height, std::string taxis_labels){

  std::string can_name;
  if(rms) can_name= fHistName + "_rms_time_can";
  else can_name= fHistName + "_metric_time_can";

  std::string can_title;
  if(rms)  can_title = "RMS of " + fHistTitle;
  else  can_title = "Mean of " + fHistTitle;


  if(zoom) can_name += "_zoom";


  TCanvas* can = new TCanvas(can_name.c_str(), can_title.c_str(), width, height);
  can->cd();
  gPad->SetGridx();

  TGraph* gr;
  std::vector<float> yValuesVec;
  if(rms){
    fGraphMetricRmsTime = new TGraph(fPlotCount);
    gr = fGraphMetricRmsTime;
    yValuesVec = fMetricRmsVec;
  }
  else{
    fGraphMetricTime = new TGraph(fPlotCount);
    gr = fGraphMetricTime;
    yValuesVec = fMetricVec;
  }

  
  for(int i=0;i<fPlotCount;i++) gr->SetPoint(i, fTimeVec.at(i), yValuesVec.at(i));
  
  gr->SetMarkerColor(kBlue);
  gr->GetXaxis()->SetTimeDisplay(1);
  gr->GetXaxis()->SetLabelSize(0.03);

  if(taxis_labels==""){
    if(fPlotInfo.fNumDays <= 2) taxis_labels = "%H:%M";
    else taxis_labels = "%m/%d";
  }

  gr->GetXaxis()->SetTimeFormat(taxis_labels.c_str());
  gr->GetXaxis()->SetLimits(time_ago,XNow);
  gr->GetXaxis()->SetTitle("(central time)");
  gr->Draw("A*");

  if(zoom){
    graphZoom(gr, 2.0);
    gr->SetTitle(std::string(can_title + " - Zoom").c_str());
  }
  else   gr->SetTitle(can_title.c_str());

  updateText->Draw();

  int maxtime = 0;
  double max  = 0.0, ave = 0.0;
  TPaveText *LastPoint = new TPaveText(0.3,0.88,0.93,0.93,"NDC");
  LastPoint->SetLineColor(1);
  LastPoint->SetFillColor(0);
  LastPoint->SetBorderSize(1);
  char lptext[128];
  maxtime = 0;
  max = 0.0;
  ave = 0.0;
  for(int i = 0; i < fPlotCount; ++i) {
    ave += (double)fMetricVec.at(i);
    if(fTimeVec.at(i) > maxtime) {
      maxtime = fTimeVec.at(i);
      max     = fMetricVec.at(i);
    }
  }
  if(fPlotCount > 0) ave = ave/(double)fPlotCount;
  sprintf(lptext,"Last Point = %f  /  Average = %f",max,ave);
  LastPoint->Clear();
  LastPoint->AddText(lptext);


  LastPoint->Draw();

  return can;
  
}

void NearlinePlot::printPlots(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){

  printHistogram1D(plot_dir, updateText, time_ago, XNow, width, height, taxis_labels);

  if(fPlotEnables.fMake2DHisto) printHistogram2D(plot_dir, updateText, time_ago, XNow, width, height, taxis_labels);
  
  if(fPlotEnables.fMakeMetricTimeGraph) printGraphs(plot_dir, updateText, time_ago, XNow, width, height, taxis_labels);

}

void NearlinePlot::printHistogram1D(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){
  TCanvas* can = makeHistoCanvas(updateText,  width,  height);
  std::string can_name = plot_dir + "/" + fPlotInfo.GetHistOutputName();
  can->Print(can_name.c_str());
  delete can;
}

void NearlinePlot::printHistogram2D(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){
  TCanvas* can = makeHisto2DCanvas(updateText, time_ago, XNow,  width,  height, taxis_labels);
  std::string can_name = plot_dir + "/" + fPlotInfo.GetHist2DOutputName();
  can->Print(can_name.c_str());
  delete can;
}

void NearlinePlot::printGraphs(std::string plot_dir, TPaveText* updateText, int time_ago, int XNow, int width, int height, std::string taxis_labels){

  bool zoom;
  bool rms;
  TCanvas *can;
  std::string can_name;

  rms=false;  

  zoom=false;
  can = makeGraphMetricTimeCanvas(updateText, time_ago, XNow, rms, zoom);
  can_name = plot_dir + "/" + fPlotInfo.GetMetricMeanTimeGraphName(zoom);
  can->Print(can_name.c_str());

  delete can;

  zoom=true;
  can = makeGraphMetricTimeCanvas(updateText, time_ago, XNow, rms, zoom);
  can_name = plot_dir + "/" + fPlotInfo.GetMetricMeanTimeGraphName(zoom);
  can->Print(can_name.c_str());
  
  delete can;

  rms=true;  

  zoom=false;
  can = makeGraphMetricTimeCanvas(updateText, time_ago, XNow, rms, zoom);
  can_name = plot_dir + "/" + fPlotInfo.GetMetricRmsTimeGraphName(zoom);
  can->Print(can_name.c_str());

  delete can;

  zoom=true;
  can = makeGraphMetricTimeCanvas(updateText, time_ago, XNow, rms, zoom);
  can_name = plot_dir + "/" + fPlotInfo.GetMetricRmsTimeGraphName(zoom);
  can->Print(can_name.c_str());
  
  delete can;

  if(fPlotEnables.fMakeBinByBinPlots){
    for(unsigned int bin=0;bin<fBinByBinMetricVec.size();bin++){
      can = makeBinByBinGraphTime(bin, updateText, time_ago, XNow);
      can_name = plot_dir + "/" + fPlotInfo.GetBinByBinTimeGraphName(bin);
      can->Print(can_name.c_str());
      delete can;
    }//bin
  }

}



struct NearlineHTML{

  static std::string MakeStartYearPlot(std::string relative_plot_path){

    std::string output;
    output+="<h3>Start Year of the Run VS Run Number.</h3>\n";
    output+="<figure>\n";
    output+="<img src=\"" + relative_plot_path + "\" width=\"800\">\n";
    output+="</figure>\n";
    output+="<p>\n";
    output+="<b>This plot requires some explanation:</b>\n";
    output+="I noticed that for a large number of runs, some or all of the data appeared to be missing from the events in the files. This resulted in one of two behaviors:\n";
    output+="<BR>\n";
    output+="<b>1)</b> The run number was correct in the art::events but the year cooresponding to the event time was 0 (which defaults to 1969.)\n";
    output+="<BR>\n";
    output+="<b>2)</b> The file from that run was entirely devoid of usable events making the run number and year default to zero (again, 1969 for the year.)\n";
    output+="<BR>\n";
    output+="Both the run number and the year are obtained from the art::events by the nearline analyzer module. So this plot should be interpreted as a very general DAQ-health indicator. Situation 1 can occur if the art::events are present in the file but there is no RCE information. Situation 2 can occur if there are no good events in the entire file. I would imagine that once the DAQ is stable during normal running, his plot will become obsolete.\n";
    output+="</p>\n";
    output+="<BR><BR><BR>\n";

    return output;
  }

  static std::string MakeHistogram(std::string plot_location, NearlinePlotInfo plot_info){

    std::string metric_name = plot_info.fMetricName;
    int channel = plot_info.fChannel;
    std::string metric_details = plot_info.fMetricDetails;
    plot_location = plot_location + "/" + plot_info.GetHistOutputName();

    std::string output;
    output += "<hr>\n";
    if(channel >=0) output += "<h3>" + metric_name + " spectrum for channel # " + std::to_string(channel);
    else output += "<h3>" + metric_name + " spectrum";
    if(metric_details!="") output += " " + metric_details; 
    output += ".</h3>\n";
    output += "<figure>\n";
    output += "<img src=\"" + plot_location + "\" width=\"800\">\n"; 
    output += "</figure>\n"; 
    return output;
  }

  static std::string MakeHistogram2D(std::string plot_location, NearlinePlotInfo plot_info){

    std::string metric_name = plot_info.fMetricName;
    int channel = plot_info.fChannel;
    std::string metric_details = plot_info.fMetricDetails;
    plot_location = plot_location + "/" + plot_info.GetHist2DOutputName();

    std::string output;
    output += "<hr>\n";
    if(channel >= 0)    output += "<h3>" + metric_name + " spectrum vs. time for channel # " + std::to_string(channel);
    else     output += "<h3>" + metric_name + " spectrum vs. time";
    if(metric_details!="") output += " " + metric_details; 
    output += ".</h3>\n";
    output += "<figure>\n";
    output += "<img src=\"" + plot_location + "\" width=\"800\">\n"; 
    output += "</figure>\n"; 
    return output;

  }

  static std::string MakeHistogramPair(std::string plot_location, NearlinePlotInfo plot_info){

    std::string metric_name = plot_info.fMetricName;
    int channel = plot_info.fChannel;
    std::string metric_details = plot_info.fMetricDetails;

    std::string plot_location_histo = plot_location + "/" + plot_info.GetHistOutputName();
    std::string plot_location_histo2d = plot_location + "/" + plot_info.GetHist2DOutputName();

    std::string output;
    output += "<hr>\n";
    if(channel >= 0) output += "<h3>" + metric_name + " spectrum  for channel # " + std::to_string(channel);
    output += "<h3>" + metric_name + " spectrum";
    if(metric_details!="") output += " " + metric_details; 
    output += ".</h3>\n";
    output += "<table>\n";
    output += "<tr>\n";
    output += "<td> <img src=\"" + plot_location_histo + "\" width=\"800\"></td>\n"; 
    output += "<td> <img src=\"" + plot_location_histo2d + "\" width=\"800\"></td>\n"; 
    output += "</tr>\n";
    output += "</table>\n";

    return output;

  }

  
  static std::string MakeGraphPair(std::string plot_location, NearlinePlotInfo plot_info, bool rms){

    std::string metric_name = plot_info.fMetricName;
    int channel = plot_info.fChannel;
    std::string metric_details = plot_info.fMetricDetails;

    std::string plot_location_zoom;
    std::string plot_location_unzoom;
    if(rms){
      plot_location_zoom = plot_location + "/" + plot_info.GetMetricRmsTimeGraphName(true);
      plot_location_unzoom = plot_location + "/" + plot_info.GetMetricRmsTimeGraphName();
    }
    else{
      plot_location_zoom = plot_location + "/" + plot_info.GetMetricMeanTimeGraphName(true);
      plot_location_unzoom = plot_location + "/" + plot_info.GetMetricMeanTimeGraphName();
    }
    
    std::string output;
    output+="<h3>";
    if(rms) output += "RMS of the ";
    else output += "Mean of the ";
    if(channel >= 0) output += metric_name + " spectra for channel # " + std::to_string(channel);
    else output += metric_name + " spectra";
    if(metric_details!="") output += " " + metric_details;
    output +=".</h3>\n";
    output +="<table>\n";
    output +="<tr>\n";
    output +="<td> <img src=\"" + plot_location_unzoom + "\" width=\"800\"></td>\n";
    output +="<td> <img src=\"" + plot_location_zoom + "\" width=\"800\"></td>\n";
    output +="</tr>\n";
    output +="</table>\n";

    return output;
    
  }

  static std::string MakePlotSet(std::string plot_location, NearlinePlotInfo plot_info, NearlinePlotEnables plot_enables){

    std::string metric_name = plot_info.fMetricName;
    std::string output;
    output += "\n\n\n";
    /* output += MakeHistogram(plot_location, plot_info); */
    /* output += "\n"; */
    /* output += MakeHistogram2D(plot_location, plot_info); */
    /* output += "\n"; */

    if(plot_enables.fMake2DHisto) output += MakeHistogramPair(plot_location, plot_info) + "\n";
    else output += MakeHistogram(plot_location, plot_info) + "\n";
    //    output += "\n";
    if(plot_enables.fMakeMetricTimeGraph) output += MakeGraphPair(plot_location, plot_info, false) + "\n";
    //    output += "\n";
    if(plot_enables.fMakeMetricTimeGraph) output += MakeGraphPair(plot_location, plot_info, true) +"\n";
    output += "\n\n";
    output += "<p>\n";
    output += "First plot: " + metric_name + " spectra on a specified channel in the specified time period.\n";
    output += "Second plot: Mean of the top plot over time (each blue dot corresponds to one subrun).\n";
    output += "Third plot: RMS of the top plot over time (each blue dot corresponds to one subrun).\n";
    output += "</p>\n";
    output += "<BR><BR><BR>\n\n";

    return output;

  }

  static std::string MakePageHeader(int Ndays){

    std::string output;
    output+="<head>\n";
    output+="<meta http-equiv=\"refresh\" content=\"60\">\n";
    output+="</head>\n\n\n";
    if(Ndays<=2) output+="<h1>35T Nearline Monitoring - 24 Hour Plots</h1>\n\n";
    else if(Ndays==7) output+="<h1>35T Nearline Monitoring - 7 Days Plots</h1>\n\n";
    else if(Ndays==31) output+="<h1>35T Nearline Monitoring - 31 Days Plots</h1>\n\n";
    else output+="<h1>35T Nearline Monitoring - " + std::to_string(Ndays) + " Days Plots</h1>\n\n";
    output+="<h2><b>THIS PAGE IS CURRENTLY UNDER CONSTRUCTION.</b></h2>\n";
    output+="<BR><BR><BR>\n";

    return output;
  }

};



#endif
