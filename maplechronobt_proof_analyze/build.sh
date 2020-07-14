#!/bin/bash

EXEC_DIR="maplechronobt_proof_analyze/simp"

echo "Compile"
cd $EXEC_DIR
make r -j
make ra -j
cd -

cd drat-trim
make -j
cd -


