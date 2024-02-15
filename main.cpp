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
#include <TMatrixD.h>
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
        runs[i].set_data(directory);
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

        const char* ds = this_run.SPS[d]->get_name().data();
        auto detector_folder = AllGraphs->mkdir(ds,ds);

        const int n = runs.size();

        //QDC this_qdc = this_run.SPS[d].qdc;
        //Gain this_gain = this_run.SPS[d].gain;

        int bars;
        if (this_run.SPS[d]->isFront()){
            bars = 18;
        } else {
            bars = 28;
        }

        float_t x[n];

        for (int b = 0; b < bars; b++) {

            x[n];

            float_t up[n];
            float_t up_err[n];

            float_t down[n];
            float_t down_err[n];

            const char* bar_str = Form("Bar%02d", b);
            auto bar_folder = detector_folder->mkdir(bar_str, bar_str);

            for (int i = 0; i < runs.size(); i++) {
                this_run = Run(runs[i]);

                x[i] = this_run.get_number();

                up[i] = this_run.SPS[d]->qdc.trig.up.get_means()[b];
                up_err[i] = this_run.SPS[d]->qdc.trig.up.get_errors()[b];

                down[i] = this_run.SPS[d]->qdc.trig.down.get_means()[b];
                down_err[i] = this_run.SPS[d]->qdc.trig.down.get_errors()[b];
            }
            

            //-------------------UP AND DOWN QDC MEANS--------------------//
            
            auto mg = new TMultiGraph();
            auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
            gr1->SetName("up");
            gr1->SetTitle(Form("Mean QDC Up %s Bar %02d;Run;QDC", ds, b));
            gr1->SetLineColor(kRed);
            gr1->SetLineWidth(1);
            gr1->SetMarkerSize(.5);
            gr1->SetMarkerStyle(21);
            gr1->SetMarkerColor(kRed);

            auto gr2 = new TGraphErrors(n, x, down, nullptr, down_err);
            gr2->SetName("down");
            gr2->SetTitle(Form("Mean QDC Down %s Bar %02d;Run;QDC", ds, b));
            gr2->SetLineColor(kBlue);
            gr2->SetMarkerSize(.5);
            gr2->SetMarkerStyle(21);
            gr2->SetMarkerColor(kBlue);

            auto qdc_folder = bar_folder->mkdir("QDC", "QDC");
            qdc_folder->WriteObject(gr1, "Up" );
            qdc_folder->WriteObject(gr2, "Down");
        
        //--------------------------QDC UP/DOWN RATIOS---------------------//

            float_t ratios[n];
            float_t ratio_err[n];

            for (int i = 0; i < runs.size(); i++) {
                this_run = runs[i];
                x[i] = this_run.get_number();
                if (this_run.SPS[d]->qdc.trig.down.get_means()[b] != 0){
                    ratios[i] = this_run.SPS[d]->qdc.trig.get_ratios()[b];
                    ratio_err[i] = this_run.SPS[d]->qdc.trig.std_err('r')[b];
                }
            }
            gr1 = new TGraphErrors(n, x, ratios, nullptr, ratio_err);
            gr1->SetMarkerSize(.5);
            gr1->SetMarkerStyle(21); 
            gr1->SetTitle(Form("QDC Up/Down Ratio %s Bar %02d;Run;QDC Ratio", ds, b));

            qdc_folder->WriteObject(gr1, "ratios");

        //---------------------------------GAIN R ATTENUATION-----------------------------//

            auto gain_folder = bar_folder->mkdir("Gain", "Gain");    

        //Sort good runs from the bad runs
            std::vector<float> good_runs= {};
            std::vector<float> good_data = {};
            std::vector<float> bad_runs = {};
            std::vector<float> bad_data = {};
            std::vector<float> null_data = {};
            std::vector<float> null_runs = {};
            for (int i = 0; i < runs.size(); i++){
                this_run = runs[i];
                float Rx = this_run.SPS[d]->gain.profile_ratio.get_R()[b];
                if (Rx == 1.0){
                    null_data.push_back(0.0);
                    null_runs.push_back(this_run.get_number());
                    errorLog << Form("%s,%d,%02d,R attenuation,returns NULL\n", ds, this_run.get_number(),b);
                } else {
                    
                    if (abs(Rx) > .1){
                        bad_data.push_back(0.1 * Rx/abs(Rx));
                        bad_runs.push_back(this_run.get_number());
                        errorLog << Form("%s,%d,%02d,R attenuation,|R| > 10%%\n", ds, this_run.get_number(),b);
                    }
                    good_data.push_back(this_run.SPS[d]->gain.profile_ratio.get_R()[b]);
                    good_runs.push_back(this_run.get_number());
                }
            }
            const int n1 = good_data.size();
            float_t x_valid[n1];
            float_t R[n1];
            const int n2 = bad_data.size();
            float_t x2[n2];
            float_t y2[n2];
            const int n3 = null_data.size();
            float_t x3[n3];
            float_t y3[n3];

            for (int i = 0; i < n1; i++) {
                x_valid[i] = good_runs[i];
                R[i] = good_data[i];
            }
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
            mg = new TMultiGraph();
            mg->SetTitle(Form("Gain Attenuation R Values %s Bar %d;Run;R value", ds, b));

            gr2 = new TGraphErrors(n2, x2, y2, nullptr, nullptr);
            gr2->SetMarkerSize(.5);
            gr2->SetMarkerStyle(21);
            gr2->SetMarkerColor(kBlue);
            mg->Add(gr2);

            
            auto gr3 = new TGraphErrors(n3, x3, y3, nullptr, nullptr);
            gr3->SetMarkerSize(.5);
            gr3->SetMarkerStyle(21);
            gr3->SetMarkerColor(kRed);
            mg->Add(gr3);
            

            gr1 = new TGraphErrors(n1, x_valid, R, nullptr, nullptr);
            gr1->SetMarkerSize(.5);
            gr1->SetMarkerStyle(21); 
            gr1->SetTitle(Form("Gain Attenuation R Values %s Bar %d;Run;R value", ds, b));
            gain_folder->WriteObject(gr1, "Raw R Values");
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
            for (int i = 0; i < n1;i++){
                if (typeid(R[i]) != typeid(nullptr)){
                    sum = sum + R[i];
                    count++;
                }
            }
            float mean = sum/count;

            for (int i=0; i<n1; i++){
                if (typeid(R[i]) != typeid(nullptr)){
                    R[i] = R[i]-mean;
                }
            }

            gr1 = new TGraphErrors(n1, x_valid, R, nullptr, nullptr);
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

        //--------------------PEDESTAL----------------------------//
                
            auto ped_folder = bar_folder->mkdir("Pedestal", "Pedestal");

            float_t up_p[n];
            float_t up_w[n];
            float_t down_p[n];
            float_t down_w[n];

            for (int i = 0; i < n; i++){
                try {
                    up_p[i] = runs[i].SPS[d]->ped.getUpPositions()[b];
                } catch(...) {
                    errorLog << ds << "," << this_run.get_number() << "," << Form("%02d", b) << ",Pedestal,Up Position Invalid\n";
                    std::cout << "Run " << this_run.get_number() << " bar " << b << " Pedestal Up Position invalid";
                }

                try{
                    up_w[i] = runs[i].SPS[d]->ped.getUpWidths()[b];
                } catch(...) {
                    errorLog << ds << "," << this_run.get_number() << "," << Form("%02d", b) << ",Pedestal,Up Width Invalid\n";
                    std::cout << "Run " << this_run.get_number() << " bar " << b << " Pedestal Up width invalid";
                }   

                try {
                    down_p[i] = runs[i].SPS[d]->ped.getDownPositions()[b];
                } catch(...) {
                    errorLog << ds << "," << this_run.get_number() << "," << Form("%02d", b) << ",Pedestal,Down Position Invalid\n";
                    std::cout << "Run " << this_run.get_number() << " bar " << b << " Pedestal Down Position invalid";
                }

                try{
                    down_w[i] = runs[i].SPS[d]->ped.getDownWidths()[b];
                } catch(...) {
                    errorLog << ds << "," << this_run.get_number() << "," << Form("%02d", b) << ",Pedestal,Down Width Invalid\n";
                    std::cout << "Run " << this_run.get_number() << " bar " << b << " Pedestal Down Width invalid";
                }
                x[i] = runs[i].get_number();
            }

            auto grA = new TGraphErrors(n, x, up_p, nullptr, nullptr);
            grA->SetMarkerSize(.5);
            grA->SetMarkerStyle(21);
            grA->SetTitle(Form("QDC Pedestal Position Up %s;Run;QDC", ds));
            ped_folder->WriteObject(grA, "Up Positions");

            auto grB = new TGraphErrors(n, x, up_w, nullptr, nullptr);
            grB->SetMarkerSize(.5);
            grB->SetMarkerStyle(21);
            grB->SetTitle(Form("QDC Pedestal Width Up %s;Run;QDC", ds));
            ped_folder->WriteObject(grB, "Up Widths");

            auto grC = new TGraphErrors(n, x, down_p, nullptr, nullptr);
            grC->SetMarkerSize(.5);
            grC->SetMarkerStyle(21);
            grC->SetTitle(Form("QDC Pedestal Position Up %s;Run;QDC", ds));
            ped_folder->WriteObject(grC, "Down Positions");

            auto grD = new TGraphErrors(n, x, down_w, nullptr, nullptr);
            grD->SetMarkerSize(.5);
            grD->SetMarkerStyle(21);
            grD->SetTitle(Form("QDC Pedestal Width Up %s;Run;QDC", ds));
            ped_folder->WriteObject(grD, "Down Widths");
            
            c->Close();
        }
    
    //c1->SaveAs(Form("%s%ss.pdf", ds, calculation.c_str()));
    }
errorLog.close();

//std::unique_ptr<TFile> myFile( TFile::Open("file.root", "RECREATE") );


}