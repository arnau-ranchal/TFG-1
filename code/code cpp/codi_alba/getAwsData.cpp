#include "getAwsData.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>

#include "include/json.hpp"
using json = nlohmann::json;


using json = nlohmann::json;

std::string exec(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");

    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

std::map<std::string, std::string> getAwsData(int M, int N, double SNR, double R) {
    std::map<std::string, std::string> data;

    std::ostringstream cmd;
    cmd << "python get_data.py"
        << " --table exponents"
        << " --constellationM " << M
        << " --nodesN " << N
        << " --SNR " << SNR
        << " --transmissionRate " << R;

    try {
        std::string output = exec(cmd.str());

        auto json_data = json::parse(output);
        for (auto& [key, value] : json_data.items()) {
            data[key] = value.dump();  // almacenas como string
        }
    } catch (const std::exception& e) {
        data["status"] = "error";
        data["message"] = std::string("Python script call or JSON parsing failed: ") + e.what();
    }

    return data;
}
