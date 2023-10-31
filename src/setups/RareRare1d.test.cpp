/**
 * @author Luca-Philipp Grumbach & Richard Hofmann 
 *
 * @section DESCRIPTION
 * Test for the implementation of rare-rare Riemann problems 
 **/

#include <catch2/catch.hpp>
#include "RareRare1d.h"

TEST_CASE( "Test the one-dimensional rare-rare setup.", "[RareRare1d]" ) {
  tsunami_lab::setups::RareRare1d l_RareRare( 20,
                                              10,
                                               5 );
  // left side
  REQUIRE( l_RareRare.getHeight( 2, 0 ) == 20 );

  REQUIRE( l_RareRare.getMomentumX( 2, 0 ) == -10 );

  REQUIRE( l_RareRare.getMomentumY( 2, 0 ) == 0 );

  REQUIRE( l_RareRare.getHeight( 2, 5 ) == 25 );

  REQUIRE( l_RareRare.getMomentumX( 2, 5 ) == -10 );

  REQUIRE( l_RareRare.getMomentumY( 2, 2 ) == 0 );

  // right side
  REQUIRE( l_RareRare.getHeight( 8, 0 ) == 20 );

  REQUIRE( l_RareRare.getMomentumX( 8, 0 ) == 10 );

  REQUIRE( l_RareRare.getMomentumY( 8, 0 ) == 0 );

  REQUIRE( l_RareRare.getHeight( 8, 5 ) == 20 );

  REQUIRE( l_RareRare.getMomentumX( 8, 5 ) == 10 );

  REQUIRE( l_RareRare.getMomentumY( 8, 2 ) == 0 );  
}

