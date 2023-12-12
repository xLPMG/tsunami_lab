#!/bin/bash
#SBATCH --job-name=tohoku_5000
#SBATCH --output=tohoku_5000.out
#SBATCH --error=tohoku_5000.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

./build/tsunami_lab configs/tohoku5000.json