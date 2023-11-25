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

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(const char *bathymetryPath,
                                                    const char *displacementPath,
                                                    t_idx i_stride)
{
    m_stride = i_stride;

    tsunami_lab::io::NetCdf *l_netCdf = new tsunami_lab::io::NetCdf(100,
                                                                    100);
    m_b = l_netCdf->read(bathymetryPath, "z");
    m_d = l_netCdf->read(displacementPath, "z");
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real i_x,
                                                                   t_real i_y) const
{
    if (m_b[t_idx(i_x + i_y * m_stride)] < 0)
    {
        return (std::max(-m_b[t_idx(i_x + i_y * m_stride)], m_delta));
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
    if (m_b[t_idx(i_x + i_y * m_stride)] < 0)
    {
        return std::min(m_b[t_idx(i_x + i_y * m_stride)], -m_delta) + m_d[t_idx(i_x + i_y * m_stride)];
    }
    else
    {
        return std::min(m_b[t_idx(i_x + i_y * m_stride)], m_delta) + m_d[t_idx(i_x + i_y * m_stride)];
    }
}