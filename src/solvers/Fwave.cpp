/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Fwave
 **/
#include "Fwave.h"
#include <cmath>
#include <iostream>

void tsunami_lab::solvers::Fwave::computeEigenvalues( t_real   i_hL,
                                            t_real   i_hR,
                                            t_real   i_uL,
                                            t_real   i_uR,
                                            t_real & eigenvalue_roe_1,
                                            t_real & eigenvalue_roe_2 ) {
  // pre-compute square-root ops
  t_real l_hSqrtL = std::sqrt( i_hL );
  t_real l_hSqrtR = std::sqrt( i_hR );

  // compute Roe averages
  t_real l_hRoe = 0.5f * ( i_hL + i_hR );
  t_real l_uRoe = l_hSqrtL * i_uL + l_hSqrtR * i_uR;
  l_uRoe /= l_hSqrtL + l_hSqrtR;

  // compute eigenvalues
  t_real l_ghSqrtRoe = m_gSqrt * std::sqrt( l_hRoe );
  eigenvalue_roe_1 = l_uRoe - l_ghSqrtRoe;
  eigenvalue_roe_2 = l_uRoe + l_ghSqrtRoe;
}

void tsunami_lab::solvers::Fwave::computeEigencoefficients( t_real   i_hL,
                                               t_real   i_hR,
                                               t_real   i_huL,
                                               t_real   i_huR,
                                               t_real   eigenvalue_roe_1,
                                               t_real   eigenvalue_roe_2,
                                               t_real & alpha_1,
                                               t_real & alpha_2 ) {  
  // compute inverse of right eigenvector-matrix
  t_real l_detInv = 1 / (eigenvalue_roe_2 - eigenvalue_roe_1);

  t_real l_rInv[2][2] = {{0}};
  l_rInv[0][0] =  l_detInv * eigenvalue_roe_2;
  l_rInv[0][1] = -l_detInv;
  l_rInv[1][0] = -l_detInv * eigenvalue_roe_1;
  l_rInv[1][1] =  l_detInv;

  // compute particle velocities
  t_real l_uL = i_huL / i_hL;
  t_real l_uR = i_huR / i_hR;

  //compute f difference
  t_real f_diff[2] = {0};
  f_diff[0] = i_huR - i_huL;
  f_diff[1] = (i_huR*l_uR+0.5*m_g*i_hR*i_hR)-(i_huL*l_uL+0.5*m_g*i_hL*i_hL);
  std::cout << "F diff: " << f_diff[0] << ", " << f_diff[1] << std::endl;

  // compute alpha
  alpha_1 = l_rInv[0][0] * f_diff[0] + l_rInv[0][1] * f_diff[1];
  alpha_2  = l_rInv[1][0] * f_diff[0] + l_rInv[1][1] * f_diff[1];
}

void tsunami_lab::solvers::Fwave::netUpdates( t_real i_hL,
                                            t_real i_hR,
                                            t_real i_huL,
                                            t_real i_huR,
                                            t_real o_netUpdateL[2],
                                            t_real o_netUpdateR[2] ) {

// compute particle velocities
t_real l_uL = i_huL / i_hL;
t_real l_uR = i_huR / i_hR;

//compute eigenvalues
t_real eigenvalue_roe_1 = 0;
t_real eigenvalue_roe_2 = 0;

computeEigenvalues(i_hL,
              i_hR,
              l_uL,
              l_uR,
              eigenvalue_roe_1,
              eigenvalue_roe_2);

// compute eigenvectors
t_real eigenvector_roe_1[2] = {1, eigenvalue_roe_1};
t_real eigenvector_roe_2[2] = {1, eigenvalue_roe_2};

//compute eigencoefficients
t_real eigencoefficient_roe_1 = 0;
t_real eigencoefficient_roe_2 = 0;

computeEigencoefficients(i_hL,
                 i_hR,
                 i_huL,
                 i_huR,
                 eigenvalue_roe_1,
                 eigenvalue_roe_2,
                 eigencoefficient_roe_1,
                 eigencoefficient_roe_2);

  t_real z1[2] = {0};
  z1[0] = eigencoefficient_roe_1 * eigenvector_roe_1[0];
  z1[1] = eigencoefficient_roe_1 * eigenvector_roe_1[1];

  t_real z2[2] = {0};
  z2[0] = eigencoefficient_roe_2 * eigenvector_roe_2[0];
  z2[1] = eigencoefficient_roe_2 * eigenvector_roe_2[1];

  std::cout << "Z1: " << z1[0] << ", " << z1[1] << std::endl;
  std::cout << "Z2: " << z2[0] << ", " << z2[1] << std::endl;
  std::cout << "Sum Z: " << z1[0]+z2[0] << ", " << z1[1]+z2[1] << std::endl;

  // set net-updates depending on wave speeds
  for( unsigned short l_qt = 0; l_qt < 2; l_qt++ ) {
    // init
    o_netUpdateL[l_qt] = 0;
    o_netUpdateR[l_qt] = 0;

    // 1st wave
    if( eigenvalue_roe_1 < 0 ) {
      o_netUpdateL[l_qt] += z1[l_qt];
    }
    else {
      o_netUpdateR[l_qt] += z1[l_qt];
    }

    // 2nd wave
    if( eigenvalue_roe_2 < 0 ) {
      o_netUpdateR[l_qt] += z2[l_qt];
    }
    else {
      o_netUpdateL[l_qt] += z2[l_qt];
    }
  }

}
