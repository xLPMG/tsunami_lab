/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Stations
 **/

#include "Station.h"
#include <sstream>
#include <fstream>
#include <iostream>

tsunami_lab::io::Station::Station(t_real i_x,
                                  t_real i_y,
                                  std::string i_name,
                                  tsunami_lab::patches::WavePropagation *i_waveProp)
{
    m_x = i_x;
    m_y = i_y;
    m_name = i_name;
    m_waveProp = i_waveProp;
    m_stride = i_waveProp->getStride();
    m_data = new std::vector<std::vector<t_real>>;
}

tsunami_lab::io::Station::~Station()
{
    delete m_data;
}

void tsunami_lab::io::Station::capture(t_real i_time)
{
    std::vector<t_real> capturedData;
    capturedData.push_back(i_time);
    capturedData.push_back(m_waveProp->getHeight()[t_idx(m_x + m_y * m_stride)]);
    capturedData.push_back(m_waveProp->getMomentumX()[t_idx(m_x + m_y * m_stride)]);
    capturedData.push_back(m_waveProp->getMomentumY()[t_idx(m_x + m_y * m_stride)]);
    capturedData.push_back(m_waveProp->getBathymetry()[t_idx(m_x + m_y * m_stride)]);
    m_data->push_back(capturedData);
}

void tsunami_lab::io::Station::write()
{
    std::string l_path = m_filepath + "/" + m_name + ".csv";
    std::ofstream l_file;
    l_file.open(l_path);
    l_file << "time,height,momentum_x,momentum_y,bathymetry"
           << "\n";
    for (std::vector<t_real> elem : *m_data)
    {
        l_file << elem[0] << ",";
        l_file << elem[1] << ",";
        l_file << elem[2] << ",";
        l_file << elem[3] << ",";
        l_file << elem[4] << "\n";
    }
    l_file.close();
}
