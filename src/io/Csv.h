/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#ifndef TSUNAMI_LAB_IO_CSV
#define TSUNAMI_LAB_IO_CSV

#include "../constants.h"
#include <cstring>
#include <iostream>
#include <vector>


namespace tsunami_lab {
  namespace io {
    class Csv;
  }
}

class tsunami_lab::io::Csv {
  public:
    /**
     * Writes the data as CSV to the given stream.
     *
     * @param i_dx cell width in x-direction.
     * @param i_dy cell width in y-direction.
     * @param i_nx number of cells in x-direction.
     * @param i_ny number of cells in y-direction.
     * @param i_stride stride of the data arrays in y-direction (x is assumed to be stride-1).
     * @param i_h water height of the cells; optional: use nullptr if not required.
     * @param i_hu momentum in x-direction of the cells; optional: use nullptr if not required.
     * @param i_hv momentum in y-direction of the cells; optional: use nullptr if not required.
     * @param i_b bathymetry; optional: use nullptr if not required.
     * @param io_stream stream to which the CSV-data is written.
     **/
    static void write( t_real              i_dx,
                       t_real              i_dy,
                       t_idx               i_nx,
                       t_idx               i_ny,
                       t_idx               i_stride,
                       t_real       const *i_h,
                       t_real       const *i_hu,
                       t_real       const *i_hv,
                       t_real       const *i_b,
                       std::ostream       &io_stream);

    /**
     * Splits a CSV-style line into a vector of strings.
     *
     * @param line input line as stringstream
     * @param separator character which separates the words (will be used to split)
     * @param valuesVector a vector of stringsto which the words of the csv line are written.
     **/
    static void splitLine(std::stringstream line, 
                          char separator,
                          std::vector<std::string> &valuesVector);
};

#endif