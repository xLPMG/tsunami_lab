/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Two-dimensional artificial tsunami
 **/

#include "ArtificialTsunami2d.h"
#include <cmath>
#include <cstdlib>

tsunami_lab::setups::ArtificialTsunami2d::ArtificialTsunami2d(){};

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::computeD(t_real i_x,
                                                                       t_real i_y) const
{
    if (i_x >= -500 && i_x <= 500 && i_y >= -500 && i_y <= 500)
    {
        return (5 * computeF(i_x) * computeG(i_y));
    }
    else
    {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::computeF(t_real i_x) const
{
    return (sin((i_x / 500 + 1) * m_pi));
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::computeG(t_real i_y) const
{
    return (-((i_y / 500) * (i_y / 500)) + 1);
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getHeight(t_real, t_real) const
{
    return 100;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumX(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumY(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getBathymetry(t_real i_x, t_real i_y) const
{
    return -100 + computeD(i_x, i_y);
}
