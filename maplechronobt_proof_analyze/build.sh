#!/bin/bash

EXEC_DIR="sources/simp"

echo "Compile"
cd $EXEC_DIR
make r -j
make ra -j
cd -

cd drat-trim
make -j
cd -


