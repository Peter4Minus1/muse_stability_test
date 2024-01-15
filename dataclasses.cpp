#include <iostream>
#include <cmath>
#include <vector>


#include <limits>
#include <TFile.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TProfile.h>
#include "dataclasses.h"

//------------------HIST-------------------------//

void Hist::set_means(std::vector<float> a) {means = a;}
void Hist::set_errors(std::vector<float> a) {errors = a;}

std::vector<float> Hist::get_means() {return means;}
std::vector<float> Hist::get_errors() {return errors;}

int Hist::size() {return means.size();}

//------------------UPDOWN-------------------------//

void UpDown::set_ratios() {
    ratios = {};
    for (int i=0; i<up.size(); i++) {ratios.push_back(up.get_means()[i]/down.get_means()[i]);}
    }
void UpDown::set_products() {
    products = {};
    for (int i=0; i<up.size(); i++) {products.push_back(up.get_means()[i]*down.get_means()[i]);}
    }

std::vector<float> UpDown::get_ratios() {return ratios;}
std::vector<float> UpDown::get_products() {return products;}

std::vector<float> UpDown::std_err(char type) {
	std::vector<float> sd = {};
	std::vector<float> a = up.get_means();
	std::vector<float> b = down.get_means();
	std::vector<float> a_err = up.get_errors();
	std::vector<float> b_err = down.get_errors();
	
	float f;
	float Of;
	for (int i = 0; i<a.size(); i++) {
	    if (type == 'r') {f = a[i]/b[i];} 
	    else if (type == 'p') {f = a[i]*b[i];}
	    Of = f*sqrt(pow(a_err[i],2)/pow(a[i],2)+pow(b_err[i],2)/pow(b[i],2));
	    sd.push_back(Of);
	}
	return sd;
}

//------------------PROFILE-------------------------//

std::vector<float> Profile::get_R() {return R_values;}
void Profile::set_R(std::vector<float> a) {R_values = a;}

//------------------DETECTOR-------------------------//

Detector::Detector(std::string name, bool front) {
    this->name = name;
    this->front = front;
}

Detector::isFront(){return front;}
Detector::get_name(){return name;}

//------------------RUN-------------------------//

Run::Run(int run_num) {
    this->run_num = run_num;
    SPSLF = new Detector("SPSLF", true);
    SPSRF = new Detector("SPSRF", true);
    SPSLR = new Detector("SPSLR", false);
    SPSRR = new Detector("SPSRR", false);
    SPS = {SPSLF, SPSRF, SPSLR, SPSRR};

}

void Run::set_data(std::string directory) {
    std::string file_name = directory + Form("/run%d.root", run_num);
    TFile* datafile = new TFile(file_name.c_str(), "READ");

    for(int d = 0; d < SPS.size(); d++) {

        Detector detector = SPS[d];

        std::vector<float> u = {};
        std::vector<float> d = {};
        std::vector<float> u_err = {};
        std::vector<float> d_err = {};
        std::vector<float> R_att = {};

        int length;
        const char* name_ptr = detector.get_name().data();

        if (detector.isFront()) {
            length = 18;
        } else {
            length = 28;
        } 

        //QDC

        for (int bar = 0; bar < length; bar++) {
            TH1F* hist_up = (TH1F*)datafile->Get(Form("/%s/Bar%02d/QDC/qdc_trig_%s%02dup", name_ptr, bar, name_ptr, bar));
            TH1F* hist_down = (TH1F*)datafile->Get(Form("/%s/Bar%02d/QDC/qdc_trig_%s%02ddown", name_ptr, bar, name_ptr, bar));
                
            u.push_back(hist_up->GetMean());
            d.push_back(hist_down->GetMean());
                
            u_err.push_back(hist_up->GetMean(11));
            d_err.push_back(hist_down->GetMean(11));
        }

        detector.qdc.trig.up.set_means(u);
        detector.qdc.trig.down.set_means(d);
        detector.qdc.trig.up.set_errors(u_err);
        detector.qdc.trig.down.set_errors(d_err);

        detector.qdc.trig.set_ratios();
        detector.qdc.trig.set_products();

        //Gain

        for (int bar = 0; bar < length; bar++) {
            TProfile* attenuation_graph = (TProfile*)datafile->Get(Form("/%s/Bar%02d/Gain/profile_ratio_%s%02d", name_ptr, bar, name_ptr, bar));
            TF1* g = (TF1*)attenuation_graph->GetListOfFunctions()->FindObject("pol1");
            double R;
            if (g == nullptr) {
                R = 1.0;
            } else {
                double r;
                if (detector.isFront()){
                    r = g->Eval(60.0);
                } else {
                    r = g->Eval(110.0);
                }
                R = exp(r)-1;
            }
            
            R_att.push_back(R);
        }

        detector.gain.profile_ratio.set_R(R_att);
    }
   
    datafile->Close();
    delete datafile;
}

void Run::set_run(int n) {run_num = n;}
int Run::get_number() {return run_num;}

//-------------------MISC----------------------//

void convert(std::vector<float> a, float_t *b, int n)
{
    for (int i=0; i<n; i++) {
        b[i] = a[i];
    }
}
