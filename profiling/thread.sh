#!/bin/bash
#SBATCH --job-name=thread_scaling
#SBATCH --output=thread32.out
#SBATCH --error=thread32.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=32
#SBATCH --time=05:00:00

export OMP_NUM_THREADS=32

mpirun ../src/trading_simulator