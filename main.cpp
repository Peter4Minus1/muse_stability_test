#include <iostream>
#include <string>
#include <cmath>
#include <vector>


#include <limits>
#include <TFile.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include "dataclasses.h"

int main(int argc, char** argv) {
    std::vector<int> runs = {14495,14497,14513,14516,14520,14530,14535,14539,14547,14550,14555,14560,14564,14565,14566};
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

        for (int b = 0; b < bars; b++) {
            for (int i = 0; i < runs.size(); i++) {
                current_run = Run(runs[i]);
                current_run.QDC::set_data(runs[i], detector);
                x[i] = runs[i];
                up[i] = current_run.up.get_means()[b];
                up_err[i] = current_run.up.get_errors()[b];
                down[i] = current_run.down.get_means()[b];
                down_err[i] = current_run.down.get_errors()[b];
            }
        auto mg = new TMultiGraph();
        auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
        gr1->SetLineColor(kRed);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21);
        mg->Add(gr1);

        auto gr2 = new TGraphErrors(n, x, down, nullptr, down_err);
        gr2->SetLineColor(kBlue);
        gr2->SetMarkerSize(.5);
        gr2->SetMarkerStyle(21);
        mg->Add(gr2);
        mg->SetTitle(Form("QDC %s Bar %d;Run;Mean QDC", ds, b));
        c1->cd(b+1); mg->Draw("ALP");
        }
    c1->SaveAs(Form("%smeans.pdf", ds));
    c1->Clear();
    }
c1->Close();


}