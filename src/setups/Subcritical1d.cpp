/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
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
    setMaxFroude();
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

void tsunami_lab::setups::Subcritical1d::setMaxFroude() const
{
    t_real l_maxFroude = 0;
    t_real l_posFroude = 0;
    for (t_real l_i = 0; l_i < 25; l_i += 0.1)
    {
        if (0 < l_i && l_i < 25)
        {
            t_real l_u = getMomentumX(l_i, 0) / getHeight(l_i, 0);
            t_real i_sqrt_m_h = t_real(std::sqrt(m_g * getHeight(l_i, 0)));
            t_real l_result = l_u / i_sqrt_m_h;
            if (l_result > l_maxFroude)
            {
                l_maxFroude = l_result;
                l_posFroude = l_i;
            }
        }
    }
    std::cout << "max Froude number for subcritical: " << l_maxFroude << " at " << l_posFroude << std::endl;
}