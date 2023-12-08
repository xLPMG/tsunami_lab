/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * two-dimensional wave propagation patch.
 **/
#include "WavePropagation2d.h"
#include "../solvers/Roe.h"
#include "../solvers/Fwave.h"

tsunami_lab::patches::WavePropagation2d::WavePropagation2d(t_idx i_nCellsX,
                                                           t_idx i_nCellsY,
                                                           Boundary i_boundaryL,
                                                           Boundary i_boundaryR,
                                                           Boundary i_boundaryT,
                                                           Boundary i_boundaryB)
{
  m_nCellsX = i_nCellsX;
  m_nCellsY = i_nCellsY;
  m_boundaryL = i_boundaryL;
  m_boundaryR = i_boundaryR;
  m_boundaryT = i_boundaryT;
  m_boundaryB = i_boundaryB;

  // allocate memory including a single ghost cell on each side
  t_idx l_totalCells = (m_nCellsX + 2) * (m_nCellsY + 2);
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    m_h[l_st] = new t_real[l_totalCells];
    m_huX[l_st] = new t_real[l_totalCells];
    m_huY[l_st] = new t_real[l_totalCells];
  }
  m_b = new t_real[l_totalCells];

  // init to zero
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    for (t_idx l_ce = 0; l_ce < l_totalCells; l_ce++)
    {
      m_h[l_st][l_ce] = 0;
      m_huX[l_st][l_ce] = 0;
      m_huY[l_st][l_ce] = 0;
      m_b[l_ce] = 0;
    }
  }
}

tsunami_lab::patches::WavePropagation2d::~WavePropagation2d()
{
  for (unsigned short l_st = 0; l_st < 2; l_st++)
  {
    delete[] m_h[l_st];
    delete[] m_huX[l_st];
    delete[] m_huY[l_st];
  }
  delete[] m_b;
}

