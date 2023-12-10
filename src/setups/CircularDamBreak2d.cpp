/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Two-dimensional circular dam break problem.
 **/

#include "CircularDamBreak2d.h"
#include <cmath>
#include <iostream>

tsunami_lab::setups::CircularDamBreak2d::CircularDamBreak2d()
{
}

tsunami_lab::t_real tsunami_lab::setups::CircularDamBreak2d::getHeight(t_real i_x,
                                                                       t_real i_y) const
{
    i_x-=50;
    i_y-=50;
    tsunami_lab::t_real sumOfSquares = i_x * i_x + i_y * i_y;
    return std::sqrt(sumOfSquares) < 10 ? 10 : 5;
}

tsunami_lab::t_real tsunami_lab::setups::CircularDamBreak2d::getMomentumX(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::CircularDamBreak2d::getMomentumY(t_real, t_real) const
{
    return 0;
}

tsunami_lab::t_real tsunami_lab::setups::CircularDamBreak2d::getBathymetry(t_real, t_real) const
{
    return 0;
}