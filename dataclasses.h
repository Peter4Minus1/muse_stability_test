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

class Hist {

private:
    std::vector<float> means;
    std::vector<float> errors;
    //std::vector<float> maxbins;
    
public:
    Hist();

    void set_means(std::vector<float> a);
    void set_errors(std::vector<float> a);
    //void set_maxbins(std::vector<float> a);
    
    std::vector<float> get_means();
    std::vector<float> get_errors();
    //std::vector<float> get_maxbins();

    int size();
};

class UpDown {
private:
    std::vector<float> ratios;
    std::vector<float> products;
public:
    Hist up;
    Hist down;
    
    UpDown();

    //Hist _up();
    //Hist _down();
    void set_ratios();
    void set_products();
    
    std::vector<float> get_ratios();
    std::vector<float> get_products();
    
    std::vector<float> std_err(char type);    
};

class Profile {
private:
    std::vector<float> R_values;
public:
    Profile();
    std::vector<float> get_R();
    void set_R(std::vector<float> a);
};


class QDC {
    public:
        //UpDown raw;
        UpDown trig;
        //UpDown QDC_ped;
//    public:
        QDC();
        
        //UpDown _trig();
};

class Gain {
    public:
        Profile profile_ratio;
    //public:
        Gain();
        //Profile _profile_ratio();
};

class Detector{
    private:
        std::string name;
        bool front;
    public:

        Gain gain;
        QDC qdc;
    //public:
        Detector();
        Detector(std::string name, bool front);


        //Gain getGain();
        //QDC getQdc();
        
        bool isFront();
        std::string get_name();
};

class Run {
private:
    int run_num;

public:
    Detector* SPS[4];
    Run(int num);
    void set_run(int n);
    int get_number();
    void set_data(std::string directory);
};

void convert(std::vector<float> v, float_t* a, int n);

