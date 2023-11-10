/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * one-dimensional subcritical problem
 **/

#include "Subcritical1d.h"

tsunami_lab::setups::Subcritical1d::Subcritical1d(t_real i_h,
                                                   t_real i_hu)
{
    m_height = i_h;
    m_momentum = i_hu;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getHeight(t_real i_x,
                                                                  t_real) const
{   
    if(i_x <= 25 && i_x >= 0) return -getBathymetry(i_x, 0);
    return m_height;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getMomentumX(t_real i_x,
                                                                     t_real) const
{
    if(i_x <= 25 && i_x >= 0) return 4.42;
    return m_momentum;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getMomentumY(t_real,
                                                                     t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::Subcritical1d::getBathymetry(t_real i_x,
                                                                      t_real) const
{
    if(i_x < 12 && i_x > 8) return -1.8 - 0.05 * (i_x - 10) * (i_x - 10);
    return -2;
}