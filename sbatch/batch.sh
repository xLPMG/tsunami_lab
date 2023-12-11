#!/bin/bash
#SBATCH --job-name=tohoku_1000
#SBATCH --output=tohoku_1000.out
#SBATCH --error=tohoku_1000.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

#load modules
module load compiler/gcc/11.2.0

# Enter your executable commands here
# Execute the compiled program
scons
./build/tsunami_lab