/**
 * @author Maximilian Grumbacher & Richard Hofmann
 *
 * @section DESCRIPTION
 * Implementation of shock-shock Riemann problems
 **/

#ifndef TSUNAMI_LAB_SETUPS_SHOCK_SHOCK_H
#define TSUNAMI_LAB_SETUPS_SHOCK_SHOCK_H

#include "Setup.h"

namespace tsunami_lab
{
  namespace setups
  {
    class ShockShock1d;
  }
}

class tsunami_lab::setups::ShockShock1d : public Setup
{
private:
  //! height on the left side
  t_real m_height = 0;

  //! height on the right side
  t_real m_momentumLeft = 0;

  //! position where waves interact
  t_real m_xdis = 0;

public:
  /**
   * Constructor.
   *
   * @param i_h water height of both sides
   * @param i_huL water momentum on the left side
   **/
  ShockShock1d(t_real i_h,
               t_real i_huL,
               t_real i_xdis);

  /**
   * Gets the water height at a given point.
   *
   * @param i_x x-coordinate of the queried point.
   * @return height at the given point.
   **/
  t_real getHeight(t_real i_x,
                   t_real) const;

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
};

#endif