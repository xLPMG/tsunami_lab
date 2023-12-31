/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * one-dimensional subcritical problem
 **/

#include "Subcritical1d.h"
#include <cmath>
#include <iostream>

tsunami_lab::setups::Subcritical1d::Subcritical1d(t_real i_h,
                                                  t_real i_hu)
{
    m_height = i_h;
    m_momentum = i_hu;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getHeight(t_real i_x,
                                                                  t_real) const
{
    if (i_x <= 25 && i_x >= 0)
    {
        return -getBathymetry(i_x, 0);
    }
    else
    {
        return m_height;
    }
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getMomentumX(t_real i_x,
                                                                     t_real) const
{
    if (i_x <= 25 && i_x >= 0)
    {
        return 4.42;
    }
    else
    {
        return m_momentum;
    }
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getMomentumY(t_real,
                                                                     t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getBathymetry(t_real i_x,
                                                                      t_real) const
{
    if (i_x < 12 && i_x > 8)
    {
        return -1.8 - (0.05 * (i_x - 10) * (i_x - 10));
    }
    else
    {
        return -2;
    }
}