#!/bin/bash
#SBATCH --job-name=strong_scaling
#SBATCH --output=papi8_4.out
#SBATCH --error=papi8_4.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=4
#SBATCH --time=05:00:00

export OMP_NUM_THREADS=4

mpirun ../src/trading_simulator