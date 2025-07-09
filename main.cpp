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
fs::path pathToCsvFile;

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
void FromCsvToXml(std::string fileNameWithExtension, std::string fileNameWithoutExtension) {
    std::ifstream fin(fileNameWithExtension);
    std::string fullPath = (pathToCsvFile / "OutputFolder").string() + "/" + fileNameWithoutExtension + ".xml";
    std::ofstream fout(fullPath);
    if (!fin.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        sleep_seconds(5); // va fi sters
        return ;
    }
    std::vector<std::string> headers;
    std::string line;
    std::getline(fin, line); // specific formatului pe care il asteptam
    std::getline(fin, line); // specific formatului pe care il asteptam
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string header;
        while (std::getline(ss, header, ':')) {
            headers.push_back(header);
        }
        break; 
    }

    if (headers.empty()) {
        std::cerr << "No headers found in the file." << std::endl;
        sleep_seconds(5);
        return ;
    }
    fout << "<facturi> \n"; 
    int cnt = 0;
    headers.pop_back(); // specific formatului pe care il asteptam
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> row;

        while (std::getline(ss, value, ':')) {
            row.push_back(value);
        }

        if (row.size() != headers.size()) {
            continue;
        }
        int n = std::atoi(row[0].c_str());
        if (n <= 0) {
            continue; 
        }

        fout << "<factura id =\"" << cnt << "\">\n"; 
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string xmlTag = headers[i];
            std::replace(xmlTag.begin(), xmlTag.end(), ' ', '_');
            xmlTag.erase(std::remove(xmlTag.begin(), xmlTag.end(), '.'), xmlTag.end());
            xmlTag.erase(std::remove(xmlTag.begin(), xmlTag.end(), '/'), xmlTag.end());

            fout << "<" << xmlTag << ">" << row[i] << "</" << xmlTag << ">\n";
        }
        fout << "</factura>\n";
        cnt++;
    }
    fout << "</facturi>\n";

    fin.close();
    std::cout << "File processed: " << fileNameWithoutExtension << std::endl;
}
void searchForCsvFile() {
    std::string folder = (pathToCsvFile / "InputFolder").string();
    while (!foundFile) {
        std::cout<< "Waiting for file...\n";
        if (fs::exists(folder) && fs::is_directory(folder)) {
            for (const auto& entry : fs::directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string(); 
                    if (ext == ".csv"){
                        foundFile = true;
                        FromCsvToXml(entry.path().string(),entry.path().stem().string());
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
    pathToCsvFile = getPath();
    std::thread t1(searchForCsvFile);

    t1.join();
    sleep_seconds(2);
    return 0;
}
