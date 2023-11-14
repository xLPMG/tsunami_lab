/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * two-dimensional discontinuity problem
 **/

#include "GeneralDiscontinuity2d.h"

tsunami_lab::setups::GeneralDiscontinuity2d::GeneralDiscontinuity2d(t_real i_hL,
                                                                    t_real i_hR,
                                                                    t_real i_huXL,
                                                                    t_real i_huXR,
                                                                    t_real i_huYL,
                                                                    t_real i_huYR,
                                                                    t_real i_xdis,
                                                                    t_real i_ydis)
{
    m_heightLeft = i_hL;
    m_heightRight = i_hR;
    m_momentumXLeft = i_huXL;
    m_momentumXRight = i_huXR;
    m_momentumYLeft = i_huYL;
    m_momentumYRight = i_huYR;
    m_xdis = i_xdis;
    m_ydis = i_ydis;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity2d::getHeight(t_real i_x,
                                                                           t_real) const
{
    return i_x < m_xdis ? m_heightLeft : m_heightRight;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity2d::getMomentumX(t_real i_x,
                                                                              t_real) const
{
    return i_x < m_xdis ? m_momentumXLeft : m_momentumXRight;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity2d::getMomentumY(t_real i_x,
                                                                              t_real) const
{
    return i_x < m_ydis ? m_momentumYLeft : m_momentumYRight;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity2d::getBathymetry(t_real,
                                                                               t_real) const
{
    return 0;
}