/**
 * @author Maximilian Grumbacher & Richard Hofmann 
 *
 * @section DESCRIPTION
 * Test for the implementation of shock-shock Riemann problems
 **/

#include <catch2/catch.hpp>
#include "ShockShock1d.h"

TEST_CASE( "Test the one-dimensional shock shock setup.", "[ShockShock]" ) {
  tsunami_lab::setups::ShockShock1d l_ShockShock( 20,
                                                  10,
                                                  5 );

  // left side
  REQUIRE( l_ShockShock.getHeight( 2, 0 ) == 20 );

  REQUIRE( l_ShockShock.getMomentumX( 2, 0 ) == 10 );

  REQUIRE( l_ShockShock.getMomentumY( 2, 0 ) == 0 );

  REQUIRE( l_ShockShock.getHeight( 2, 5 ) == 20 );

  REQUIRE( l_ShockShock.getMomentumX( 2, 5 ) == 10 );

  REQUIRE( l_ShockShock.getMomentumY( 2, 2 ) == 0 );

  // right side
  REQUIRE( l_ShockShock.getHeight( 8, 0 ) == 20 );

  REQUIRE( l_ShockShock.getMomentumX( 8, 0 ) == -10 );

  REQUIRE( l_ShockShock.getMomentumY( 8, 0 ) == 0 );

  REQUIRE( l_ShockShock.getHeight( 8, 5 ) == 20 );

  REQUIRE( l_ShockShock.getMomentumX( 8, 5 ) == -10 );

  REQUIRE( l_ShockShock.getMomentumY( 4, 2 ) == 0 );  
}


 