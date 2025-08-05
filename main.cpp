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

    std::vector<char> FileNameWithoutExtension(fileNameWithoutExtension.begin(), fileNameWithoutExtension.end());
    for(int i =0 ;i < 9 ;i++) {
        FileNameWithoutExtension.erase(FileNameWithoutExtension.begin()); 
    }
    std::string auxToReplaceTheChars(FileNameWithoutExtension.begin(), FileNameWithoutExtension.end());
    std::replace(auxToReplaceTheChars.begin(), auxToReplaceTheChars.end(), '_', '.');
    std::string fileNameWithoutExtensionFinal = "I_" + auxToReplaceTheChars;
    
    std::string fullPath = (pathToCsvFile / "OutputFolder").string() + "/" + fileNameWithoutExtensionFinal + ".xml";

    std::ofstream fout(fullPath);
    if (!fin.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        sleep_seconds(5); // va fi sters
        return ;
    }
    std::vector<std::string> headers = {"Data","Nuamr","Suma","Cont","ContClient","Explicatie","FacturaNumar","CodFiscal"};
    std::string line;
    std::getline(fin, line); // specific formatului pe care il asteptam
    std::getline(fin, line); // specific formatului pe care il asteptam
    std::getline(fin, line); // specific formatului pe care il asteptam
    /*while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string header;
        while (std::getline(ss, header, ':')) {
            headers.push_back(header);
        }
        break; 
    }*/
    fout << "<Incasari> \n"; 
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> row;

        while (std::getline(ss, value, ':')) {
            row.push_back(value);
        }
        /*if (row.size() != headers.size()) {
            continue;
        }*/
        int n = std::atoi(row[0].c_str());
        if (n <= 0) {
            continue; 
        }
        std::vector<char> formatulCoreect = {'E' , 'C' , 'R' , 'C' , 'M'}; //ECRCM10002
        std::vector<char> numar(row[4].begin(), row[4].end());
        bool isValid = true;
        for(int i = 0;i < formatulCoreect.size();i++) {
            if (numar[i] != formatulCoreect[i]) {
                isValid = false;
                break; 
            }

        }
        if (!isValid) {
            continue; //
        }
        fout << "<Linie>\n";
        for (size_t i = 0; i < headers.size(); ++i) {
            switch (i) {
                case 0: // Data
                    fout << "<Data>" << row[6] << "</Data>\n";
                    break;
                case 1: { // Nuamr
                    fout << "<Numar>" << row[4] << "</Numar>\n";
                    break;

                }
                case 2: { // Suma
                    std::string sumaFormat = row[8];
                    std::replace(sumaFormat.begin(), sumaFormat.end(), ',', '.'); 
                    fout << "<Suma>" << sumaFormat << "</Suma>\n";
                    break;
                }
                case 3: // Cont
                    fout << "<Cont> "  << row[9]<< "</Cont>\n";
                    break;
                case 4: // ContClient
                    fout << "<ContClient> "<< row[9] << "</ContClient>\n";
                    break;
                case 5: // Explicatie
                    fout << "<Explicatie> " <<"</Explicatie>\n";
                    break;
                case 6: // FacturaNumar
                    fout << "<FacturaNumar>" << row[5] << "</FacturaNumar>\n";
                    break;
                case 7: // CodFiscal
                    fout << "<CodFiscal>" << row[2] << "</CodFiscal>\n";
                    break;
            }
            /*std::string xmlTag = headers[i];
            std::replace(xmlTag.begin(), xmlTag.end(), ' ', '_');
            xmlTag.erase(std::remove(xmlTag.begin(), xmlTag.end(), '.'), xmlTag.end());
            xmlTag.erase(std::remove(xmlTag.begin(), xmlTag.end(), '/'), xmlTag.end());
            
            fout << "<" << xmlTag << ">" << row[i] << "</" << xmlTag << ">\n";*/
        }
        fout << "</Linie>\n";
    }
    fout << "</Incasari>\n";

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
    if(!fs::exists(pathToCsvFile / "InputFolder") || !fs::exists(pathToCsvFile / "OutputFolder")) {
        std::cout << "Creating directories...\n";
        fs::create_directories(pathToCsvFile / "InputFolder");
        fs::create_directories(pathToCsvFile / "OutputFolder");
        sleep_seconds(1);
        return 0;
    }
    std::thread t1(searchForCsvFile);

    t1.join();
    sleep_seconds(1);
    return 0;
}
