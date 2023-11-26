/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional circular dam break problem.
 **/
#ifndef TSUNAMI_LAB_SETUPS_CIRCULAR_DAM_BREAK_2D_H
#define TSUNAMI_LAB_SETUPS_CIRCULAR_DAM_BREAK_2D_H

#include "Setup.h"

namespace tsunami_lab
{
  namespace setups
  {
    class CircularDamBreak2d;
  }
}

/**
 * 2d dam break setup.
 **/
class tsunami_lab::setups::CircularDamBreak2d : public Setup
{
private:

public:
  /**
   * Constructor.
   *
   **/
  CircularDamBreak2d();

  /**
   * Gets the water height at a given point.
   * @param i_x x-coordinate of the queried point.
   * @param i_y y-coordinate of the queried point.
   * @return height at the given point.
   **/
  t_real getHeight(t_real i_x,
                   t_real i_y) const;

  /**
   * Gets the momentum in x-direction.
   * 
   * @return momentum in x-direction.
   **/
  t_real getMomentumX(t_real,
                      t_real) const;

  /**
   * Gets the momentum in y-direction.
   *
   * @return momentum in y-direction.
   **/
  t_real getMomentumY(t_real,
                      t_real) const;

  /**
   * Gets the bathymetry
   *
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real,
                       t_real) const;
};

#endif