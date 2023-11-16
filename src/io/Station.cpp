/**
 * @author Luca Gru,bachh, Richard Hofmann
 *
 * @section DESCRIPTION
 * Stations 
 **/

#include "Station.h"
#include <sstream>

tsunami_lab::io::Station::Station(t_real i_x,
                                  t_real i_y,
                                  std::string i_name,
                                  t_real i_h,
                                  t_real i_b)
{
    m_x = i_x;
    m_y = i_y;
    m_name = i_name;
    m_h  =i_h;
    m_b = i_b;
}

tsunami_lab::io::Station::~Station(){};

void tsunami_lab::io::Station::setHeight(tsunami_lab::t_real i_h) {
    m_h = i_h;
}

void tsunami_lab::io::Station::setBathymetry(tsunami_lab::t_real i_b) {
    m_b = i_b;
}

tsunami_lab::t_real tsunami_lab::io::Station::getHeight()const{
    return m_h;
}

tsunami_lab::t_real tsunami_lab::io::Station::getBathymetry()const{
    return m_b;
}

