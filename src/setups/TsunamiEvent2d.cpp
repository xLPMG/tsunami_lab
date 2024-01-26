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
#include <algorithm>

using netcdf = tsunami_lab::io::NetCdf;

inline bool exists(const char *name)
{
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(const char *i_bathymetryPath,
                                                    const char *i_displacementPath,
                                                    t_real &o_domainSizeX,
                                                    t_real &o_domainSizeY,
                                                    t_real &o_offsetX,
                                                    t_real &o_offsetY)
{
    m_bathymetryPath = i_bathymetryPath;
    m_displacementPath = i_displacementPath;

    if (exists(i_bathymetryPath))
    {
        netcdf::getDimensionSize(i_bathymetryPath,
                                 "x",
                                 m_nxB);
        netcdf::getDimensionSize(i_bathymetryPath,
                                 "y",
                                 m_nyB);
    }
    if (exists(i_displacementPath))
    {
        netcdf::getDimensionSize(i_displacementPath,
                                 "x",
                                 m_nxD);
        netcdf::getDimensionSize(i_displacementPath,
                                 "y",
                                 m_nyD);
    }

    m_xDataB = new t_real[m_nxB];
    m_yDataB = new t_real[m_nyB];
    m_b = new t_real[m_nxB * m_nyB];
    if (exists(i_bathymetryPath))
    {
        netcdf::read(i_bathymetryPath,
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
        netcdf::read(i_displacementPath,
                     "z",
                     &m_xDataD,
                     &m_yDataD,
                     &m_d);
    }

    m_bathymetryOffsetX = m_xDataB[0];
    m_bathymetryOffsetY = m_yDataB[0];
    m_displacementOffsetX = m_xDataD[0];
    m_displacementOffsetY = m_yDataD[0];

    m_bathymetrySampleDistanceX = m_xDataB[1] - m_xDataB[0];
    m_bathymetrySampleDistanceY = m_yDataB[1] - m_yDataB[0];
    m_displacementSampleDistanceX = m_xDataD[1] - m_xDataD[0];
    m_displacementSampleDistanceY = m_yDataD[1] - m_yDataD[0];

    m_bathymetrySampleDistanceXInverse = 1 / m_bathymetrySampleDistanceX;
    m_bathymetrySampleDistanceYInverse = 1 / m_bathymetrySampleDistanceY;
    m_displacementSampleDistanceXInverse = 1 / m_displacementSampleDistanceX;
    m_displacementSampleDistanceYInverse = 1 / m_displacementSampleDistanceY;

    o_domainSizeX = m_xDataB[m_nxB - 1] - m_xDataB[0];
    o_domainSizeY = m_yDataB[m_nyB - 1] - m_yDataB[0];
    o_offsetX = m_xDataB[0];
    o_offsetY = m_yDataB[0];
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

    t_real l_x = (i_x - m_bathymetryOffsetX) * m_bathymetrySampleDistanceXInverse;
    t_real l_y = (i_y - m_bathymetryOffsetY) * m_bathymetrySampleDistanceYInverse;
    t_idx l_xRounded = std::max(0.0, (l_x + 0.5 - (l_x < 0)));
    t_idx l_yRounded = std::max(0.0, (l_y + 0.5 - (l_y < 0)));

    if (l_xRounded >= m_nxB)
        l_xRounded = m_nxB - 1;
    if (l_yRounded >= m_nyB)
        l_yRounded = m_nyB - 1;

    return m_b[l_xRounded + m_nxB * l_yRounded];
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getDisplacementFromArray(t_real i_x,
                                                                                  t_real i_y) const
{
    if (!exists(m_displacementPath))
        return 0;

    t_real l_x = (i_x - m_displacementOffsetX) * m_displacementSampleDistanceXInverse;
    t_real l_y = (i_y - m_displacementOffsetY) * m_displacementSampleDistanceYInverse;
    int l_xRounded = l_x + 0.5 - (l_x < 0);
    int l_yRounded = l_y + 0.5 - (l_y < 0);

    if (l_xRounded < 0 || l_yRounded < 0)
        return 0;

    if (t_idx(l_xRounded) >= m_nxD || t_idx(l_yRounded) >= m_nyD)
        return 0;

    return m_d[t_idx(l_xRounded) + m_nxD * t_idx(l_yRounded)];
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