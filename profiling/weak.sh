#!/bin/bash
#SBATCH --job-name=weak_scaling
#SBATCH --output=weak9.out
#SBATCH --error=weak9.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=9
#SBATCH --cpus-per-task=4
#SBATCH --time=05:00:00

export OMP_NUM_THREADS=4

mpirun ../src/trading_simulator