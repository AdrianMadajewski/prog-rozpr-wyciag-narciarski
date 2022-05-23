#include "File.h"

std::vector<int> loadDataFromFile(const std::string& filename)
{
    std::vector<int> data;
    std::ifstream file;
    file.open(filename);

    if(!file.is_open())
    {
        std::cerr << "Couldn't find the file. Please restart." << '\n';
        exit(EXIT_FAILURE);
    }
    while(!file.eof())
    {
        int read;
        file >> read;
        data.emplace_back(read);
    }
    file.close();
    return data;
}