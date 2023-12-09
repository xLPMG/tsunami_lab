/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Froude number calculation
 **/

#include "Froude.h"
#include <cmath>

void tsunami_lab::calculations::Froude::getMaxFroude(tsunami_lab::setups::Setup *i_setup,
                                                     t_idx i_domainSize,
                                                     t_real i_stepSize,
                                                     t_real &o_maxFroude,
                                                     t_real &o_maxFroudePosition)
{
    t_real l_maxFroude = 0;
    t_real l_posFroude = 0;
    t_real l_result = 0;
    if (i_stepSize > 0 && i_domainSize > 0 && i_stepSize <= i_domainSize)
    {
        for (t_real l_i = 0; l_i < i_domainSize; l_i += i_stepSize)
        {
            getFroude(i_setup,
                      l_i,
                      l_result);
            if (l_result > l_maxFroude)
            {
                l_maxFroude = l_result;
                l_posFroude = l_i;
            }
        }
    }
    o_maxFroude = l_maxFroude;
    o_maxFroudePosition = l_posFroude;
}

void tsunami_lab::calculations::Froude::getFroude(tsunami_lab::setups::Setup *i_setup,
                                                  t_real i_position,
                                                  t_real &o_Froude)
{
    t_real l_u = i_setup->getMomentumX(i_position, 0) / i_setup->getHeight(i_position, 0);
    t_real i_sqrt_m_h = t_real(std::sqrt(m_g * i_setup->getHeight(i_position, 0)));
    o_Froude = l_u / i_sqrt_m_h;
}