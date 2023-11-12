/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * one-dimensional discontinuity problem
 **/

#include "GeneralDiscontinuity1d.h"

tsunami_lab::setups::GeneralDiscontinuity1d::GeneralDiscontinuity1d(t_real i_hL,
                                                                    t_real i_hR,
                                                                    t_real i_huL,
                                                                    t_real i_huR,
                                                                    t_real i_xdis)
{
    m_heightLeft = i_hL;
    m_heightRight = i_hR;
    m_momentumLeft = i_huL;
    m_momentumRight = i_huR;
    m_xdis = i_xdis;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity1d::getHeight(t_real i_x,
                                                                           t_real) const
{
    return i_x < m_xdis ? m_heightLeft : m_heightRight;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity1d::getMomentumX(t_real i_x,
                                                                              t_real) const
{
    return i_x < m_xdis ? m_momentumLeft : m_momentumRight;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity1d::getMomentumY(t_real,
                                                                              t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::GeneralDiscontinuity1d::getBathymetry(t_real,
                                                                               t_real) const
{
    return 0;
}