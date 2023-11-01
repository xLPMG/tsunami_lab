/**
 * @author Luca-Philipp Grumbach & Richard Hofmann 
 *
 * @section DESCRIPTION
 * Implementation of rare-rare Riemann problems 
 **/
#ifndef TSUNAMI_LAB_SETUPS_MIDDLE_STATES_H
#define TSUNAMI_LAB_SETUPS_MIDDLE_STATES_H

#include "Setup.h"

namespace tsunami_lab {
  namespace setups {
    class MiddleStates;
  }
}

class tsunami_lab::setups::MiddleStates: public Setup {
  private:
    //! height on the left side 
    t_real m_heightLeft = 0;
    //! height on the left side 
    t_real m_heightRight = 0;
    
    //! height on the right side
    t_real m_momentumLeft = 0;
     //! height on the right side
    t_real m_momentumRight = 0;

    //! position where waves interact
    t_real m_xdis = 0;

  public:
    /**
     * Constructor.
     *
     * @param i_h water height of both sides
     * @param i_huL water momentum on the left side
     **/
    MiddleStates( t_real i_hL,
                  t_real i_hR, 
                  t_real i_huL,
                  t_real i_huR,
                  t_real i_xdis );

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point.
     **/
    t_real getHeight( t_real i_x,
                      t_real      ) const;

    /**
     * Gets the momentum in x-direction.
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real i_x,
                         t_real ) const;

    /**
     * Gets the momentum in y-direction.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real,
                         t_real ) const;
};

#endif