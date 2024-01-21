#!/bin/bash
#SBATCH --job-name=tsunami_lab_auto
#SBATCH --output=outpu23t.txt
#SBATCH --error=erro23r.txt
#SBATCH --time=10:00:23

echo STARTING SERVER
./build/tsunami_lab server $1
