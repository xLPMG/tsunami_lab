/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional artificial tsunami
 **/

#include "ArtificialTsunami2d.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>

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
    t_real l_bathOriginal = -100;
    return l_bathOriginal + computeD(i_x, i_y);
}
