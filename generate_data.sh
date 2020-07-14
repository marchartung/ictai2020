#!/bin/bash
echo "generating data in figures. For raw data please see the *.out files in the misc/ subdirectories"
cd misc
bash generate_graphs_from_raw.sh
cd -
