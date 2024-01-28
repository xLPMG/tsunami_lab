/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
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
  // Initial height of the water
  t_real m_height = 0;
  // radius of the circular wave
  t_real m_radius = 0;
  // water height we return if position ist not in the circle
  t_real m_baseHeight = 0;

public:
  /**
   * Constructor.
   * @param i_height height of the water
   * @param i_diameter diameter of the circular wave
   **/
  CircularDamBreak2d(tsunami_lab::t_real i_height,
                     tsunami_lab::t_real i_baseHeight,
                     tsunami_lab::t_real i_diameter);

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