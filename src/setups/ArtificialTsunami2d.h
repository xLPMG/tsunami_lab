/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Two-dimensional artificial tsunami event
 **/
#ifndef TSUNAMI_LAB_SETUPS_ARTIFICIAL_TSUNAMI_2D
#define TSUNAMI_LAB_SETUPS_ARTIFICIAL_TSUNAMI_2D

#include "Setup.h"
#include <cmath>

namespace tsunami_lab
{
  namespace setups
  {
    class ArtificialTsunami2d;
  }
}

/**
 * 2d dam break setup.
 **/
class tsunami_lab::setups::ArtificialTsunami2d : public Setup
{
private:
  static t_real constexpr m_pi = 3.1415926;

public:
  /**
   * Constructor.
   *
   **/
  ArtificialTsunami2d();

  /**
   * compute the displacement
   * 
   * @param i_x postion in x direction
   * @param i_y postion in y direction
   * @return displacement
   */
  t_real computeD(t_real i_x,
                  t_real i_y) const;

  /**
   * compute equation for f
   * 
   * @param i_x postion in x direction
   * @return value for f
   */
  t_real computeF(t_real i_x) const;

  /**
   * compute equation for g
   * 
   * @param i_y postion in y direction
   * @return value for g
   */
  t_real computeG(t_real i_y) const;

  /**
   * Gets the water height at a given point.
   *
   * @param i_x x-coordinate of the queried point.
   * @param i_y y-coordinate of the queried point.
   * @return water height at the given point.
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
   * Gets the bathymetry.
   *
   * @param i_x x-coordinate of the queried point.
   * @param i_y y-coordinate of the queried point.
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real i_x,
                       t_real i_y) const;
};

#endif