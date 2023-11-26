/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional artificial tsunami
 **/

#include "TsunamiEvent2d.h"
#include "../io/NetCdf.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(const char *i_bathymetryPath,
                                                    const char *i_displacementPath,
                                                    tsunami_lab::io::NetCdf *i_netCdf,
                                                    t_idx i_stride)
{
    m_stride = i_stride;

    i_netCdf->read(i_bathymetryPath, "z", m_nxB, m_nyB, &m_xDataB, &m_yDataB, &m_b);
    i_netCdf->read(i_displacementPath, "z", m_nxD, m_nyD, &m_xDataD, &m_yDataD, &m_d);

    // find breaking point between negative and positive numbers (if it exists)
    // this is so for positive coordinates, we dont need to traverse the negative part of the array
    if (m_xDataB[0] < 0 && m_xDataB[m_nxB] > 0)
    {
        for (t_idx l_ix = 0; l_ix < m_nxB; l_ix++)
        {
            if (m_xDataB[l_ix - 1] < 0 && m_xDataB[l_ix] >= 0)
            {
                m_lastNegativeIndexBX = l_ix - 1;
                break;
            }
        }
    }
    if (m_yDataB[0] < 0 && m_yDataB[m_nyB] > 0)
    {
        for (t_idx l_iy = 0; l_iy < m_nyB; l_iy++)
        {
            if (m_yDataB[l_iy - 1] < 0 && m_yDataB[l_iy] >= 0)
            {
                m_lastNegativeIndexBY = l_iy - 1;
                break;
            }
        }
    }
    if (m_xDataD[0] < 0 && m_xDataD[m_nxD] > 0)
    {
        for (t_idx l_ix = 0; l_ix < m_nxD; l_ix++)
        {
            if (m_xDataD[l_ix - 1] < 0 && m_xDataD[l_ix] >= 0)
            {
                m_lastNegativeIndexDX = l_ix - 1;
                break;
            }
        }
    }
    if (m_yDataD[0] < 0 && m_yDataD[m_nyD] > 0)
    {
        for (t_idx l_iy = 0; l_iy < m_nyD; l_iy++)
        {
            if (m_yDataD[l_iy - 1] < 0 && m_yDataD[l_iy] >= 0)
            {
                m_lastNegativeIndexDY = l_iy - 1;
                break;
            }
        }
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetryFromArray(t_real i_x,
                                                                                t_real i_y) const
{
    bool l_setX = false;
    t_idx l_x = 0;

    t_idx l_ix = i_x >= 0 ? m_lastNegativeIndexBX : 0;
    for (; l_ix < m_nxB; l_ix++)
    {
        if (m_xDataB[l_ix] > i_x)
        {
            if (m_xDataB[l_ix] - i_x > m_xDataB[l_ix - 1] - i_x)
            {
                l_x = l_ix - 1;
                l_setX = true;
            }
            else
            {
                l_x = l_ix;
                l_setX = true;
            }
            break;
        }
    }

    bool l_setY = false;
    t_idx l_y = 0;
    t_idx l_iy = i_y >= 0 ? m_lastNegativeIndexBY : 0;
    for (; l_iy < m_nyB; l_iy++)
    {
        if (m_yDataB[l_iy] > i_y)
        {
            if (m_yDataB[l_iy] - i_x > m_yDataB[l_iy - 1] - i_y)
            {
                l_y = l_iy - 1;
                l_setY = true;
            }
            else
            {
                l_y = l_iy;
                l_setY = true;
            }
            break;
        }
    }
    if (l_setX + l_setY < 1)
    {
        return 0;
    }
    return m_b[l_x + m_nxB * l_y];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getDisplacementFromArray(t_real i_x,
                                                                                  t_real i_y) const
{
    bool l_setX = false;
    t_idx l_x = 0;
    t_idx l_ix = i_x >= 0 ? m_lastNegativeIndexDX : 0;
    for (; l_ix < m_nxD; l_ix++)
    {
        if (m_xDataD[l_ix] > i_x)
        {
            if (m_xDataD[l_ix] - i_x > m_xDataD[l_ix - 1] - i_x)
            {
                l_x = l_ix - 1;
                l_setX = true;
            }
            else
            {
                l_x = l_ix;
                l_setX = true;
            }
            break;
        }
    }

    bool l_setY = false;
    t_idx l_y = 0;
    t_idx l_iy = i_x >= 0 ? m_lastNegativeIndexDY : 0;
    for (; l_iy < m_nyD; l_iy++)
    {
        if (m_yDataD[l_iy] > i_y)
        {
            if (m_yDataD[l_iy] - i_x > m_yDataD[l_iy - 1] - i_y)
            {
                l_y = l_iy - 1;
                l_setY = true;
            }
            else
            {
                l_y = l_iy;
                l_setY = true;
            }
            break;
        }
    }
    if (l_setX + l_setY < 1)
    {
        return 0;
    }
    return m_b[l_x + m_nxB * l_y];
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
        return std::min(l_bath, m_delta) + l_displ;
    }
}