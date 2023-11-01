/**
 * @author Luca-Philipp Grumbach & Richard Hofmann 
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional rare-rare Riemann problems 
 **/

#include "MiddleStates.h"

tsunami_lab::setups::MiddleStates::MiddleStates( t_real i_hL,
                                                t_real i_hR, 
                                             t_real i_huL,
                                             t_real i_huR,
                                             t_real i_xdis ) {
    m_heightLeft = i_hL;
    m_heightRight = i_hR;
    m_momentumLeft = i_huL;
    m_momentumRight = i_huR;
    m_xdis = i_xdis;
}

tsunami_lab::t_real tsunami_lab::setups::MiddleStates::getHeight( t_real i_x,
                                                                t_real      ) const {
    return i_x < m_xdis ? m_heightLeft : m_heightRight;
}

tsunami_lab::t_real tsunami_lab::setups::MiddleStates::getMomentumX( t_real i_x,
                                                                   t_real ) const {
    return i_x < m_xdis ? m_momentumLeft : m_momentumRight;
}

tsunami_lab::t_real tsunami_lab::setups::MiddleStates::getMomentumY( t_real,
                                                                   t_real ) const {
    return 0;
}