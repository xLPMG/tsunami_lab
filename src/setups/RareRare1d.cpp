/**
 * @author Luca-Philipp Grumbach & Richard Hofmann 
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional rare-rare Riemann problems 
 **/

#include "RareRare1d.h"

tsunami_lab::setups::RareRare1d::RareRare1d( t_real i_h, 
                                             t_real i_huL,
                                             t_real i_xdis ) {
    m_height = i_h;
    m_momentumLeft = i_huL;
    m_xdis = i_xdis;
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getHeight( t_real,
                                                                t_real      ) const {
    return m_height;
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getMomentumX( t_real i_x,
                                                                   t_real ) const {
    return i_x <= m_xdis ? -m_momentumLeft : m_momentumLeft;
}

tsunami_lab::t_real tsunami_lab::setups::RareRare1d::getMomentumY( t_real,
                                                                   t_real ) const {
    return 0;
}