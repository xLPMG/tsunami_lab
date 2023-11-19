/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Loads bathymetry data from files
 **/
#include "BathymetryLoader.h"
#include "Csv.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

tsunami_lab::io::BathymetryLoader::~BathymetryLoader()
{
    delete[] m_b;
}

void tsunami_lab::io::BathymetryLoader::loadBathymetry(const std::string &i_file)
{
    std::cout << "Loading bathymetry from file: " << i_file << std::endl;
    if (!std::filesystem::exists(i_file))
    {
        std::cerr << "Error: File not found!" << std::endl;
        exit(1);
    }

    std::ifstream l_inputFile(i_file);

    std::string l_line;
    std::vector<std::string> l_row;
    // load bathymetry dimension
    while (getline(l_inputFile, l_line))
    {
        if (l_line.substr(0, 1) == "#")
            continue;
        if (l_line.substr(0, 3) == "DIM")
        {
            tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);
            m_sizeX = std::stof(l_row[1]);
            m_sizeY = std::stof(l_row[1]);
            // initialize bathymetry array
            m_b = new t_real[t_idx(m_sizeX * m_sizeY)]{0};
            break;
        }
    }

    if (m_b != nullptr)
    {
        while (getline(l_inputFile, l_line))
        {
            if (l_line.substr(0, 1) == "#")
                continue;
            tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);
            // x and y location in metres
            t_real l_x = std::stof(l_row[0]);
            t_real l_y = std::stof(l_row[1]);
            // bathymetry data
            t_real l_b = std::stof(l_row[2]);
            m_b[t_idx(l_x + l_y * m_sizeX)] = l_b;
        }
        l_inputFile.close();
        std::cout << "Done loading bathymetry." << std::endl;
    }
    else
    {
        std::cerr << "Error: No dimensions specified in bathymetry file!" << std::endl;
    }
}