/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional Tsunami event
 **/

#include "TsunamiEvent1d.h"
#include <math.h>
#include <vector>
#include "../io/Csv.h"
#include <iostream>

tsunami_lab::setups::TsunamiEvent1d::TsunamiEvent1d(const std::string &i_file,
                                                    t_real i_h,
                                                    t_real i_hu)
{
    m_height = i_h;
    m_momentum = i_hu;

    std::ifstream l_inputFile(i_file);
    m_bathymetry = new std::vector<tsunami_lab::t_real>;

    std::string l_line;
    std::vector<std::string> l_row;
    int l_rowCount = 0;
    while (getline(l_inputFile, l_line))
    {
        if (l_line.substr(0, 1) == "#")
            continue;
        tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);
        m_bathymetry->push_back(std::stof(l_row[3]));
        ++l_rowCount;
    }
    l_inputFile.close();
    m_bathymetryDataSize = l_rowCount;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getHeight(t_real i_x,
                                                                   t_real) const
{
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
        return m_height;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumX(t_real,
                                                                      t_real) const
{
    return m_momentum;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumY(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getBathymetry(t_real i_x,
                                                                       t_real) const
{
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
