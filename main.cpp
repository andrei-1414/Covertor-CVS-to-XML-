#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
fs::path pathToFile;

std::atomic<bool> foundFile(false);
void sleep_seconds(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
fs::path getPath(){
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    fs::path exePath(path);
    return exePath.parent_path();
}
void functie2(std::string fileNameWithExtension, std::string fileNameWithoutExtension) {
    std::ifstream fin(fileNameWithExtension);
    std::string fullPath = (pathToFile / "OutputFolder").string() + "/" + fileNameWithoutExtension + ".xml";
    std::ofstream fout(fullPath);
    if (!fin.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        sleep_seconds(5); // va fi sters
        return ;
    }
    std::vector<std::string> Headers;
    //std::vector<std::string> rows;
    std::string line;
    std::getline(fin, line); 
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string header;
        while (std::getline(ss, header, ':')) {
            Headers.push_back(header);
        }
        break; 
    }

    if (Headers.empty()) {
        std::cerr << "No headers found in the file." << std::endl;
        sleep_seconds(5);
        return ;
    }
    fout << "<facturi> \n"; 
    int cnt = 0;
    Headers.pop_back();
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> row;

        while (std::getline(ss, value, ':')) {
            row.push_back(value);
        }

        if (row.size() != Headers.size()) {
            continue;
        }
        int n = std::atoi(row[0].c_str());
        if (n <= 0) {
            continue; 
        }

        fout << "<factura id =\"" << cnt << "\">\n"; 
        for (size_t i = 0; i < Headers.size(); ++i) {
            std::string tag1 = Headers[i];
            std::string tag2 = row[i];
            std::replace(tag1.begin(), tag1.end(), ' ', '_');
            tag1.erase(std::remove(tag1.begin(), tag1.end(), '.'), tag1.end());
            tag1.erase(std::remove(tag1.begin(), tag1.end(), '/'), tag1.end());

            std::replace(tag2.begin(), tag2.end(), ' ', '_');
            tag2.erase(std::remove(tag2.begin(), tag2.end(), '.'), tag2.end());
            tag2.erase(std::remove(tag2.begin(), tag2.end(), '/'), tag2.end());


            fout << "<" << tag1 << ">" << tag2 << "</" << tag1 << ">\n";
        }
        fout << "</factura>\n";
        cnt++;
    }
    fout << "</facturi>\n";

    fin.close();
    std::cout << "File processed: " << fileNameWithoutExtension << std::endl;
}
void functie1() {
    std::string folder = (pathToFile / "InputFolder").string();
    while (!foundFile) {
        std::cout<< "Waiting for file...\n";
        if (fs::exists(folder) && fs::is_directory(folder)) {
            for (const auto& entry : fs::directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string(); 
                    if (ext == ".csv"){
                        foundFile = true;
                        functie2(entry.path().string(),entry.path().stem().string());
                        fs::remove(entry.path());
                    }
                        
                }
            }
        } else {
            std::cout << "Folderul nu există sau nu este valid.\n";
        }
        sleep_seconds(1);
    }
}

int main() {
    pathToFile = getPath();
    std::thread t1(functie1);

    t1.join();
    sleep_seconds(2);
    return 0;
}
