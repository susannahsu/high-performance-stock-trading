#!/bin/bash
#SBATCH --job-name=strong_scaling
#SBATCH --output=strong9.out
#SBATCH --error=strong9.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=9
#SBATCH --cpus-per-task=4
#SBATCH --time=05:00:00

export OMP_NUM_THREADS=4

mpirun ../src/trading_simulator