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


    std::vector<Run> runs = {};

    for (int r = minrun; r <= maxrun; r++) {
        //std::cout << r;
        std::string filename = directory + Form("/run%d.root", r);
        TFile f(filename.c_str());
        if (!(f.IsZombie())) {
            runs.push_back(Run(r));
        } 
        f.Close();
    }
    
    for (int i = 0; i < runs.size(); i++){
        runs[i].set_data();
    }
    const int n = runs.size();

    //QDC means graph

    std::ofstream errorLog;

    std::cout << "\nRoot file about to create\n";
    std::unique_ptr<TFile> AllGraphs( TFile::Open("StabilityTest.root", "RECREATE") );
    std::cout << "Root File created";
    errorLog.open("ErrorLog.csv");
    errorLog << "Detector,Run,Bar,Data,Error Type\n";
 
    Run this_run = runs[0];


    for (int d = 0; d<4; d++) {

        const char* ds = this_run.SPS[d].get_name().data();
        auto detector_folder = AllGraphs->mkdir(ds,ds);

        const int n = runs.size();

        std::vector<float> errs = {};

        UpDown this_qdc = this_run.SPS[d].qdc;

        int bars;
        if this_run.SPS[d].isFront(){
            bars = 18;
        } else {
            bars = 28
        }

        for (int b = 0; b < bars; b++) {

            float_t x[n];

            float_t up[n];
            float_t up_err[n];

            float_t down[n];
            float_t down_err[n];

            const char* bar_str = Form("Bar%02d", b).data();
            auto bar_folder = detector_folder->mkdir(bar_str, bar_str);

            for (int i = 0; i < runs.size(); i++) {
                this_run = Run(runs[i]);

                x[i] = this_run.get_number();

                up[i] = this_qdc.trig.up.get_means()[b];
                up_err[i] = this_qdc.trig.up.get_errors()[b];

                down[i] = this_qdc.trig.down.get_means()[b];
                down_err[i] = this_qdc.trig.down.get_errors()[b];

                
            }

            //-------------------UP AND DOWN QDC MEANS--------------------//
            
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

            auto qdc_folder = bar_folder->mkdir("QDC", "QDC");
            qdc_folder->WriteObject(gr1, "Up" );
            qdc_folder->WriteObject(gr2, "Down");
        
        //--------------------------QDC UP/DOWN RATIOS---------------------//

            float_t ratios[n];
            float_t ratio_err[n];

            for (int i = 0; i < runs.size(); i++) {
                this_run = runs[i];
                x[i] = this_run.get_number();
                ratios[i] = this_qdc.trig.get_ratios()[b];
                ratio_err[i] = this_qdc.trig.std_err('r')[b];
            }
            auto gr1 = new TGraphErrors(n, x, ratios, nullptr, ratio_err);
            gr1->SetMarkerSize(.5);
            gr1->SetMarkerStyle(21); 
            gr1->SetTitle(Form("QDC Up/Down Ratio %s Bar %d;Run;QDC Ratio", ds, b));

            qdc_folder->WriteObject(gr1, Form("ratios", b));

        //---------------------------------GAIN R ATTENUATION-----------------------------//

            auto gain_folder = bar_folder->mkdir("Gain", "Gain");

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


        //Sort good runs from the bad runs
            float_t x[n];
            float_t R[n];
            std::vector<float> bad_runs = {};
            std::vector<float> bad_data = {};
            std::vector<float> null_data = {};
            std::vector<float> null_runs = {};
            for (int i = 0; i < runs.size(); i++){
                this_run = runs[i];
                float Rx = this_run.get_R()[b];
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

            //-------------------R ATTENUATION------------------------------///
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

            gain_folder->WriteObject(c, "Regular");
            c->Close();
            gr1->Clear();
            gr2->Clear();

            //----------------------------------CENTERED R ATTENUATION------------------------------//

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

            gain_folder->WriteObject(c, "Mean_Centered");
            

            c->Close();
        }
    
    //c1->SaveAs(Form("%s%ss.pdf", ds, calculation.c_str()));
    c1->Clear();
    }
c1->Close();
errorLog.close();

//std::unique_ptr<TFile> myFile( TFile::Open("file.root", "RECREATE") );


}