/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * A station which can capture data in a specified location on demand and also save it to a csv file
 **/

#include "Station.h"
#include <sstream>
#include <fstream>

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
    if (m_waveProp->getHeight() != nullptr)
        capturedData.push_back(m_waveProp->getHeight()[t_idx(m_x + m_y * m_stride)]);
    if (m_waveProp->getMomentumX() != nullptr)
        capturedData.push_back(m_waveProp->getMomentumX()[t_idx(m_x + m_y * m_stride)]);
    if (m_waveProp->getMomentumY() != nullptr)
        capturedData.push_back(m_waveProp->getMomentumY()[t_idx(m_x + m_y * m_stride)]);
    if (m_waveProp->getBathymetry() != nullptr)
        capturedData.push_back(m_waveProp->getBathymetry()[t_idx(m_x + m_y * m_stride)]);
    if (m_waveProp->getHeight() != nullptr && m_waveProp->getBathymetry() != nullptr)
        capturedData.push_back(m_waveProp->getHeight()[t_idx(m_x + m_y * m_stride)] +
                               m_waveProp->getBathymetry()[t_idx(m_x + m_y * m_stride)]);
    m_data->push_back(capturedData);
}

void tsunami_lab::io::Station::write()
{
    std::string l_path = m_filepath + "/" + m_name + ".csv";
    std::ofstream l_file;
    l_file.open(l_path);
    // write the CSV header
    l_file << "time";
    if (m_waveProp->getHeight() != nullptr)
        l_file << ",height";
    if (m_waveProp->getMomentumX() != nullptr)
        l_file << ",momentum_x";
    if (m_waveProp->getMomentumY() != nullptr)
        l_file << ",momentum_y";
    if (m_waveProp->getBathymetry() != nullptr)
        l_file << ",bathymetry";
    if (m_waveProp->getHeight() != nullptr && m_waveProp->getBathymetry() != nullptr)
        l_file << ",totalHeight";
    l_file << "\n";

    // write data
    for (std::vector<t_real> elem : *m_data)
    {
        for (t_idx i = 0; i < elem.size()-1; i++)
        {
            l_file << elem[i] << ",";
        }
        l_file << elem[elem.size()-1];
        l_file << "\n";
    }
    l_file.close();
}
