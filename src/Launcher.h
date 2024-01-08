#ifndef TSUNAMI_LAB_LAUNCHER_H
#define TSUNAMI_LAB_LAUNCHER_H

// waveprop patches
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"

// setups
#include "setups/DamBreak1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/RareRare1d.h"
#include "setups/ShockShock1d.h"
#include "setups/Subcritical1d.h"
#include "setups/Supercritical1d.h"
#include "setups/GeneralDiscontinuity1d.h"
#include "setups/TsunamiEvent1d.h"
#include "setups/TsunamiEvent2d.h"
#include "setups/ArtificialTsunami2d.h"

// io
#include "io/Csv.h"
#include "io/BathymetryLoader.h"
#include "io/Station.h"
#include "io/NetCdf.h"

// external libraries
#include <nlohmann/json.hpp>
#include <netcdf.h>

#include <string>
using json = nlohmann::json;
using Boundary = tsunami_lab::patches::WavePropagation::Boundary;

namespace tsunami_lab
{
    class Launcher;
}

class tsunami_lab::Launcher
{
public:
    int start();
};

#endif
