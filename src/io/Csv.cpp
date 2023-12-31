/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#include "Csv.h"
#include <sstream>

void tsunami_lab::io::Csv::write(t_real i_dx,
                                 t_real i_dy,
                                 t_idx i_nx,
                                 t_idx i_ny,
                                 t_idx i_stride,
                                 t_real const *i_h,
                                 t_real const *i_hu,
                                 t_real const *i_hv,
                                 t_real const *i_b,
                                 std::ostream &io_stream)
{
  // write the CSV header
  io_stream << "x,y";
  if (i_h != nullptr)
    io_stream << ",height";
  if (i_hu != nullptr)
    io_stream << ",momentum_x";
  if (i_hv != nullptr)
    io_stream << ",momentum_y";
  if (i_b != nullptr)
    io_stream << ",bathymetry";
  if (i_h != nullptr && i_b != nullptr)
    io_stream << ",totalHeight";
  io_stream << "\n";

  // iterate over all cells
  for (t_idx l_iy = 0; l_iy < i_ny; l_iy++)
  {
    for (t_idx l_ix = 0; l_ix < i_nx; l_ix++)
    {
      // derive coordinates of cell center
      t_real l_posX = (l_ix + t_real(0.5)) * i_dx;
      t_real l_posY = (l_iy + t_real(0.5)) * i_dy;

      t_idx l_id = l_iy * i_stride + l_ix;

      // write data
      io_stream << l_posX << "," << l_posY;
      if (i_h != nullptr)
        io_stream << "," << i_h[l_id];
      if (i_hu != nullptr)
        io_stream << "," << i_hu[l_id];
      if (i_hv != nullptr)
        io_stream << "," << i_hv[l_id];
      if (i_b != nullptr)
        io_stream << "," << i_b[l_id];
      if (i_h != nullptr && i_b != nullptr)
        io_stream << "," << i_h[l_id]+i_b[l_id];
      io_stream << "\n";
    }
  }
  io_stream << std::flush;
}

void tsunami_lab::io::Csv::splitLine(std::stringstream line,
                                     char separator,
                                     std::vector<std::string> &valuesVector)
{
  std::vector<std::string> result;
  std::string word;
  while (getline(line, word, separator))
    result.push_back(word);
  valuesVector = result;
}