#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <filesystem>


#include <limits>
#include <TFile.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include "dataclasses.h"

int main(int argc, char *argv[]) {

    std::string directory = argv[1]; //'/home/peter4minus1/runs'
    int minrun = 0;
    int maxrun = 30000;
    std::string testfor = "QDC";
    std::string calculation = "mean";
    char *a;
    if (argc >= 3) {
        a = argv[2];
        minrun = atoi(a); //minimum run number
    }
    if (argc >= 4) {
        a = argv[3];
        maxrun = atoi(a); //maximum run number
    }
    if (argc >= 5) {
        testfor = argv[4]; //Data type i.e. QDC
    }
    if (argc >= 6) {
        calculation = argv[5]; //calculation type i.e. mean, ratio
    }


    std::vector<int> runs = {};//{14495,14497,14513,14516,14520,14530,14535,14539,14547,14550,14555,14560,14564,14565,14566};

    for (int r = minrun; r <= maxrun; r++) {
        //std::cout << r;
        std::string filename = directory + Form("/run%d_SPS_monitor.root", r);
        TFile f(filename.c_str());
        if (!(f.IsZombie())) {
            runs.push_back(r);   
            std::cout << r << std::endl;
        }
    }
    
    std::vector<std::string> detectors = {"SPSLF", "SPSRF", "SPSLR", "SPSRR"};
    std::string detector;
    int bars;
    const int n = runs.size();

    //QDC means graph
    Run current_run = Run(runs[0]);
    auto c1 = new TCanvas("c1", "QDC Mean Values", 1200, 2400);

    
    

    for (int d = 0; d<4; d++) {
        detector = detectors[d];
        const char* ds = detector.data();
        if (detector == "SPSLF" | detector == "SPSRF") {bars = 18; c1->Divide(3,6);}
        else {bars = 28;c1->Divide(4,7);} 

        const int n = runs.size();
        float_t x[n];
        float_t up[n];
        float_t up_err[n];
        float_t down[n];
        float_t down_err[n];

        std::vector<float> errs = {};

        if (calculation == "mean") {
        for (int b = 0; b < bars; b++) {
            for (int i = 0; i < runs.size(); i++) {
                current_run = Run(runs[i]);
                current_run.QDC::set_data(runs[i], directory, detector);
                x[i] = runs[i];
                up[i] = current_run.up.get_means()[b];
                up_err[i] = current_run.up.get_errors()[b];
                down[i] = current_run.down.get_means()[b];
                down_err[i] = current_run.down.get_errors()[b];
            }
        auto mg = new TMultiGraph();
        auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
        gr1->SetName("up");
        gr1->SetLineColor(kRed);
        gr1->SetLineWidth(1);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21);
        mg->Add(gr1);

        auto gr2 = new TGraphErrors(n, x, down, nullptr, down_err);
        gr2->SetName("down");
        gr2->SetLineColorAlpha(kBlue, 0.5);
        gr2->SetMarkerSize(.5);
        gr2->SetMarkerStyle(21);
        mg->Add(gr2);
        mg->SetTitle(Form("QDC %s Bar %d;Run;Mean QDC", ds, b));
        c1->cd(b+1); mg->Draw("ALP");

        auto leg = new TLegend(0.15,0.75,0.3,0.85);
        leg->AddEntry("up","Up","l");
        leg->AddEntry("down","Down","l");
        leg->Draw();
        }
        } else if (calculation == "ratio") {
        for (int b = 0; b < bars; b++) {
            for (int i = 0; i < runs.size(); i++) {
                current_run = Run(runs[i]);
                current_run.QDC::set_data(runs[i], directory, detector);
                current_run.QDC::set_ratios();
                x[i] = runs[i];
                up[i] = current_run.QDC::get_ratios()[b];
                up_err[i] = current_run.QDC::std_err('r')[b];
            }
        auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21); 
        gr1->SetTitle(Form("QDC Up/Down Ratio %s Bar %d;Run;QDC Ratio", ds, b));
        c1->cd(b+1); gr1->Draw("ALP");


        float sum=0;
        for (int i = 0; i < runs.size(); i++) {
            sum = sum + up[i];
        }
        float mean = sum/bars;
        float Sr = 0;
        for (int i = 0; i < runs.size(); i++) {
            Sr = Sr + pow(mean-up[i], 2);
        }
        float stddev = pow(Sr/bars, 0.5);
        float err = stddev/pow(bars, 0.5);
        errs.push_back(err);
        }
        }
    c1->SaveAs(Form("%s%ss.pdf", ds, calculation.c_str()));
    c1->Clear();
    for (int i = 0; i<errs.size(); i++) {
        std::cout << errs[i];
    }
    }
c1->Close();




}