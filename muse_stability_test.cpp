#include <iostream>
#include <string>
#include <cmath>


#include <limits>
#include <TFile.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include "helloworld.h"


void report(std::string title, TH1F* h)
{
    std::cout << title << "\n";

    std::cout << "number of bins: " << h->GetNbinsX() << "\n";
    std::cout << "mean:       " << h->GetMean() << "\n";
    std::cout << "std. dev.:  " << h->GetStdDev() << "\n\n";
}

class Run {
    private:
	int run_number;
	std::vector<float> QDC_mean_up;
	std::vector<float> QDC_mean_down;
	std::vector<float> QDC_ratios;
	std::vector<float> QDC_products;
	
	std::vector<float> QDC_error_up;
	std::vector<float> QDC_error_down;
	
	//std::vector<float> QDC_maxbin_up;
	//std::vector<float> QDC_maxbin_down;
	

	
    public:
    	Run(int num) {
    	    setNumber(num);
    	    get_data();
    	    set_QDC_ratios();
    	    set_QDC_products();
    	    
    	}
    	
    	void setNumber(int num) {run_number = num;}
    	int getNumber() {return run_number;}
    	
    	void get_data() {
    	    std::string file_name = Form("/home/peter4minus1/runs/run%d_SPS_monitor.root", run_number);
   	    TFile* datafile = new TFile(file_name.c_str(), "READ");
    
	    std::vector<float> u = {};
	    std::vector<float> d = {};
	    std::vector<float> u_err = {};
	    std::vector<float> d_err = {};
	    //std::vector<float> u_maxbin = {};
	    //std::vector<float> d_maxbin = {};
	    
	    double x = 0;
    	    for (int b = 0; b < 18; b++) {
    	    	TH1F* hist_up = (TH1F*)datafile->Get(Form("/SPSLF/Bar%02d/QDC/qdc_SPSLF%02dup", b, b));
    	    	TH1F* hist_down = (TH1F*)datafile->Get(Form("/SPSLF/Bar%02d/QDC/qdc_SPSLF%02ddown", b, b));
    	    	
    	    	u.push_back(hist_up->GetMean());
    	    	d.push_back(hist_down->GetMean());
    	    	
    	    	u_err.push_back(hist_up->GetMean(11));
    	    	d_err.push_back(hist_down->GetMean(11));
    	    	
    	    	//u_maxbin.push_back(hist_up->GetMaximumBin())
    	    	//d_maxbin.push_back(hist_down->GetMaximumBin())    	    	
    	    	
    	    }
    	    datafile->Close();
    	    delete datafile;
    	    
    	    QDC_mean_up = u;
    	    QDC_mean_down = d;
    	    QDC_error_up = u_err;
    	    QDC_error_down = d_err;
    	    //QDC_maxbin_up = u_maxbin;
    	    //QDC_maxbin_down = d_maxbin;
	}
	std::vector<float> getQDC_up() {return QDC_mean_up;}
	std::vector<float> getQDC_down() {return QDC_mean_down;}
	std::vector<float> getQDC_error_up() {return QDC_error_up;}
	std::vector<float> getQDC_error_down() {return QDC_error_down;}
	
	void set_QDC_ratios() {
	    std::vector<float> r = {};
	    for (int b = 0; b < 18; b++) {
	    	r.push_back(QDC_mean_up[b] / QDC_mean_down[b]);
	    	}
	    QDC_ratios = r;
	}

	std::vector<float> getQDC_ratios() {return QDC_ratios;}
	    
	void set_QDC_products() {
	    std::vector<float> p = {};
	    for (int b = 0; b < 18; b++) {
	    	p.push_back(QDC_mean_up[b] * QDC_mean_down[b]);
	    	}
	    QDC_products = p;
	}
	std::vector<float> getQDC_products() {return QDC_products;}
	
	std::vector<float> std_dev(char type = 'r') {
	    std::vector<float> sd = {};
	    float f;
	    float Of;
	    
	
	    std::vector<float> a = getQDC_up();
	    std::vector<float> b = getQDC_down();
	    std::vector<float> Oa = getQDC_error_up();
	    std::vector<float> Ob = getQDC_error_down();
	    for (int i = 0; i<a.size(); i++) {
	        if (type == 'r') {f = a[i]/b[i];} 
	        else if (type == 'p') {f = a[i]*b[i];}
	        Of = f*sqrt(pow(Oa[i],2)/pow(a[i],2)+pow(Ob[i],2)/pow(b[i],2));
	        sd.push_back(Of);
	    }
	    return sd;
	}
	};




