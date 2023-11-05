/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * one-dimensional discontinuity problem
 **/
#ifndef TSUNAMI_LAB_SETUPS_GENERAL_DISCONTINUITY1D_H
#define TSUNAMI_LAB_SETUPS_GENERAL_DISCONTINUITY1D_H

#include "Setup.h"

namespace tsunami_lab
{
  namespace setups
  {
    class GeneralDiscontinuity1d;
  }
}

class tsunami_lab::setups::GeneralDiscontinuity1d : public Setup
{
private:
  //! height on the left side
  t_real m_heightLeft = 0;
  //! height on the right side
  t_real m_heightRight = 0;

  //! momentum on the left side
  t_real m_momentumLeft = 0;
  //! momentum on the right side
  t_real m_momentumRight = 0;

  //! wave discontinuity location on the x-axis
  t_real m_xdis = 0;

public:
  /**
   * Constructor.
   *
   * @param i_h water height of both sides
   * @param i_huL water momentum on the left side
   **/
  GeneralDiscontinuity1d(t_real i_hL,
                       t_real i_hR,
                       t_real i_huL,
                       t_real i_huR,
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
   * @param i_x x-coordinate of the queried point.
   * @return momentum in x-direction.
   **/
  t_real getMomentumX(t_real i_x,
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