void tsunami_lab::patches::WavePropagation2d::timeStep(t_real i_scalingX,
                                                       t_real i_scalingY)
{
  // pointers to old and new data
  t_real *l_hOld = m_h[m_step];
  t_real *l_huOldX = m_huX[m_step];
  t_real *l_huOldY = m_huY[m_step];

  m_step = (m_step + 1) % 2;
  t_real *l_hNew = m_h[m_step];
  t_real *l_huNewX = m_huX[m_step];
  t_real *l_huNewY = m_huY[m_step];

  // init new cell quantities
  for (t_idx l_ce = 0; l_ce < (m_nCellsX + 2) * (m_nCellsY + 2); l_ce++)
  {
    l_hNew[l_ce] = l_hOld[l_ce];
    l_huNewX[l_ce] = l_huOldX[l_ce];
    l_huNewY[l_ce] = l_huOldY[l_ce];
  }

  // TODO: break nested loop into chunks to stay within cache

  // iterate over edges and update with Riemann solutions
  // X-SWEEP

  for (t_idx l_ed = 0; l_ed < m_nCellsY + 1; l_ed++)
  {
    for (t_idx l_ec = 1; l_ec < m_nCellsX; l_ec++)
    {
      // determine left and right cell-id
      t_idx l_ceL = l_ec + getStride() * l_ed;
      t_idx l_ceR = l_ceL + 1;

      t_real l_hL = l_hOld[l_ceL];
      t_real l_hR = l_hOld[l_ceR];
      t_real l_huL = l_huOldX[l_ceL];
      t_real l_huR = l_huOldX[l_ceR];
      t_real l_bL = m_b[l_ceL];
      t_real l_bR = m_b[l_ceR];

      // handle reflections
      handleReflections(l_hOld,
                        l_huOldX,
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

      if (l_hL == 0 && l_hR == 0)
        continue;
      solvers::Fwave::netUpdates(l_hL,
                                 l_hR,
                                 l_huL,
                                 l_huR,
                                 l_bL,
                                 l_bR,
                                 l_netUpdates[0],
                                 l_netUpdates[1]);

      if (l_hOld[l_ceL] > 0)
      {
        l_hNew[l_ceL] -= i_scalingX * l_netUpdates[0][0];
        l_huNewX[l_ceL] -= i_scalingX * l_netUpdates[0][1];
      }
      else
      {
        l_hNew[l_ceL] = 0;
        l_huNewX[l_ceL] = 0;
      }

      if (l_hOld[l_ceR] > 0)
      {
        l_hNew[l_ceR] -= i_scalingX * l_netUpdates[1][0];
        l_huNewX[l_ceR] -= i_scalingX * l_netUpdates[1][1];
      }
      else
      {
        l_hNew[l_ceR] = 0;
        l_huNewX[l_ceR] = 0;
      }
    }
  }

  // Y-SWEEP
  for (t_idx l_ec = 0; l_ec < m_nCellsY + 1; l_ec++)
  {
    for (t_idx l_ed = 1; l_ed < m_nCellsX; l_ed++)
    {
      // determine upper and lower cell-id
      t_idx l_ceB = l_ec * getStride() + l_ed;
      t_idx l_ceT = l_ceB + getStride();

      t_real l_hD = l_hOld[l_ceB];
      t_real l_hU = l_hOld[l_ceT];
      t_real l_huD = l_huOldY[l_ceB];
      t_real l_huU = l_huOldY[l_ceT];
      t_real l_bD = m_b[l_ceB];
      t_real l_bU = m_b[l_ceT];

      // handle reflections
      handleReflections(l_hOld,
                        l_huOldY,
                        l_ceB,
                        l_ceT,
                        l_hD,
                        l_hU,
                        l_huD,
                        l_huU,
                        l_bD,
                        l_bU);

      // compute net-updates
      t_real l_netUpdates[2][2];

      if (l_hD == 0 && l_hU == 0)
        continue;
      solvers::Fwave::netUpdates(l_hD,
                                 l_hU,
                                 l_huD,
                                 l_huU,
                                 l_bD,
                                 l_bU,
                                 l_netUpdates[0],
                                 l_netUpdates[1]);

      if (l_hOld[l_ceB] > 0)
      {
        l_hNew[l_ceB] -= i_scalingY * l_netUpdates[0][0];
        l_huNewY[l_ceB] -= i_scalingY * l_netUpdates[0][1];
      }
      else
      {
        l_hNew[l_ceB] = 0;
        l_huNewY[l_ceB] = 0;
      }

      if (l_hOld[l_ceT] > 0)
      {
        l_hNew[l_ceT] -= i_scalingY * l_netUpdates[1][0];
        l_huNewY[l_ceT] -= i_scalingY * l_netUpdates[1][1];
      }
      else
      {
        l_hNew[l_ceT] = 0;
        l_huNewY[l_ceT] = 0;
      }
    }
  }
}

void tsunami_lab::patches::WavePropagation2d::setGhostOutflow()
{
  t_real *l_h = m_h[m_step];
  t_real *l_huX = m_huX[m_step];
  t_real *l_huY = m_huY[m_step];
  t_real *l_b = m_b;

  for (t_idx i = 1; i < m_nCellsY; i++)
  {
    t_idx ceL = getStride() * i;
    t_idx ceR = ceL + m_nCellsX + 1;
    // left column
    l_h[ceL] = m_boundaryL == WALL ? 0 : l_h[ceL + 1];
    l_huX[ceL] = l_huX[ceL + 1];
    l_huY[ceL] = l_huY[ceL + 1];
    l_b[ceL] = l_b[ceL + 1];
    // right column
    l_h[ceR] = m_boundaryR == WALL ? 0 : l_h[ceR - 1];
    l_huX[ceR] = l_huX[ceR - 1];
    l_huY[ceR] = l_huY[ceR - 1];
    l_b[ceR] = l_b[ceR - 1];
  }
  for (t_idx i = 0; i < m_nCellsX + 2; i++)
  {
    t_idx ceB = i;
    t_idx ceT = i + (m_nCellsY + 1) * getStride();
    // bottom row
    l_h[ceB] = m_boundaryB == WALL ? 0 : l_h[ceB + getStride()];
    l_huX[ceB] = l_huX[ceB + getStride()];
    l_huY[ceB] = l_huY[ceB + getStride()];
    l_b[ceB] = l_b[ceB + getStride()];
    // top row
    l_h[ceT] = m_boundaryT == WALL ? 0 : l_h[ceT - getStride()];
    l_huX[ceT] = l_huX[ceT - getStride()];
    l_huY[ceT] = l_huY[ceT - getStride()];
    l_b[ceT] = l_b[ceT - getStride()];
  }
}

void tsunami_lab::patches::WavePropagation2d::handleReflections(t_real *i_h,
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
  // use margin for comparison in case of rounding errors
  tsunami_lab::t_real margin = 0.00001;
  if (i_h[i_ceR] <= margin)
  {
    // right cell dry
    o_hR = i_h[i_ceL];
    o_bR = m_b[i_ceL];
    o_huR = -i_hu[i_ceL];
  }
  else if (i_h[i_ceL] <= margin)
  {
    // left cell dry
    o_hL = i_h[i_ceR];
    o_bL = m_b[i_ceR];
    o_huL = -i_hu[i_ceR];
  }
}

void tsunami_lab::patches::WavePropagation2d::adjustWaterHeight()
{
  for (t_idx l_iy = 1; l_iy < m_nCellsY + 1; l_iy++)
  {
    for (t_idx l_ix = 1; l_ix < m_nCellsX + 1; l_ix++)
    {

      m_h[m_step][l_ix + l_iy * getStride()] -= m_b[l_ix + l_iy * getStride()];
      if (m_h[m_step][l_ix + l_iy * getStride()] < 0)
        m_h[m_step][l_ix + l_iy * getStride()] = 0;
    }
  }
}
