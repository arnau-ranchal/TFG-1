#include "functions.h"
#include <iomanip>
#include "database.h"
#include "database_main.h"
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <string>
#include <algorithm>
#include <fstream>
#include <locale>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

double minRate = 0, maxRate = 1, ptsRate = 1 /*10*/, incrementRate;
double minSNR = .5, maxSNR = 1, ptsSNR = 1, incrementSNR; // todo change
int minmodexp = 1; //1
int maxmodexp = 1; //10
double minMod = pow(2, minmodexp), maxMod = pow(2, maxmodexp), ptsMod = maxmodexp - minmodexp + 1, incrementMod;
int indexRate = 1, indexSNR = 2, indexMod = 3, indexE0 = 4;
int constellation = 1;

bool mode1 = true, mode2 = false, mode3 = false;
bool E0debugs = false;
bool database = true;
bool tests = true;

std::chrono::microseconds sum(vector<std::chrono::microseconds> vector1) {
    std:
    chrono::microseconds s = (std::chrono::microseconds) 0;
    for (auto x: vector1) {
        s += x;
    }
    return s;
}

std::string getCurrentTimeWithPrecision() {
    using namespace std::chrono;

    // Get current time with high precision
    auto now = system_clock::now();

    // Convert to time_t for date/time components
    auto now_time = system_clock::to_time_t(now);

    // Convert to `tm` struct (UTC time)
    std::tm tm = *std::localtime(&now_time); // Use `gmtime` for UTC; `localtime` for local time

    // Format date/time up to seconds
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    // Add fractional seconds (milliseconds/microseconds/nanoseconds)
    auto since_epoch = now.time_since_epoch();
    auto seconds = duration_cast<chrono::seconds>(since_epoch);
    since_epoch -= seconds;
    auto microseconds = duration_cast<chrono::microseconds>(since_epoch);

    oss << "." << std::setfill('0') << std::setw(6) << microseconds.count();

    return oss.str();
}

struct Data {
    double snr;
    int mod;
    std::string n;  // Changed to string to handle "20+"
    double rho;
    double err;
};

struct CustomNumpunct : std::numpunct<char> {
protected:
    char do_decimal_point() const override {
        return ','; // Use ',' as the decimal separator
    }
};

