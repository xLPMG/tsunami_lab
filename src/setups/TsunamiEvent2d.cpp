/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional artificial tsunami
 **/

#include "TsunamiEvent2d.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>

tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(t_real * i_b,
                                                    t_idx i_stride)
{
    t_real * m_b = i_b;
    t_idx m_stride = i_stride;
    m_artificial = new tsunami_lab::setups::ArtificialTsunami2d();   
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real i_x, t_real i_y) const
{
    if(m_b[i_x + i_y * m_stride]<0){
        return (std::max(-m_b,m_delta));
    } else {
        return 0;
    }
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumX(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getMomentumY(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry(t_real i_x, t_real i_y) const
{
    
    if(m_b[i_x + i_y * m_stride]<0){
        return std::min(m_b[i_x + i_y * m_stride],-m_delta) + m_artificial->computeD(i_x,i_y);
    } else {
        return std::min(m_b[i_x + i_y * m_stride],m_delta) + m_artificial->computeD(i_x,i_y);
    }
}