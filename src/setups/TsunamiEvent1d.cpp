/**
 * @author Luca Grombacher, Richard Hofmann
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional Tsunami event
 **/

#include "TsunamiEvent1d.h"
#include <math.h>    

tsunami_lab::setups::TsunamiEvent1d::TsunamiEvent1d(std::string i_file)
{
    std::ifstream l_inputFile(i_file);

    std::string l_line;
    while(getline(l_inputFile, l_line) == " "){
        
    };
}

tsunami_lab::setups::TsunamiEvent1d::~TsunamiEvent1d()
{
 delete[] m_bathymetry;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getHeight(t_real i_x,
                                                                   t_real) const
{   
    return m_height;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumX(t_real i_x,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::getMomentumY(t_real,
                                                                      t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent1d::computeD(t_real i_x,
                                                                      t_real) const
{
    if(175000 < i_x < 250000){
        10 * sin(((i_x-175000)/37500)*m_pi+m_pi);
    } else {
        return 0;
    }
}
