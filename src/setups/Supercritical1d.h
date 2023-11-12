/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * One-dimensional supercritical problem
 **/
#ifndef TSUNAMI_LAB_SETUPS_SUPERCRITICAL_1D_H
#define TSUNAMI_LAB_SETUPS_SUPERCRITICAL_1D_H

#include "Setup.h"

namespace tsunami_lab
{
  namespace setups
  {
    class Supercritical1d;
  }
}

class tsunami_lab::setups::Supercritical1d : public Setup
{
private:
  //! height
  t_real m_height = 0;

  //! momentum
  t_real m_momentum = 0;

  //!gravity
  static t_real constexpr m_g = 9.80665;

public:
  /**
   * Constructor.
   *
   * @param i_h water height
   * @param i_hu water momentum
   **/
  Supercritical1d(t_real i_h,
                  t_real i_hu);

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

  /**
   * sets the maximum Froude number and the position and prints it at the end
   **/
  void setMaxFroude() const;
};

#endif