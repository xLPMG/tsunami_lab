/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * one-dimensional supercritical problem
 **/

#include "Supercritical1d.h"
#include <cmath>
#include <iostream>

tsunami_lab::setups::Supercritical1d::Supercritical1d(t_real i_h,
                                                      t_real i_hu)
{
    m_height = i_h;
    m_momentum = i_hu;
    setMax();
}

tsunami_lab::t_real tsunami_lab::setups::Supercritical1d::getHeight(t_real i_x,
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

tsunami_lab::t_real tsunami_lab::setups::Supercritical1d::getMomentumX(t_real i_x,
                                                                       t_real) const
{
    if (i_x <= 25 && i_x >= 0)
    {
        return 0.18;
    }
    else
    {
        return m_momentum;
    }
}

tsunami_lab::t_real tsunami_lab::setups::Supercritical1d::getMomentumY(t_real,
                                                                       t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::Supercritical1d::getBathymetry(t_real i_x,
                                                                        t_real) const
{
    if (i_x < 12 && i_x > 8)
    {
        return -0.13 - 0.05 * (i_x - 10) * (i_x - 10);
    }
    else
    {
        return -0.33;
    }
}

void tsunami_lab::setups::Supercritical1d::setMax() const
{
    t_real l_maxFroude = 0;
    t_real l_posFroude = 0;
    for (t_real l_i = 0; l_i < 25; l_i += 0.1)
    {
        if (0 < l_i &&  l_i < 25)
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
    std::cout << "max Froude number for supercritical: " << l_maxFroude << " at " << l_posFroude << std::endl;
}