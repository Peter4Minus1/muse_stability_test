#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <filesystem>
#include <fstream>
#include <typeinfo>


#include <limits>
#include <TFile.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TProfile.h>
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


    std::vector<Run> runs = {};//{14495,14497,14513,14516,14520,14530,14535,14539,14547,14550,14555,14560,14564,14565,14566};

    for (int r = minrun; r <= maxrun; r++) {
        //std::cout << r;
        std::string filename = directory + Form("/run%d.root", r);
        TFile f(filename.c_str());
        if (!(f.IsZombie())) {
            runs.push_back(Run(r));
        } 
        f.Close();
    }
    
    std::vector<std::string> detectors = {"SPSLF", "SPSRF", "SPSLR", "SPSRR"};
    std::string detector;
    int bars;
    const int n = runs.size();

    //QDC means graph
    Run current_run = runs[0];
    auto c1 = new TCanvas("c1", "QDC Mean Values", 1200, 2400);

    std::ofstream errorLog;

    std::cout << "Root file about to create";
    std::unique_ptr<TFile> AllGraphs( TFile::Open("StabilityTest.root", "RECREATE") );
    std::cout << "Root File created";
    for (int d = 0; d<4; d++) {
        detector = detectors[d];
        const char* ds = detector.data();
        if (detector == "SPSLF" | detector == "SPSRF") {bars = 18; c1->Divide(3,6);}
        else {bars = 28;c1->Divide(4,7);} 

        auto detector_folder = AllGraphs->mkdir(ds,ds);
        
        errorLog.open("ErrorLog.csv");
        errorLog << "Detector,Run,Bar,Data,Error Type\n";

        const int n = runs.size();
        float_t x[n];
        float_t up[n];
        float_t up_err[n];
        float_t down[n];
        float_t down_err[n];

        std::vector<float> errs = {};
        
        auto means = detector_folder->mkdir("means", "QDC Historgram Means");

        for (int b = 0; b < bars; b++) {
            for (int i = 0; i < runs.size(); i++) {
                current_run = Run(runs[i]);
                current_run.QDC::set_data(runs[i].get_number(), directory, detector);
                x[i] = runs[i].get_number();
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

        auto gr2 = new TGraphErrors(n, x, down, nullptr, down_err);
        gr2->SetName("down");
        gr2->SetLineColorAlpha(kBlue, 0.5);
        gr2->SetMarkerSize(.5);
        gr2->SetMarkerStyle(21);
        c1->cd(b+1); mg->Draw("AL");

        auto updown = means->mkdir(Form("Bar%02d", b),Form("Bar%02d", b));
        updown->WriteObject(gr1, "Up" );
        updown->WriteObject(gr2, "Down");
        }

        auto ratios = detector_folder->mkdir("ratios", "QDC Up-Down Ratios");

        for (int b = 0; b < bars; b++) {
            for (int i = 0; i < runs.size(); i++) {
                current_run = runs[i];
                current_run.QDC::set_data(runs[i].get_number(), directory, detector);
                current_run.QDC::set_ratios();
                x[i] = runs[i].get_number();
                up[i] = current_run.QDC::get_ratios()[b];
                up_err[i] = current_run.QDC::std_err('r')[b];
            }
        auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21); 
        gr1->SetTitle(Form("QDC Up/Down Ratio %s Bar %d;Run;QDC Ratio", ds, b));
        c1->cd(b+1); gr1->Draw("AL");


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
        
        ratios->WriteObject(gr1, Form("Bar%02d", b));
        }

        auto R_folder = detector_folder->mkdir("R values", "Gain Attenuation R values");
        


        for (int b = 0; b<bars; b++)   {
            auto bar_folder = R_folder->mkdir(Form("Bar%02d", b));
            float_t R[n];
            std::vector<float> bad_runs = {};
            std::vector<float> bad_data = {};
            std::vector<float> null_data = {};
            std::vector<float> null_runs = {};
            for (int i = 0; i < runs.size(); i++){
                current_run = runs[i];
                current_run.QDC::set_data(runs[i].get_number(), directory, detector);
                float Rx = current_run.get_R()[b];
                if (Rx == 1.0){
                    null_data.push_back(0.0);
                    null_runs.push_back(current_run.get_number());
                    errorLog << Form("%s,%d,%02d,R attenuation,returns NULL\n", ds, current_run.get_number(),b);
                } else {
                    
                    if (abs(Rx) > .1){
                        bad_data.push_back(0.1 * Rx/abs(Rx));
                        bad_runs.push_back(current_run.get_number());
                        errorLog << Form("%s,%d,%02d,R attenuation,|R| > 10%%\n", ds, current_run.get_number(),b);
                    }
                    R[i] = current_run.get_R()[b];
                    x[i] = current_run.get_number();
                }
            }
        const int n2 = bad_data.size();
        float_t x2[n2];
        float_t y2[n2];
        const int n3 = null_data.size();
        float_t x3[n3];
        float_t y3[n3];

        for (int i = 0; i < n2; i++) {
            x2[i] = bad_runs[i];

            y2[i] = bad_data[i];
        }
        for (int i = 0; i < n3; i++){
            x3[i] = null_runs[i];
            y3[i] = null_data[i];
        }

        auto c = new TCanvas("c", "R Attenuation values");
        auto mg = new TMultiGraph();
        mg->SetTitle(Form("Gain Attenuation R Values %s Bar %d;Run;R value", ds, b));
        auto gr2 = new TGraphErrors(n2, x2, y2, nullptr, nullptr);
        gr2->SetMarkerSize(.5);
        gr2->SetMarkerStyle(21);
        gr2->SetMarkerColor(kBlue);
        mg->Add(gr2);

        
        auto gr3 = new TGraphErrors(n3, x3, y3, nullptr, nullptr);
        gr3->SetMarkerSize(.5);
        gr3->SetMarkerStyle(21);
        gr3->SetMarkerColor(kRed);
        mg->Add(gr3);
        

        auto gr1 = new TGraphErrors(n, x, R, nullptr, nullptr);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21); 
        gr1->SetTitle(Form("QDC Gain Attenuation R Values %s Bar %d;Run;R value", ds, b));
        mg->Add(gr1);
        mg->SetMinimum(-.11);
        mg->SetMaximum(.11);
        mg->Draw("AP");

        bar_folder->WriteObject(c, "Regular");
        c->Close();
        gr1->Clear();
        gr2->Clear();

        c = new TCanvas("c", "Centered at 0");
        float sum = 0;
        int count = 0;
        for (int i = 0; i < n;i++){
            if (typeid(R[i]) != typeid(nullptr)){
                sum = sum + R[i];
                count++;
            }
        }
        float mean = sum/count;

        for (int i=0; i<n; i++){
            if (typeid(R[i]) != typeid(nullptr)){
                R[i] = R[i]-mean;
            }
        }

        gr1 = new TGraphErrors(n, x, R, nullptr, nullptr);
        gr1->SetMarkerSize(.5);
        gr1->SetMarkerStyle(21);

        mg->Clear();
        mg = new TMultiGraph();
        mg->SetTitle(Form("Gain Attenuation R Values Centered %s Bar %d;Run;ΔR", ds, b));
        mg->Add(gr1);
        mg->Add(gr3);
        mg->SetMinimum(-.11);
        mg->SetMaximum(.11);
        mg->Draw("AP");

        auto leg = new TLegend(0.15,0.75,0.3,0.85);
        leg->AddEntry((TObject*)0, Form("R = %f", mean), "");
        leg->Draw();

        bar_folder->WriteObject(c, "Mean_Centered");
        

        c->Close();
        }
    errorLog.close();
    //c1->SaveAs(Form("%s%ss.pdf", ds, calculation.c_str()));
    c1->Clear();
    }
c1->Close();

//std::unique_ptr<TFile> myFile( TFile::Open("file.root", "RECREATE") );


}