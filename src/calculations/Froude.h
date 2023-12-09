/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Froude number calculation
 **/
#ifndef FROUDE_H
#define FROUDE_H

#include "../setups/Setup.h"
#include "../constants.h"

namespace tsunami_lab
{
    namespace calculations
    {
        class Froude;
    }
}

class tsunami_lab::calculations::Froude
{
private:
    //! gravity
    static t_real constexpr m_g = 9.80665;

public:
    /**
     * Gets the maximum Froude number within a given domain.
     * 
     *@param i_setup setup for which the Froude number will be calculated for
     *@param i_domainSize size of the domain in which the max Fraude number will be searched for
     *@param i_stepSize accuracy of the data queries towards th setup
     *@param o_maxFroude maximum Froude number output
     *@param o_maxFroudePosition position of the maximum Froude number
     **/
    static void getMaxFroude(tsunami_lab::setups::Setup *i_setup,
                             t_idx i_domainSize,
                             t_real i_stepSize,
                             t_real &o_maxFroude,
                             t_real &o_maxFroudePosition);

    /**
     * Gets the Fraude number for a specified location.
     * 
     *@param i_setup setup for which the Froude number will be calculated for
     *@param i_position location of the queried point
     *@param o_Froude Froude number output
     **/
    static void getFroude(tsunami_lab::setups::Setup *i_setup,
                          t_real i_position,
                          t_real &o_Froude);
};
#endif