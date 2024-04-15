#include <TCanvas.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TMarker.h>
#include <iostream>
#include <string>
#include <vector>

struct channel_id {
    std::string wall_id;
    std::string bar_id;
    int col;
};

void plot_qdc_up(std::string id, TFile* f, int run_min, int run_max, double y_min, double y_max, std::vector<channel_id> bars)
{
    std::string can_id = Form("plot_ratio_%s", id.c_str());
    TCanvas* can = new TCanvas(can_id.c_str(), can_id.c_str(), 600, 400);
    can->SetRightMargin(0.10);

    // Draw the axis

    TH1F* hr = gPad->DrawFrame(run_min, y_min, run_max, y_max);
    hr->SetXTitle("Run Number");
    hr->SetYTitle("Lightoutput");
    hr->GetXaxis()->CenterTitle(-1);
    hr->GetYaxis()->CenterTitle(-1);
    hr->GetXaxis()->SetTickLength(0.035);
    hr->GetYaxis()->SetTickLength(0.02);
    hr->SetNdivisions(504, "x");
    hr->SetNdivisions(504, "y");
    hr->SetLineStyle(1);
    hr->SetTitle("");
    hr->Draw("");

    // Draw the graphs

    for (auto b : bars) {
        if (f) {
            TCanvas* c = (TCanvas*)f->Get(Form("/%s/%s/QDC/up", b.wall_id.c_str(), b.bar_id.c_str()));
            if (c) {
                TGraphErrors* gr = (TGraphErrors*)c->FindObject("Graph");
                gr->SetLineColor(b.col);
                gr->SetMarkerColor(b.col);
                can->cd();
                gr->Draw("*");
            }
        }
    }
    can->SaveAs(Form("poster_ratio_%s.pdf", id.c_str()));
}

void plot_ratio(std::string id, TFile* f, int run_min, int run_max, double y_min, double y_max, std::vector<channel_id> bars)
{
    std::string can_id = Form("plot_ratio_%s", id.c_str());
    TCanvas* can = new TCanvas(can_id.c_str(), can_id.c_str(), 600, 400);
    can->SetRightMargin(0.10);

    // Draw the axis

    TH1F* hr = gPad->DrawFrame(run_min, y_min, run_max, y_max);
    hr->SetXTitle("Run Number");
    hr->SetYTitle("Up/Down Lightoutput Ratio");
    hr->GetXaxis()->CenterTitle(-1);
    hr->GetYaxis()->CenterTitle(-1);
    hr->GetXaxis()->SetTickLength(0.035);
    hr->GetYaxis()->SetTickLength(0.02);
    hr->SetNdivisions(504, "x");
    hr->SetNdivisions(504, "y");
    hr->SetLineStyle(1);
    hr->SetTitle("");
    hr->Draw("");

    // Draw the graphs

    for (auto b : bars) {
        if (f) {
            TCanvas* c = (TCanvas*)f->Get(Form("/%s/%s/QDC/ratios", b.wall_id.c_str(), b.bar_id.c_str()));
            if (c) {
                TGraphErrors* gr = (TGraphErrors*)c->FindObject("Graph");
                gr->SetLineColor(b.col);
                gr->SetMarkerColor(b.col);
                can->cd();
                gr->Draw("*");
            }
        }
    }
    can->SaveAs(Form("poster_ratio_%s.pdf", id.c_str()));
}

void plot_r_value(std::string id, TFile* f, int run_min, int run_max, double y_min, double y_max, std::vector<channel_id> bars)
{
    std::string can_id = Form("plot_r_value_%s", id.c_str());
    TCanvas* can = new TCanvas(can_id.c_str(), can_id.c_str(), 600, 400);
    can->SetRightMargin(0.10);

    // Draw the axis

    TH1F* hr = gPad->DrawFrame(run_min, y_min, run_max, y_max);
    hr->SetXTitle("Run Number");
    hr->SetYTitle("R Value");
    hr->GetXaxis()->CenterTitle(-1);
    hr->GetYaxis()->CenterTitle(-1);
    hr->GetXaxis()->SetTickLength(0.035);
    hr->GetYaxis()->SetTickLength(0.02);
    hr->SetNdivisions(504, "x");
    hr->SetNdivisions(504, "y");
    hr->SetLineStyle(1);
    hr->SetTitle("");
    hr->Draw("hist");

    // Draw the graphs

    for (auto b : bars) {
        if (f) {
            TCanvas* c = nullptr;
            c = (TCanvas*)f->Get(Form("/%s/%s/Gain/Raw_R_Values", b.wall_id.c_str(), b.bar_id.c_str()));
            if (c) {
                TGraphErrors* gr = (TGraphErrors*)c->FindObject("Graph");
                gr->SetLineColor(b.col);
                gr->SetMarkerColor(b.col);
                can->cd();
                gr->Draw("*");
            }
        }
    }
    can->SaveAs(Form("poster_r_value_%s.pdf", id.c_str()));
}

void poster_plots()
{
    TFile* f = new TFile("../build/StabilityTest.root");

    plot_ratio("up", f, 18860, 19080, 0.0, 800, {{"SPSRF", "Bar02", kRed + 1}, {"SPSRF", "Bar03", kBlue}, {"SPSRF", "Bar04", kGreen +1}});

    // various ratio plots

    plot_ratio("full", f, 17600, 21000, 0.0, 2.0, { { "SPSRR", "Bar00", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 } });
    plot_ratio("zoom1", f, 20000, 20500, 0.5, 1.5, { { "SPSRR", "Bar00", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 }, { "SPSLR", "Bar00", kGreen + 1 } });
    plot_ratio("zoom2", f, 20280, 20500, 0.92, 1.08, { { "SPSLR", "Bar01", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 }, { "SPSLR", "Bar00", kGreen + 1 } });

    // various r-vaue plots

    plot_r_value("full", f, 17600, 21000, -0.2, 0.2, { { "SPSRR", "Bar00", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 } });
    plot_r_value("zoom1", f, 20000, 20500, -0.2, 0.2, { { "SPSRR", "Bar00", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 }, { "SPSLR", "Bar00", kGreen + 1 } });
    plot_r_value("zoom2", f, 20280, 20500, -0.2, 0.2, { { "SPSLR", "Bar01", kRed + 1 }, { "SPSRR", "Bar01", kBlue + 1 }, { "SPSLR", "Bar00", kGreen + 1 } });

    // other plots

    // plot_pedestal(...)
}
