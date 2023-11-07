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

void Data::set_means(std::vector<float> a) {means = a;}
void Data::set_errors(std::vector<float> a) {errors = a;}
//void Data::set_maxbins(std::vector<float> a) {maxbins = a;}
std::vector<float> Data::get_means() {return means;}
std::vector<float> Data::get_errors() {return errors;}
//std::vector<float> Data::get_maxbins() {return maxbins;}
int Data::size() {return means.size();}

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

std::vector<float> Gain::get_R() {return R_values;}
void Gain::set_R(std::vector<float> a) {R_values = a;}


void QDC::set_data(int run, std::string directory, std::string detector) {
    std::string file_name = directory + Form("/run%d_SPS_monitor.root", run);
    TFile* datafile = new TFile(file_name.c_str(), "READ");
    
    std::vector<float> u = {};
    std::vector<float> d = {};
    std::vector<float> u_err = {};
    std::vector<float> d_err = {};
    std::vector<float> R_att = {};
    //std::vector<float> u_maxbin = {};
    //std::vector<float> d_maxbin = {};
	    
    double x = 0;
    int bars;
    const char * sps = detector.data();
    if (detector == "SPSLF" | detector == "SPSRF") {bars = 18;}
    else {bars = 28;} 
    for (int b = 0; b < bars; b++) {
    	TH1F* hist_up = (TH1F*)datafile->Get(Form("/%s/Bar%02d/QDC/qdc_trig_%s%02dup",sps, b, sps, b));
    	TH1F* hist_down = (TH1F*)datafile->Get(Form("/%s/Bar%02d/QDC/qdc_trig_%s%02ddown", sps, b, sps, b));
    	 	
    	u.push_back(hist_up->GetMean());
    	d.push_back(hist_down->GetMean());
    		
    	u_err.push_back(hist_up->GetMean(11));
    	d_err.push_back(hist_down->GetMean(11));
    	    	
    	//u_maxbin.push_back(hist_up->GetMaximumBin())
    	//d_maxbin.push_back(hist_down->GetMaximumBin()) 

        
        TProfile* attenuation_graph = (TProfile*)datafile->Get(Form("/%s/Bar%02d/Gain/profile_ratio_%s%02d", sps, b, sps, b));
        TF1* g = (TF1*)attenuation_graph->GetListOfFunctions()->FindObject("pol1");
        double R;
        if (g == nullptr) {
            R = 1.0;
        } else {
            double r = g->Eval(60.0);
            R = exp(r)-1;
        }

        R_att.push_back(R);
        
    //plot nullptrs and above max in different colors
    //do the fit manually
    }
    
    datafile->Close();
    delete datafile;
    	
    up.set_means(u);
    down.set_means(d);
    up.set_errors(u_err);
    down.set_errors(d_err);
    set_R(R_att);
}



Run::Run(int num) {
    set_run(num);
}
void Run::set_run(int n) {run_num = n;}
int Run::get_number() {return run_num;}



void convert(std::vector<float> a, float_t *b, int n)
{
    for (int i=0; i<n; i++) {
        b[i] = a[i];
    }
}
