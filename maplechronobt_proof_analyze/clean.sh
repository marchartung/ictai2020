#!/bin/bash

EXEC_DIR="sources/simp"

cd $EXEC_DIR
make clean
cd -
cd drat-trim
make clean
cd -

