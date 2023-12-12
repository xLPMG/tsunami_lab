#!/bin/bash
#SBATCH --job-name=chile_5000
#SBATCH --output=chile_5000.out
#SBATCH --error=chile_5000.err
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

./build/tsunami_lab configs/chile5000.json