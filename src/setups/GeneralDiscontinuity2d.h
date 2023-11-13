/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional discontinuity problem
 **/
#ifndef TSUNAMI_LAB_SETUPS_GENERAL_DISCONTINUITY_2D_H
#define TSUNAMI_LAB_SETUPS_GENERAL_DISCONTINUITY_2D_H

#include "Setup.h"

namespace tsunami_lab
{
  namespace setups
  {
    class GeneralDiscontinuity2d;
  }
}

class tsunami_lab::setups::GeneralDiscontinuity2d : public Setup
{
private:
  //! height on the left side
  t_real m_heightLeft = 0;
  //! height on the right side
  t_real m_heightRight = 0;

  //! momentum on the left side in x-direction
  t_real m_momentumXLeft = 0;
  //! momentum on the right side in x-direction
  t_real m_momentumXRight = 0;
    //! momentum on the left side in y-direction
  t_real m_momentumYLeft = 0;
  //! momentum on the right side in y-direction
  t_real m_momentumYRight = 0;

  //! wave discontinuity location on the x-axis
  t_real m_xdis = 0;

public:
  /**
   * Constructor.
   *
   * @param i_hL water height on the left side
   * @param i_hR water height on the right side
   * @param i_huXL water momentum on the left side in x-direction
   * @param i_huXR water momentum on the right side in x-direction
   * @param i_huYL water momentum on the left side in y-direction
   * @param i_huYR water momentum on the right sidein y-direction
   * @param i_xdis position of discontinuity
   **/
  GeneralDiscontinuity2d(t_real i_hL,
                         t_real i_hR,
                         t_real i_huXL,
                         t_real i_huXR,
                         t_real i_huYL,
                         t_real i_huYR,
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

  /**
   * Gets the bathymetry
   *
   * @param i_x x-coordinate of the queried point.
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real i_x,
                       t_real) const;
};

#endif