int main(int argc, char** argv) {
hello();
/*
    std::vector<int> runs = {14495,14497,14513,14516,14520,14530,14535,14539,14547,14550,14555,14560,14564,14565,14566};
    
    const int n = runs.size();
    float_t x[n];
    float_t up[n];
    float_t down[n];
    float_t up_err[n];
    float_t down_err[n];
    Run current_run = Run(runs[0]);
    
    auto c1 = new TCanvas("c1", "QDC Mean Values", 1200, 2400);
    c1->Divide(3,6);
    
    
    for (int b = 0; b < 18; b++) {
    
    for (int i = 0; i < runs.size(); i++) {
    	x[i] = runs[i];
    	current_run = Run(runs[i]);
    	up[i] = current_run.getQDC_up()[b];
    	up_err[i] = current_run.getQDC_error_up()[b];
    	down[i] = current_run.getQDC_down()[b];
    	down_err[i] = current_run.getQDC_error_down()[b];
    	}

    auto mg1 = new TMultiGraph();
    auto gr1 = new TGraphErrors(n, x, up, nullptr, up_err);
    gr1->SetLineColor(kRed);
    gr1->SetMarkerSize(.5);
    gr1->SetMarkerStyle(21);
    mg1->Add(gr1);
    
    
    auto gr2 = new TGraphErrors(n, x, down, nullptr, down_err);
    gr2->SetLineColor(kBlue);
    gr2->SetMarkerSize(.5);
    gr2->SetMarkerStyle(21);
    mg1->Add(gr2);
    
    mg1->SetTitle(Form("QDC SPSLF Bar %d;Run;Mean QDC", b));
    c1->cd(b+1); mg1->Draw("ALP");
    }
    c1->SaveAs("means.pdf");
    c1->Close();
    
    auto c2 = new TCanvas("c1", "QDC Differentials", 1200, 2400);
    c2->Divide(3,6);
    
    float_t up_diffs[n];
    float_t down_diffs[n];
    
    for (int b = 0; b < 18; b++) {
    
    for (int i = 0; i < runs.size(); i++) {
    	current_run = Run(runs[i]);
    	up[i] = current_run.getQDC_up()[b];
    	down[i] = current_run.getQDC_down()[b];
    	}
    

    up_diffs[0] = 0;
    down_diffs[0] = 0;
    for (int i = 1; i<runs.size(); i++) {
    	up_diffs[i] = (up[i]-up[i-1])/(x[i] - x[i-1]);
    	down_diffs[i] = (down[i] - down[i-1])/(x[i] - x[i-1]);
    	up_err[i] = sqrt(pow(current_run.getQDC_error_up()[i],2)+pow(current_run.getQDC_error_up()[i-1],2))/(x[i] - x[i-1]);
    	down_err[i] = sqrt(pow(current_run.getQDC_error_down()[i],2)+pow(current_run.getQDC_error_down()[i-1],2))/(x[i] - x[i-1]);
    	}
    
    auto mg2 = new TMultiGraph();
    auto gr3 = new TGraphErrors(n, x, up_diffs, nullptr, up_err);
    gr3->SetLineColor(kRed);
    gr3->SetMarkerSize(.5);
    gr3->SetMarkerStyle(21);
    mg2->Add(gr3);
    
    
    auto gr4 = new TGraphErrors(n, x, down_diffs, nullptr, down_err);
    gr4->SetLineColor(kBlue);
    gr4->SetMarkerSize(.5);
    gr4->SetMarkerStyle(21);
    mg2->Add(gr4);
    
    mg2->SetTitle(Form("QDC SPSLF Bar %d;Run;QDC Differential", b));
    c2->cd(b+1); mg2->Draw("ALP");
    }
    c2->SaveAs("differentials.pdf");
    c2->Close();
    
    auto c3 = new TCanvas("c1", "QDC Up/Down Ratios", 1200, 2400);
    c3->Divide(3,6);
    
    float_t ratios[n];
    float_t r_err[n];
    float_t products[n];
    float_t p_err[n];
    
    for (int b = 0; b < 18; b++) {
    
    for (int i = 0; i < runs.size(); i++) {
    	current_run = Run(runs[i]);
    	ratios[i] = current_run.getQDC_ratios()[b];
    	r_err[i] = current_run.std_dev('r')[b];
    }
    
    auto gr5 = new TGraphErrors(n, x, ratios, nullptr,r_err);
    gr5->SetMarkerSize(.5);
    gr5->SetMarkerStyle(21);
    gr5->SetTitle(Form("QDC SPSLF Bar %d;Run;QDC Up/Down Ratio", b));
    c3->cd(b+1); gr5->Draw("ALP");
    }
    c3->SaveAs("ratios.pdf");
    c3->Close();
    
    auto c4 = new TCanvas("c1", "QDC Up*Down Products", 1200, 2400);
    c4->Divide(3,6);
    
    for (int b = 0; b < 18; b++) {
    
    for (int i = 0; i < runs.size(); i++) {
    	current_run = Run(runs[i]);
    	products[i] = current_run.getQDC_products()[b];
    	p_err[i] = current_run.std_dev('p')[b];
    }
    auto gr6 = new TGraphErrors(n, x, products, nullptr, p_err);
    gr6->SetMarkerSize(.5);
    gr6->SetMarkerStyle(21);
    gr6->SetTitle(Form("QDC SPSLF Bar %d;Run;QDC Up*Down Product", b));
    c4->cd(b+1); gr6->Draw("ALP");
    }
    c4->SaveAs("products.pdf");
    c4->Close();
  
    exit(EXIT_SUCCESS);*/
}
