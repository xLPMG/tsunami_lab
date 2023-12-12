#!/bin/bash
#SBATCH --job-name=tohoku_1000
#SBATCH --output=tohoku_1000.out
#SBATCH --error=tohoku_1000.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

./build/tsunami_lab configs/tohoku1000.json