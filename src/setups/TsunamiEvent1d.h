/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Implementation of One-dimensional Tsunami event
 **/

#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H

#include "Setup.h"
#include "../constants.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace tsunami_lab
{
  namespace setups
  {
    class TsunamiEvent1d;
  }
}

class tsunami_lab::setups::TsunamiEvent1d : public Setup
{
private:
  //! height on the left side
  t_real m_height = 0;

  //! height on the right side
  t_real m_momentumLeft = 0;

  //! position where waves interact
  t_real m_xdis = 0;

  //! stores the bathymetry data from the file
  std::vector<tsunami_lab::t_real> *m_bathymetry = nullptr;

  //! keeps track of the bathymetry data vector size
  int m_bathymetryDataSize = 0;

  //! constant for pi
  static t_real constexpr m_pi = 3.14159265358979323846;

  //
  static t_real constexpr m_delta = 20;

public:
  /**
   * Constructor
   *
   **/
  TsunamiEvent1d(std::string i_file);

  /**
   * Gets the water height at a given point.
   *
   * @param i_x x-coordinate of the queried point.
   * @return height at the given point.
   **/
  t_real getHeight(t_real,
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

  /**
   * Gets the bathymetry
   *
   * @param i_x x-coordinate of the queried point.
   * @return bathymetry.
   **/
  t_real getBathymetry(t_real i_x,
                       t_real) const;

  /**
   * computes 
   *
   * @return momentum in y-direction.
   **/
  t_real computeD(t_real i_x,
                  t_real) const;
};
#endif