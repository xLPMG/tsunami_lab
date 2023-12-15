#!/bin/bash
#SBATCH --job-name=vTune
#SBATCH --output=vTune.out
#SBATCH --error=vTune.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

module load compiler/intel/2020-Update2

/cluster/intel/vtune_profiler_2020.2.0.610396/bin64/vtune -collect hotspots -app-working-dir /beegfs/xe63nel/tsunami_lab/build -- /beegfs/xe63nel/tsunami_lab/build/tsunami_lab ../configs/config.json