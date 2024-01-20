/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Two-dimensional artificial tsunami
 **/

#include "TsunamiEvent2d.h"
#include "../io/NetCdf.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>

inline bool exists(const char *name)
{
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(const char *i_bathymetryPath,
                                                    const char *i_displacementPath,
                                                    tsunami_lab::io::NetCdf *i_netCdf,
                                                    t_idx i_stride)
{
    m_bathymetryPath = i_bathymetryPath;
    m_displacementPath = i_displacementPath;

    m_stride = i_stride;
    if (exists(i_bathymetryPath))
    {
        i_netCdf->getDimensionSize(i_bathymetryPath,
                                   "x",
                                   m_nxB);
        i_netCdf->getDimensionSize(i_bathymetryPath,
                                   "y",
                                   m_nyB);
    }
    if (exists(i_displacementPath))
    {
        i_netCdf->getDimensionSize(i_displacementPath,
                                   "x",
                                   m_nxD);
        i_netCdf->getDimensionSize(i_displacementPath,
                                   "y",
                                   m_nyD);
    }

    m_xDataB = new t_real[m_nxB];
    m_yDataB = new t_real[m_nyB];
    m_b = new t_real[m_nxB * m_nyB];
    if (exists(i_bathymetryPath))
    {
        i_netCdf->read(i_bathymetryPath,
                       "z",
                       &m_xDataB,
                       &m_yDataB,
                       &m_b);
    }

    m_xDataD = new t_real[m_nxD];
    m_yDataD = new t_real[m_nyD];
    m_d = new t_real[m_nxD * m_nyD];
    if (exists(i_displacementPath))
    {
        i_netCdf->read(i_displacementPath,
                       "z",
                       &m_xDataD,
                       &m_yDataD,
                       &m_d);
    }

    // find breaking point between negative and positive numbers (if it exists)
    // this is so for positive coordinates, we dont need to traverse the negative part of the array
    if (exists(i_bathymetryPath))
    {
        if (m_xDataB[0] < 0 && m_xDataB[m_nxB - 1] > 0)
        {
            for (t_idx l_ix = 1; l_ix < m_nxB; l_ix++)
            {
                if (m_xDataB[l_ix - 1] < 0 && m_xDataB[l_ix] >= 0)
                {
                    m_lastNegativeIndexBX = l_ix - 1;
                    break;
                }
            }
        }
        if (m_yDataB[0] < 0 && m_yDataB[m_nyB - 1] > 0)
        {
            for (t_idx l_iy = 1; l_iy < m_nyB; l_iy++)
            {
                if (m_yDataB[l_iy - 1] < 0 && m_yDataB[l_iy] >= 0)
                {
                    m_lastNegativeIndexBY = l_iy - 1;
                    break;
                }
            }
        }
    }
    if (exists(i_displacementPath))
    {
        if (m_xDataD[0] < 0 && m_xDataD[m_nxD - 1] > 0)
        {
            for (t_idx l_ix = 1; l_ix < m_nxD; l_ix++)
            {
                if (m_xDataD[l_ix - 1] < 0 && m_xDataD[l_ix] >= 0)
                {
                    m_lastNegativeIndexDX = l_ix - 1;
                    break;
                }
            }
        }
        if (m_yDataD[0] < 0 && m_yDataD[m_nyD - 1] > 0)
        {
            for (t_idx l_iy = 1; l_iy < m_nyD; l_iy++)
            {
                if (m_yDataD[l_iy - 1] < 0 && m_yDataD[l_iy] >= 0)
                {
                    m_lastNegativeIndexDY = l_iy - 1;
                    break;
                }
            }
        }
    }
}

tsunami_lab::setups::TsunamiEvent2d::~TsunamiEvent2d()
{
    delete[] m_xDataB;
    delete[] m_yDataB;
    delete[] m_b;

    delete[] m_xDataD;
    delete[] m_yDataD;
    delete[] m_d;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetryFromArray(t_real i_x,
                                                                                t_real i_y) const
{
    if (!exists(m_bathymetryPath))
        return 0;

    if (i_x < m_xDataB[0] || i_x > m_xDataB[m_nxB - 1] || i_y < m_yDataB[0] || i_y > m_yDataB[m_nyB - 1])
        return 0;

    t_idx l_x = 0;
    t_idx l_ix = i_x >= 0 ? m_lastNegativeIndexBX : 1;

    for (; l_ix < m_nxB; l_ix++)
    {
        if (m_xDataB[l_ix] >= i_x)
        {
            if (abs(m_xDataB[l_ix] - i_x) > abs(m_xDataB[l_ix - 1] - i_x))
            {
                l_x = l_ix - 1;
            }
            else
            {
                l_x = l_ix;
            }
            break;
        }
    }

    t_idx l_y = 0;
    t_idx l_iy = i_y >= 0 ? m_lastNegativeIndexBY : 1;

    for (; l_iy < m_nyB; l_iy++)
    {
        if (m_yDataB[l_iy] >= i_y)
        {
            if (abs(m_yDataB[l_iy] - i_y) > abs(m_yDataB[l_iy - 1] - i_y))
            {
                l_y = l_iy - 1;
            }
            else
            {
                l_y = l_iy;
            }
            break;
        }
    }
    return m_b[l_x + m_nxB * l_y];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getDisplacementFromArray(t_real i_x,
                                                                                  t_real i_y) const
{
    if (!exists(m_displacementPath))
        return 0;

    if (i_x < m_xDataD[0] || i_x > m_xDataD[m_nxD - 1] || i_y < m_yDataD[0] || i_y > m_yDataD[m_nyD - 1])
        return 0;

    t_idx l_x = 0;
    t_idx l_ix = i_x >= 0 ? m_lastNegativeIndexDX : 1;

    for (; l_ix < m_nxD; l_ix++)
    {

        if (m_xDataD[l_ix] >= i_x)
        {
            if (abs(m_xDataD[l_ix] - i_x) > abs(m_xDataD[l_ix - 1] - i_x))
            {
                l_x = l_ix - 1;
            }
            else
            {
                l_x = l_ix;
            }
            break;
        }
    }

    t_idx l_y = 0;
    t_idx l_iy = i_y >= 0 ? m_lastNegativeIndexDY : 1;
    for (; l_iy < m_nyD; l_iy++)
    {
        if (m_yDataD[l_iy] >= i_y)
        {
            if (abs(m_yDataD[l_iy] - i_y) > abs(m_yDataD[l_iy - 1] - i_y))
            {
                l_y = l_iy - 1;
            }
            else
            {
                l_y = l_iy;
            }
            break;
        }
    }
    return m_d[l_x + m_nxD * l_y];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real i_x,
                                                                   t_real i_y) const
{
    t_real l_bath = getBathymetryFromArray(i_x, i_y);
    if (l_bath < 0)
    {
        return (std::max(-l_bath, m_delta));
    }
    else
    {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumX(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumY(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry(t_real i_x,
                                                                       t_real i_y) const
{
    t_real l_bath = getBathymetryFromArray(i_x, i_y);
    t_real l_displ = getDisplacementFromArray(i_x, i_y);

    if (l_bath < 0)
    {
        return std::min(l_bath, -m_delta) + l_displ;
    }
    else
    {
        return std::max(l_bath, m_delta) + l_displ;
    }
}