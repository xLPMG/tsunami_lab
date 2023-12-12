#!/bin/bash
#SBATCH --job-name=chile_5000
#SBATCH --output=chile_5000.out
#SBATCH --error=chile_5000.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

#load modules
module load compiler/gcc/11.2.0

# Enter your executable commands here
# Execute the compiled program
scons mode=benchmark opt=-O2
./build/tsunami_lab configs/chile5000.json