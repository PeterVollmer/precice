#!/bin/bash

#@ job_name = ateles/cube
#@ job_type = MPICH

#@ initialdir = $(home)

###@ output = job.out
###@ error  = job.err

#@ wall_clock_limit = 00:25:00

#@ network.MPI = sn_all,not_shared,us

#@ class          = general
#@ island_count   = 1
#@ node           = 128
#@ tasks_per_node = 16
#@ queue

 distribution_type='point-to-point'
communication_type='sockets'

export configuration="${distribution_type}-${communication_type}"
export       LEFT_NP='1024'
export      RIGHT_NP='1024'

SCENARIO_DIR="${HOME}/scenarios/ateles/cube"
     RUN_DIR="${HOME}/runs/ateles/cube"
   # RUN_DIR="${WORK}/runs/ateles/cube"

. "${SCENARIO_DIR}/ateles.intel.bash"
