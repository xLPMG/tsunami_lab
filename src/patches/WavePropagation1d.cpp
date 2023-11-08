/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * One-dimensional wave propagation patch.
 **/
#include "WavePropagation1d.h"
#include "../solvers/Roe.h"
#include "../solvers/Fwave.h"
#include <string>

tsunami_lab::patches::WavePropagation1d::WavePropagation1d(t_idx i_nCells,
                                                           std::string i_solver,
                                                           bool i_hasBoundaryL,
                                                           bool i_hasBoundaryR)
{
  m_nCells = i_nCells;
  m_solver = i_solver;
  m_hasBoundaryL = i_hasBoundaryL;
  m_hasBoundaryR = i_hasBoundaryR;

  // allocate memory including a single ghost cell on each side
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    m_h[l_st] = new t_real[m_nCells + 2];
    m_hu[l_st] = new t_real[m_nCells + 2];
  }
  m_b = new t_real[m_nCells + 2];

  // init to zero
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    for (t_idx l_ce = 0; l_ce < m_nCells + 2; l_ce++)
    {
      m_h[l_st][l_ce] = 0;
      m_hu[l_st][l_ce] = 0;
      m_b[l_ce] = 0;
    }
  }
}

tsunami_lab::patches::WavePropagation1d::~WavePropagation1d()
{
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    delete[] m_h[l_st];
    delete[] m_hu[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation1d::timeStep(t_real i_scaling)
{
  // pointers to old and new data
  t_real *l_hOld = m_h[m_step];
  t_real *l_huOld = m_hu[m_step];

  m_step = (m_step + 1) % 2;
  t_real *l_hNew = m_h[m_step];
  t_real *l_huNew = m_hu[m_step];

  // init new cell quantities
  for (t_idx l_ce = 1; l_ce < m_nCells + 1; l_ce++)
  {
    l_hNew[l_ce] = l_hOld[l_ce];
    l_huNew[l_ce] = l_huOld[l_ce];
  }

  // choose between Roe solver and Fwave solver
  if (m_solver == "roe")
  {

    // iterate over edges and update with Riemann solutions
    for (t_idx l_ed = 0; l_ed < m_nCells + 1; l_ed++)
    {
      // determine left and right cell-id
      t_idx l_ceL = l_ed;
      t_idx l_ceR = l_ed + 1;

      // compute net-updates
      t_real l_netUpdates[2][2];

      solvers::Roe::netUpdates(l_hOld[l_ceL],
                               l_hOld[l_ceR],
                               l_huOld[l_ceL],
                               l_huOld[l_ceR],
                               l_netUpdates[0],
                               l_netUpdates[1]);

      // update the cells' quantities
      l_hNew[l_ceL] -= i_scaling * l_netUpdates[0][0];
      l_huNew[l_ceL] -= i_scaling * l_netUpdates[0][1];

      l_hNew[l_ceR] -= i_scaling * l_netUpdates[1][0];
      l_huNew[l_ceR] -= i_scaling * l_netUpdates[1][1];
    }
  }
  else if (m_solver == "fwave")
  {
    // iterate over edges and update with Riemann solutions
    for (t_idx l_ed = 0; l_ed < m_nCells + 1; l_ed++)
    {
      // determine left and right cell-id
      t_idx l_ceL = l_ed;
      t_idx l_ceR = l_ed + 1;

      t_real l_hL;
			t_real l_hR;
			t_real l_huL;
			t_real l_huR;
      t_real l_bL;
			t_real l_bR;

      // handle reflections
      handleReflections(l_hOld,
                        l_huOld,
                        l_ceL,
                        l_ceR,
                        l_hL,
                        l_hR,
                        l_huL,
                        l_huR,
                        l_bL,
                        l_bR);

      // compute net-updates
      t_real l_netUpdates[2][2];

      solvers::Fwave::netUpdates(l_hL,
                                 l_hR,
                                 l_huL,
                                 l_huR,
                                 l_bL,
                                 l_bR,
                                 l_netUpdates[0],
                                 l_netUpdates[1]);

      // update the cells' quantities
      tsunami_lab::t_real margin = 0.001;
      if(l_hL > margin){
      l_hNew[l_ceL] -= i_scaling * l_netUpdates[0][0];
      l_huNew[l_ceL] -= i_scaling * l_netUpdates[0][1];
      } else {
        l_huNew[l_ceL] = 0;
        l_hNew[l_ceL] = 0;
      }

      if(l_hR > margin){
      l_hNew[l_ceR] -= i_scaling * l_netUpdates[1][0];
      l_huNew[l_ceR] -= i_scaling * l_netUpdates[1][1];
      } else {
        l_huNew[l_ceR] = 0;
        l_hNew[l_ceR] = 0;
      }
    }
  }
}

void tsunami_lab::patches::WavePropagation1d::setGhostOutflow()
{
  t_real *l_h = m_h[m_step];
  t_real *l_hu = m_hu[m_step];
  t_real *l_b = m_b;

	// set left boundary
  if(m_hasBoundaryL) l_h[0] = 0;
	else l_h[0] = l_h[1];
	l_hu[0] = l_hu[1];
	m_b[0] = m_b[1];

	// set right boundary
	if(m_hasBoundaryR) l_h[m_nCells + 1] = 0;
  else l_h[m_nCells + 1] = l_h[m_nCells];
	l_hu[m_nCells + 1] = l_hu[m_nCells];
	m_b[m_nCells + 1] = m_b[m_nCells];
}

void tsunami_lab::patches::WavePropagation1d::handleReflections(t_real *i_h,
                                                                t_real *i_hu,
                                                                t_idx i_ceL,
                                                                t_idx i_ceR,
                                                                t_real &o_hL,
                                                                t_real &o_hR,
                                                                t_real &o_huL,
                                                                t_real &o_huR,
                                                                t_real &o_bL,
                                                                t_real &o_bR)
{
  // comparing a float to 0.0f is bad practice because of possible rounding errors
  tsunami_lab::t_real margin = 0.001;
  // left cell is dry
  // if (i_h[i_ceL] <= margin)
  // {
  //   o_hL = i_h[i_ceR];
  //   o_huL = -i_hu[i_ceR];
  //   o_bL = m_b[i_ceR];
  // }
  // else
  // {
  //   o_hL = i_h[i_ceL];
  //   o_huL = -i_hu[i_ceL];
  //   o_bL = m_b[i_ceL];
  // }

  // // right cell is dry
  // if (i_h[i_ceR] <= margin)
  // {
  //   o_hR = i_h[i_ceL];
  //   o_huR = -i_hu[i_ceL];
  //   o_bR = m_b[i_ceL];
  // }
  // else
  // {
  //   o_hR = i_h[i_ceR];
  //   o_huR = -i_hu[i_ceR];
  //   o_bR = m_b[i_ceR];
  // }

    o_hL = i_h[i_ceL];
    o_huL = i_hu[i_ceL];
    o_bL = m_b[i_ceL];
    o_hR = i_h[i_ceR];
    o_huR = i_hu[i_ceR];
    o_bR = m_b[i_ceR];
}