int main() {
    const Aws::String tableName = "SignalData4";

    if (database) { // should be on
        Aws::SDKOptions options;

        Aws::InitAPI(options);
        {
            Aws::Client::ClientConfiguration config;
            tests = true;
            if(tests){
                my_main(tableName);
            }
            // Create table
            //createTable(tableName, "date",config);

            // Insert sample data
            // Retrieve data
            //getItem(tableName, currentTime, config);
            //}


            vector<vector<double>> e0_1_samples;
            vector<vector<double>> e0_2_samples;
            vector<vector<double>> e0_3_samples;
            vector<double> rates, snrs;
            vector<int> mods;

            // Generate rates, snrs, and mods based on the input ranges and pts
            if (ptsRate == 1) {
                rates.push_back(minRate);
            } else {
                for (int i = 0; i < ptsRate; ++i) {
                    double rate = minRate + i * (maxRate - minRate) / (ptsRate - 1);
                    rates.push_back(rate);
                }
            }

            if (ptsSNR == 1) {
                snrs.push_back(minSNR);
            } else {
                for (int i = 0; i < ptsSNR; ++i) {
                    double snr = minSNR + i * (maxSNR - minSNR) / (ptsSNR - 1);
                    snrs.push_back(snr);
                }
            }

            // Generate mods as powers of 2 within the specified range
            if (ptsMod == 1) {
                mods.push_back(minMod);
            } else {
                int currentMod = 1;
                while (currentMod <= maxMod) {
                    if (currentMod >= minMod) {
                        mods.push_back(currentMod);
                    }
                    currentMod *= 2;
                }
            }

            std::string constel;

            if (constellation == 1) {
                constel = "psk";
            } else if (constellation == 2) {
                constel = "pam";
            } else {
                constel = "secret"; //todo: buggy (produces nan) --> implement
            }

            vector<string> constellations = {"pam", "psk"};

            // Gradient descent parameters
            int it = 20, n = 20; // only for GD/NAG!

            // --- SETTERS ---
            compute_hweights(n, it);
            setMod(mods[0], constel);


            auto start3 = std::chrono::high_resolution_clock::now();

            auto stop3 = std::chrono::high_resolution_clock::now();
            auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
            std::cout << endl << "time filling matrices: " << duration3.count() << " microseconds" << '\n';
            // --------------


            double e0_1, e0_2, e0_3;
            int totalIterations = rates.size() * snrs.size() * mods.size();
            int currentIteration = 0;
            auto deltas_sum = 0;
            double rho = 1, r = .5;
            unordered_map<int, vector<double>> alphadict;

            for (int k = 0; k < mods.size(); k++) {
                std::vector<double> alphas(mods[k], 0.01);
                alphadict[mods[k]] = alphas;
            }

            std::ofstream times_file("times_e0.txt");
            if (!times_file.is_open()) {
                std::cerr << "Error opening file!" << std::endl;
                return 1;
            }

            std::ofstream times_file_raw("times_e0_raw.txt");
            if (!times_file.is_open()) {
                std::cerr << "Error opening file!" << std::endl;
                return 1;
            }
            /*
            snrs = {};
            // 1 30
            for (double d = 1; d <= 4; d += 1) {
                //snrs.push_back(max(d,0.1));
                snrs.push_back(d);
            }

            rates = {1}; // todo .5
             */
            //mods = {4};

            std::ofstream outfile("outfile1");
            if (!outfile.is_open()) {
                std::cerr << "Error opening file!" << std::endl;
                return 1;
            }
            //outfile.imbue(std::locale(outfile.getloc(), new CustomNumpunct));

            /*
            double error1 = 10e-1;
            double error2 = 10e-2;
            double error3 = 10e-3;
            double error4 = 10e-4;
            double error5 = 10e-5;
            double error6 = 10e-6;
            double error7 = 10e-7;
            double error8 = 10e-8;
            double error9 = 10e-9;
            double error10 = 10e-10;
            double error11 = 10e-11;
            double error12 = 10e-12;
            double error13 = 10e-13;
            double error14 = 10e-14;
            */



            vector<double> errors;// = {error1,error2,error3,error4,error5,error6,error7,error8,error9,error10,error11,error12,error13,error14};
            int nerrors = 1;

            vector<double> errorsaux;

            // Starting value (adjust as needed)
            double current = 1;

            // Determine how many elements you want (e.g., 4 elements as in the example)
            for (int i = 0; i < nerrors; ++i) {
                errors.push_back(current);
                current = current / 1.5; // Halve the value
                //current = std::floor(current * 100.0) / 100.0; // Truncate to two decimal places
            }


            // Calculate e0_samples using gradient descent for each combination of rate, snr, and mod
            for (int i = 0; i < rates.size(); ++i) {
                for (int j = 0; j < snrs.size(); /*1;*/ ++j) { // todo change
                    // 1 for e0deb, size for mode1
                    vector<double> e0_row_1, e0_row_2, e0_row_3;
                    for (int k = 0; k < mods.size(); ++k) {
                        for (int l = 0; l < constellations.size(); l++) {
                            for (int n = 1; n <= 2; n++) {

                                double r = rates[i];
                                double snr = snrs[j];
                                double mod = mods[k];
                                string con = constellations[l];

                                cout << r << " " << snr << " " << mod << " " << con << " " << n << endl;
                                /*
                                for (int i = 0; i < 60; i++) cout << "/";
                                cout << endl;
                                cout << "m: " << to_string(mods[k]) << endl;
                                cout << "n: " << n << endl;
                                cout << "constellation: " << constel << endl;
                                for (int i = 0; i < 60; i++) cout << "/";
                                cout << endl;
                                */
                                setMod(mod, con);
                                setQ(); // matrix Q

                                if (E0debugs) {
                                    std::ofstream e0_file(
                                            "new3_" + to_string(mods[k]) + "_" + constel + "_n_2" + ".txt");
                                    if (!e0_file.is_open()) {
                                        std::cerr << "Error opening file!" << std::endl;
                                        return 1;
                                    }
                                    //e0_file.imbue(std::locale(e0_file.getloc(), new CustomNumpunct));
                                    vector<std::ofstream> outfiles;


                                    for (int a = 0; a < nerrors; a++) {
                                        std::ofstream outfile_temp("outfile" + to_string(a + 1));
                                        if (!outfile_temp.is_open()) {
                                            std::cerr << "Error opening file!" << std::endl;
                                            return 1;
                                        }
                                        outfiles.push_back(std::move(outfile_temp));
                                    }

                                    outfile << "snr m n";
                                    for (auto snr: snrs) {
                                        setSNR(snr);
                                        setW();

                                        vector<double> rhos;

                                        for (double rho_ = -1; rho_ <= 2; rho_ += .5) {
                                            rhos.push_back(max(rho_, -0.99));
                                        }


                                        //rhos.push_back(-0.49);
                                        //xrhos.push_back(-0.51);
                                        //rhs.push_back(-0.5);

                                        for (double rho_: rhos) {
                                            /*bool flag1 = false, flag2 = false, flag3 = false, flag4 = false, flag5 = false;
                                            bool flag6 = false, flag7 = false, flag8 = false, flag9 = false, flag10 = false;
                                            bool flag11 = false, flag12 = false, flag13 = false, flag14 = false;*/

                                            vector<bool> flags(nerrors, false);


                                            setN(100);

                                            // matrices
                                            setPI();
                                            setW(); //setA(alphadict[mods[k]]);

                                            vector<double> hweights = getAllHweights();
                                            vector<double> roots = getAllRoots();
                                            vector<double> multhweights = getAllMultHweights();
                                            double grad_rho, e0;
                                            double e0_1 = E_0_co(r, rho_, grad_rho, e0);

                                            int maxn = 40;
                                            for (int n_ = 1; n_ <= maxn; n_++) {
                                                //compute_hweights(n_, it);
                                                cout << "n_: " << n_ << endl;
                                                //auto start = std::chrono::high_resolution_clock::now();
                                                vector<double> e0_samples;

                                                //qDebug() << "i:" << i << " j:" << j << " k:" << k;
                                                //qDebug() << "modval: " << mods[k];
                                                setR(rates[i]);
                                                //setSNR(snrs[j]);

                                                ///// CODE FOR THE ERROR TEST
                                                // n
                                                setN(n_);

                                                // matrices
                                                setPI();
                                                setW(); //setA(alphadict[mods[k]]);

                                                vector<double> hweights = getAllHweights();
                                                vector<double> roots = getAllRoots();
                                                vector<double> multhweights = getAllMultHweights();

                                                double grad_rho, grad_2_rho; // placeholders
                                                double e0ptr, e0;
                                                double incr = .01;

                                                auto start_XX = std::chrono::high_resolution_clock::now();
                                                /*
                                                for (double rho_ = -1 + incr; rho_ < 2 + incr; rho_ += incr) {
                                                    e0 = E_0_co(0, rho_, grad_rho, e0ptr, n - 1, hweights, multhweights, roots);
                                                    e0_file << fixed << setprecision(15) << e0 << endl;
                                                }
                                                */

                                                e0_file << fixed << setprecision(4) << snr << " ";
                                                //double e0_1 = E_0_1_co();
                                                //cout << "E_0_1: " << e0 << endl;
                                                //cout << "patata1" << endl;
                                                e0_file << fixed << setprecision(17) << e0 << " ";
                                                // rel2 approx
                                                double e0_1_approx = E_0_co(r, rho_, grad_rho, e0);
                                                cout << fixed << setprecision(16) << "e0_1: " << e0_1 << endl;
                                                cout << fixed << setprecision(16) << "E_0_1_approx: " << e0_1_approx
                                                     << endl;
                                                cout << "diff: " << scientific
                                                     << abs(e0_1 - e0_1_approx) / abs(e0_1_approx)
                                                     << endl;
                                                outfiles[0] << abs(e0_1 - e0_1_approx) / abs(e0_1_approx) << " " << snr
                                                            << " "
                                                            << mods[k] << " " << n_ << " " << rho_ << endl;

                                                //e0 = E_0_co(0, .5, grad_rho, e0ptr, n - 1, hweights, multhweights, roots);
                                                /*
                                                vector<double> errors = {};
                                                for(double error = 10E-1; error < 10E-15; error-= 10E-1){
                                                    errors.push_back(error);
                                                }

                                                vector<bool> flags;
                                                for(int i = 0; i < 15; i++){
                                                    flags[i] = false;
                                                }

                                                double dif = e0_1 - e0_1_approx;

                                                vector<string> ns(15,0);

                                                for(int i = 0; i < size(errors); i++){
                                                    if(dif <= errors[i] && !flags[i]){
                                                        ns[i] = to_string(n_);
                                                        flags[i] = true;
                                                        if(i==15){ // max error
                                                            break;
                                                        }
                                                    }
                                                    else if(n_ == maxn && !flags[i]){
                                                        ns[i] =  to_string(maxn) + "+";
                                                        if(i == 15){ // max error
                                                            break;
                                                        }
                                                    }
                                                }
                                                */

                                                //outfiles[0] << abs(e0_1 - e0_1_approx)/abs(e0_1_approx) << " " << snr << " " << mods[k] << " " << n_ << " " << rho_ << endl;



                                                //cout << "e0: " << e0 << endl;
                                                cout << "gr: " << grad_rho << endl;
                                                cout << "g2r: " << grad_2_rho << endl;
                                                e0_file << fixed << setprecision(17) << e0 << " ";
                                                /*
                                                e0 = E_0_2_co();
                                                cout << "E_0_2: " << e0 << endl;
                                                //cout << "patata1" << endl;
                                                e0_file << fixed << setprecision(17) << e0 << " ";

                                                e0 = E_0_co(r, 2, grad_rho, e0, n-1, hweights, multhweights, roots);
                                                cout << "E_0_1_co: " << e0 << endl;
                                                //e0 = E_0_co(0, .5, grad_rho, e0ptr, n - 1, hweights, multhweights, roots);

                                                cout << "e0: " << e0 << endl;
                                                cout << "gr: " << grad_rho << endl;
                                                cout << "gr: " << grad_2_rho << endl;
                                                e0_file << fixed << setprecision(17) << e0 << endl;
                                                */
                                                outfile << fixed << setprecision(17) << snr << " " << e0_1 << " "
                                                        << e0_1_approx
                                                        << endl;

                                                auto stop_XX = std::chrono::high_resolution_clock::now();
                                                auto duration_XX = std::chrono::duration_cast<std::chrono::microseconds>(
                                                        stop_XX - start_XX);
                                                times_file << "m: "
                                                           << to_string(mods[k]) + " " + constel + " n_: " +
                                                              to_string(n_)
                                                           << " time: " << duration_XX.count() << " microseconds"
                                                           << endl;
                                                times_file_raw << duration_XX.count() << endl;
                                            }
                                        }
                                    }
                                }
                                if (mode1) { // iid mode
                                    //rho = 0.01; // for fair testing
                                    setR(r);
                                    setSNR(snr);
                                    setN(n);

                                    // matrices
                                    setPI();
                                    setW(); //setA(alphadict[mods[k]]);

                                    vector<double> hweights = getAllHweights();
                                    vector<double> roots = getAllRoots();
                                    vector<double> multhweights = getAllMultHweights();
                                    /*
                                    double grad_rho, grad_2_rho; // placeholders
                                    double e0ptr, e0;
                                    double incr = .01;
                                     */
                                    /*
                                    //e0_1 = NAG_iid(r, rho, 1/(-0.395401+0.721448),it, n, 0.76 //2.6);
                                    /*1/beta  //1.55 / 0.644112,
                                    double rho_gd, rho_interpolated;

                                    // double e0_1;              // gauss hermite e0 at rho_gd
                                    double e0_1_interpolated;    // interpolated e0 at rho_interpolated
                                    double e0_1_rhointerpolated; // gauss hermite e0 at rho_interpolated
                                    */
                                    double rho_gd, rho_interpolated;
                                    e0_1 = GD_iid(r, rho_gd /*rho*/, rho_interpolated, it, n);
                                    if(rho_gd != 1){
                                        cout << "rho_gd changes!" << endl;
                                    }
                                    Aws::String currentTime = getCurrentTimeWithPrecision();
                                    /*
                                    putItem(tableName,
                                            currentTime,              // date
                                            e0_1,                      // e0
                                            rho_gd,              // optimal_rho
                                            mod,                        // M
                                            con,                 // const
                                            snr,                           // snr
                                            r,                             // rho
                                            n,                             // n
                                            config);
                                            */
                                    /*
                                    const Aws::String tableName = ("test");
                                    const Aws::String artistKey = ("test");
                                    const Aws::String artistValue = ("test");
                                    const Aws::String albumTitleKey = ("test");
                                    const Aws::String albumTitleValue = ("test");
                                    const Aws::String awardsKey = ("test");
                                    const Aws::String awardsValue = ("test");
                                    const Aws::String songTitleKey = ("test");
                                    const Aws::String songTitleValue = ("test");

                                    const Aws::String primaryKey("test");
                                    Aws::Client::ClientConfiguration clientConfig;
                                    // Optional: Set to the AWS Region (overrides config file).
                                    // clientConfig.region = "us-east-1";

                                    //createTable(tableName, primaryKey, clientConfig);
                                    putItem(tableName, artistKey, artistValue, albumTitleKey,
                                                              albumTitleValue,
                                                              awardsKey, awardsValue, songTitleKey, songTitleValue,
                                                              clientConfig);


                                    const Aws::String partitionKey = ("test");
                                    const Aws::String partitionValue = ("test");

                                    getItem(tableName, partitionKey, partitionValue,
                                                              clientConfig);
                                    */

                                }
                                /*
                                // find e0(rho_int)
                                e0_1_rhointerpolated = E_0_co(r, rho_interpolated, grad_rho, e0);

                                E_0_co(r, 0, grad_rho, e0);
                                double E0_0 = e0, E0_prime_0 = grad_rho;
                                E_0_co(r, 1, grad_rho, e0);
                                //double E0_1 = e0, E0_prime_1 = grad_rho;

                                // find e0_int(rho_int)
                                //rho = initial_guess(r, E0_0, E0_1, E0_prime_0, E0_prime_1, e0_1_interpolated);

                                //double e0_1_der = E_0_co(r, 1, grad_rho, grad_2_rho, e0, n, hweights, multhweights, roots);
                                //e0_1 -= rho_gd*r;
                                //e0_1_rhointerpolated -= rho_interpolated*r;
                                //cout << "second der: " << grad_2_rho << endl;

                                outfile << snr << " " << r << " " << rho_gd << " " << rho_interpolated << " "
                                        << rho_gd - rho_interpolated << " " << e0_1 - (e0_1_interpolated + rho_interpolated * r)
                                        << " " << (e0_1 - rho_gd * r) - (e0_1_rhointerpolated - rho_interpolated * r) << " "
                                        << e0_1
                                        << " " << e0_1_interpolated << " " << e0_1_rhointerpolated << endl;
                                //outfile << snr << " " << r << " " << rho_gd << " " << rho_interpolated << " " << rho_gd - rho_interpolated << " " << e0_1 - (e0_1_interpolated+rho_interpolated*r) << " " << (e0_1-rho_gd*r-rho_gd*r) - (e0_1_rhointerpolated-rho_interpolated*r-rho_interpolated*r) << " " << e0_1 << " " << e0_1_interpolated << " " << e0_1_rhointerpolated << endl;
                                //}
                                //e0_1 = NM_co(r, rho, it, n,false);
                                */
                                //e0_row_1.push_back(e0_1);

                                if (mode2) { // cost constraint mode
                                    //rho = 0; r = 0;
                                    //e0_2 = GD_cc(r, rho, 1/beta, it, n);
                                    e0_2 = NAG_iid(r, rho, /*1/beta*/1 / 0.644112, it, n, 2.6);
                                    e0_row_2.push_back(e0_2);
                                }
                                if (mode3) { // constant composition mode
                                    //vector<double> alphas(mods[k],0); rho = 0;
                                    double beta = 3; //amongus
                                    e0_3 = GD_ccomp(/*alphas*/alphadict[mods[k]], rho, 1 / beta, it, n);
                                    //e0_3 = NAG(// alphas alphadict[mods[k]], rho, 1/beta, it, n, 2.6);
                                    e0_row_3.push_back(e0_3);
                                }
                                /*
                                auto stop = std::chrono::high_resolution_clock::now();
                                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                                //qDebug() << "modulation " << mods[k] << "Duration: " << duration.count() * pow(10,-6) << "s";
                                deltas_sum += duration.count();
                                //cout << duration.count() << endl;
                                ++currentIteration;
                                */
                                //float percentage = (static_cast<float>(currentIteration) / totalIterations) * 100;
                                //qDebug() << "progress: " << percentage;


                                // Vector to store all data
                                //std::vector<std::vector<Data>> allData;
                                /*
                                // Read data from outfile1 to outfile14
                                for (int i = 1; i <= nerrors; ++i) {
                                    std::string filename = "outfile" + std::to_string(i);
                                    std::ifstream infile(filename);
                                    if (!infile.is_open()) {
                                        std::cerr << "Error opening file: " << filename << std::endl;
                                        return 1;
                                    }

                                    std::vector<Data> fileData;
                                    std::string line;
                                    while (std::getline(infile, line)) {
                                        std::istringstream iss(line);
                                        Data data;
                                        if (iss >> data.err >> data.snr >> data.mod >> data.n >> data.rho) {
                                            fileData.push_back(data);
                                        }
                                    }
                                    allData.push_back(fileData);
                                    infile.close();
                                }
                                 */
                                /*
                                // Write data to CSV

                                std::ofstream csvFile("C:\\Users\\TESTER\\Documents\\MATLAB\\output.csv");
                                if (!csvFile.is_open()) {
                                    std::cerr << "Error creating CSV file!" << std::endl;
                                    return 1;
                                }

                                // Write header
                                csvFile << "error,snr,mod,n,rho\n";

                                // Write data for each error
                                for (int i = 0; i < nerrors; ++i) {
                                    double error = errors[i]; // error1 = 10^-1, error2 = 10^-2, ..., error15 = 10^-14
                                    for (const auto &data: allData[i]) {
                                        csvFile << std::fixed << std::setprecision(15) << data.err << ","
                                                << data.snr << "," << data.mod << "," << data.n << "," << data.rho << "\n";
                                    }
                                }

                                csvFile.close();

                                std::cout << "CSV file created successfully: output.csv" << std::endl;
                                 */
                            }
                        }
                    }
                    /*
                    if (mode1)
                        e0_1_samples.push_back(e0_row_1);
                    if (mode2)
                        e0_2_samples.push_back(e0_row_2);
                    if (mode3)
                        e0_3_samples.push_back(e0_row_3);
                        */
                }
            }
            /*
            if (mode1) {

                cout << "rates" << "," << "snrs" << "," << "mods" << "," << "e0" << endl;
                for (int i = 0; i < rates.size(); ++i) {
                    for (int j = 0; j < snrs.size(); ++j) {
                        for (int k = 0; k < mods.size(); ++k) {
                            cout << rates[i] << "," << snrs[j] << "," << mods[k] << ","
                                 << e0_1_samples[i * snrs.size() + j][k] << "\n";
                        }
                    }
                }

            }
            if (mode2) {
                for (int i = 0; i < rates.size(); ++i) {
                    for (int j = 0; j < snrs.size(); ++j) {
                        for (int k = 0; k < mods.size(); ++k) {
                            cout << rates[i] << "," << snrs[j] << "," << mods[k] << ","
                                 << e0_2_samples[i * snrs.size() + j][k] << "\n";
                        }
                    }
                }
            }
            if (mode3) {
                for (int i = 0; i < rates.size(); ++i) {
                    for (int j = 0; j < snrs.size(); ++j) {
                        for (int k = 0; k < mods.size(); ++k) {
                            cout << rates[i] << "," << snrs[j] << "," << mods[k] << ","
                                 << e0_3_samples[i * snrs.size() + j][k] << "\n";
                        }
                    }
                }
            }
             */

            cout << "avg time: " << deltas_sum / totalIterations << endl;
            cout << "total time: " << deltas_sum << endl;


            cout << endl << "---------------------------------" << endl;

            cout << "avg time: " << deltas_sum / totalIterations << endl;
            cout << "total time: " << deltas_sum << endl;
            unordered_map<string, vector<chrono::microseconds>> e0times = get_times();

            //const unordered_map<string, vector<chrono::microseconds>>& e0times) {

            // Convert the unordered_map to a vector of pairs (for sorting)
            vector<pair<string, vector<chrono::microseconds>>> sorted_entries(e0times.begin(), e0times.end());

            // Define a sorting comparator that compares the sum of vector elements
            auto comparator = [](const pair<string, vector<chrono::microseconds>> &a,
                                 const pair<string, vector<chrono::microseconds>> &b) {
                auto sumVector = [](const vector<chrono::microseconds> &v) {
                    chrono::microseconds sum(0);
                    for (const auto &val: v) {
                        sum += val;
                    }
                    return sum;
                };

                return size(a.second) > size(b.second); // Descending order
            };

            // Sort the vector of pairs using the comparator
            sort(sorted_entries.begin(), sorted_entries.end(), comparator);

            // Create a new unordered_map to store the sorted elements
            unordered_map<string, vector<chrono::microseconds>> sorted_map;

            // Insert sorted elements into the new unordered_map
            for (const auto &entry: sorted_entries) {
                sorted_map[entry.first] = entry.second;
            }


            for (auto pair_: sorted_entries) {
                if (!pair_.second.empty()) {
                    cout << endl << pair_.first << ": " << size(pair_.second) << " times";
                }
            }
            cout << endl << "-------------" << endl;

            unordered_map<string, int> avgs; // average times

            for (auto pair_: e0times) {
                if (!pair_.second.empty()) {
                    //cout << endl << pair_.first << " " << sum(pair_.second).count()/ size(pair_.second) << endl;
                    avgs[pair_.first] = sum(pair_.second).count(); /// size(pair_.second);
                }
            }

            std::vector<std::pair<std::string, int>> sorted_avgs(avgs.begin(), avgs.end());

            std::sort(sorted_avgs.begin(), sorted_avgs.end(),
                      [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
                          return a.second > b.second;
                      });

            for (const auto &pair: sorted_avgs) {
                std::cout << pair.first << ": " << pair.second << " microseconds" << '\n';
            }

            int sum = 0;

            for (const auto &pair: avgs) {
                sum += pair.second;
            }

            std::cout << "Sum of all times: " << sum << " microseconds" << '\n';

            vector<int> e0times2;
            /*
            for(double rho_ = 0; rho_ < 1; rho_ += 0.1){
                auto start2 = std::chrono::high_resolution_clock::now();
                E_0_co(rho_,0,0,n);
                auto stop2 = std::chrono::high_resolution_clock::now();
                auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
                e0times2.push_back(duration2.count());
                std::cout << "e0total: " << duration2.count() << " microseconds" << '\n';
            }
             */

            //cout << endl << "e0av: " << sum(e0times2)/e0times2.size() << endl;


            //E_0_co(0.5,0,0, n);

            // cout << "avg E0 time: "   << e0sum.count()/totalIterations << endl;
            // cout << "total E0 time: " << e0sum.count() << endl;
        }
        Aws::ShutdownAPI(options);
        return 0;
    }
}
