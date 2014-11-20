#!/bin/sh
./utils/compile.sh;
rm ./parallel_job.sh.*;
qrun.sh 4c $1 fast ./utils/parallel_job.sh;