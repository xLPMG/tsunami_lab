/**
 * @author Luca Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Two-dimensional wave propagation patch.
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D

#include "WavePropagation.h"
#include <string>

namespace tsunami_lab
{
  namespace patches
  {
    class WavePropagation2d;
  }
}

class tsunami_lab::patches::WavePropagation2d : public WavePropagation
{
private:
  //! current step which indicates the active values in the arrays below
  unsigned short m_step = 0;

  //! number of cells discretizing the computational domain
  t_idx m_nCellsX = 0;

  t_idx m_nCellsY = 0;

  //! water heights for the current and next time step for all cells
  t_real *m_h[2] = {nullptr, nullptr};

  //! x momenta for the current and next time step for all cells
  t_real *m_huX[2] = {nullptr, nullptr};

  //! y momenta for the current and next time step for all cells
  t_real *m_huY[2] = {nullptr, nullptr};

  //! bathymetry 
  t_real *m_b = nullptr;

  //! selected solver (roe or fwave)
  std::string m_solver = "";

  //! true if there is a boundary on the left side
  bool m_hasBoundaryL = false;

  //! true if there is a boundary on the right side
  bool m_hasBoundaryR = false;

  //! true if there is a boundary on the top side
  bool m_hasBoundaryT = false;

  //! true if there is a boundary on the bottom side
  bool m_hasBoundaryB = false;

 /**
  * Compute the reflection effect
  * 
  * @param i_h water height
  * @param i_hu water momentum
  * @param i_ceL left cell
  * @param i_ceR right cell
  * @param o_hL water height on the left side
  * @param o_hR water height on the right side
  * @param o_huL water momentum on the left side
  * @param o_huR water momentum on the right side
  * @param o_bl bathymetry on the left side 
  * @param o_bR bathymetry on the right side
  */
  void handleReflections(t_real *i_h,
                         t_real *i_hu,
                         t_idx i_ceL,
                         t_idx i_ceR,
                         t_real &o_hL,
                         t_real &o_hR,
                         t_real &o_huL,
                         t_real &o_huR,
                         t_real &o_bL,
                         t_real &o_bR);

public:
  /**
   * Constructs the 2d wave propagation solver.
   *
   * @param i_nCellsX number of cells in x direction.
   * @param i_nCellsY number of cells in y direction.
   * @param i_solver selected solver.
   * @param i_hasBoundaryL has boundary on the left side
   * @param i_hasBoundaryR has boundary on the left side
   * @param i_hasBoundaryT has boundary on the top side
   * @param i_hasBoundaryB has boundary on the bottom side
   **/
  WavePropagation2d(t_idx i_nCellsX,
                    t_idx i_nCellsY,
                    const std::string &i_solver,
                    bool i_hasBoundaryL,
                    bool i_hasBoundaryR,
                    bool i_hasBoundaryT,
                    bool i_hasBoundaryB);

  /**
   * Destructor which frees all allocated memory.
   **/
  ~WavePropagation2d();

  /**
   * Performs a time step.
   *
   * @param i_scalingX scaling of the time step (dt / dx).
   * @param i_scalingY scaling of the time step (dt / dy).
   **/
  void timeStep(t_real i_scalingX,
                t_real i_scalingY);

  /**
   * Sets the values of the ghost cells according to outflow boundary conditions.
   **/
  void setGhostOutflow();

  /**
   * Gets the stride in y-direction. x-direction is stride-1.
   *
   * @return stride in y-direction.
   **/
  t_idx getStride()
  {
    return m_nCellsX + 2;
  }

  /**
   * Gets cells' water heights.
   *
   * @return water heights.
   */
  t_real const *getHeight()
  {
    return m_h[m_step] + 1 + getStride();
  }

  /**
   * Gets the cells' momenta in x-direction.
   *
   * @return momenta in x-direction.
   **/
  t_real const *getMomentumX()
  {
    return m_huX[m_step] + 1 + getStride();
  }

  /**
   * @return momenta in y-direction.
   **/
  t_real const *getMomentumY()
  {
    return m_huY[m_step] + 1 + getStride();
  }

  /**
   * Gets cells bathymetry
   *
   * @return bathymetry.
   */
  t_real const *getBathymetry()
  {
    return m_b + 1 + getStride();
  }

  /**
   * Sets the height of the cell to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_iy id of the cell in y-direction.
   * @param i_h water height.
   **/
  void setHeight(t_idx i_ix,
                 t_idx i_iy,
                 t_real i_h)
  {
    m_h[m_step][i_ix + 1 + (i_iy+1) * getStride()] = i_h;
  }

  /**
   * Sets the momentum in x-direction to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_iy id of the cell in y-direction.
   * @param i_hu momentum in x-direction.
   **/
  void setMomentumX(t_idx i_ix,
                    t_idx i_iy,
                    t_real i_huX)
  {
    m_huX[m_step][i_ix + 1 + (i_iy+1) * getStride()] = i_huX;
  }

  /**
   * Sets the momentum in y-direction to the given value.
   * @param i_ix id of the cell in x-direction.
   * @param i_iy id of the cell in y-direction.
   * @param i_hu momentum in y-direction.
   **/
  void setMomentumY(t_idx i_ix,
                    t_idx i_iy,
                    t_real i_huY)
  {
    m_huY[m_step][i_ix + 1 + (i_iy+1) * getStride()] = i_huY;
  };

  /**
   * Sets the bathymetry of the cell to the given value.
   *
   * @param i_ix id of the cell in x-direction.
   * @param i_iy id of the cell in y-direction.
   * @param i_h bathymetry.
   **/
  void setBathymetry(t_idx i_ix,
                     t_idx i_iy,
                     t_real i_b)
  {
    m_b[i_ix + 1 + (i_iy+1) * getStride()] = i_b;
  }

  /**
   * Corrects the water height based on the bathymetry
   *
   **/
  void adjustWaterHeight()
  {
    for (t_idx ix = 1; ix < m_nCellsX + 1; ix++)
    {
         for (t_idx iy = 1; iy < m_nCellsY + 1; iy++)
         {
                m_h[m_step][ix + iy * getStride()] -= m_b[ix + iy * getStride()];
                if (m_h[m_step][ix + iy * getStride()] < 0)
                    m_h[m_step][ix + iy * getStride()] = 0;
         }
    }
  }
};

#endif