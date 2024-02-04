/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Implementation of One-dimensional Tsunami event
 **/

#include "TsunamiEvent1d.h"
#include <math.h>
#include <vector>
#include "../io/Csv.h"
#include <iostream>
#ifndef BENCHMARK
#include <filesystem>
#endif

tsunami_lab::setups::TsunamiEvent1d::TsunamiEvent1d(const std::string &i_file)
{
#ifndef BENCHMARK
    if (!std::filesystem::exists(i_file))
    {
        std::cerr << "Error: File not found "
                  << "(TsunamiEvent1d.cpp)" << std::endl;
        m_fileExists = false;
    }
#endif

    if (m_fileExists)
    {
        std::ifstream l_inputFile(i_file);
        m_bathymetry = new std::vector<tsunami_lab::t_real>;

        std::string l_line;
        std::vector<std::string> l_row;
        while (getline(l_inputFile, l_line))
        {
            if (l_line.substr(0, 1) == "#")
                continue;
            tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);
            m_bathymetry->push_back(std::stof(l_row[3]));
        }
        l_inputFile.close();
        m_bathymetryDataSize = m_bathymetry->size();
    }
}

tsunami_lab::setups::TsunamiEvent1d::~TsunamiEvent1d()
{
    delete m_bathymetry;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getHeight(t_real i_x,
                                                                   t_real) const
{
    if (!m_fileExists)
    {
        return 0;
    }
    i_x /= 250;
    if (i_x <= (m_bathymetryDataSize - 1) && m_bathymetry->at(int(i_x)) < 0)
    {
        if (-(m_bathymetry->at(int(i_x))) < m_delta)
        {
            return m_delta;
        }
        else
        {
            return -(m_bathymetry->at(int(i_x)));
        }
    }
    else
    {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumX(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumY(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getBathymetry(t_real i_x,
                                                                       t_real) const
{
    if (!m_fileExists)
    {
        return 0;
    }
    i_x /= 250;
    if (i_x <= (m_bathymetryDataSize - 1))
    {
        t_real l_currBath = m_bathymetry->at(int(i_x));
        if (l_currBath < 0)
        {
            if (l_currBath < -m_delta)
            {
                return l_currBath + computeD(i_x, 0);
            }
            else
            {
                return -m_delta + computeD(i_x, 0);
            }
        }
        else
        {
            if (l_currBath > m_delta)
            {
                return l_currBath + computeD(i_x, 0);
            }
            else
            {
                return m_delta + computeD(i_x, 0);
            }
        }
    }
    else
    {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::computeD(t_real i_x,
                                                                  t_real) const
{
    i_x *= 250;
    if (i_x < 250000 && 175000 < i_x)
    {
        return 10 * sin(((i_x - 175000) / 37500) * m_pi + m_pi);
    }
    else
    {
        return 0;
    }
}
