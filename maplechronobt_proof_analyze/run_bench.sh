#!/bin/bash

if [ -z "$1" ]
then
    echo "No temporary directory passed as argument"
    exit -1
fi

if [ -z "$2" ]
then
    echo "No benchmark given"
    exit -1
fi

BENCH="$2"
EXEC_DIR="sources/simp"
DRAT_EXEC="drat-trim/drat-trim"

TMP_DIR="$1"
PROOF_ANA="$TMP_DIR/proof_ana.out"
PROOF_STD="$TMP_DIR/proof_standard.out"

echo "Running benchmark '$BENCH' on directory '$TMP_DIR'"

echo "Run Analyze"
$EXEC_DIR/glucose_release_ana -drup-file="$PROOF_ANA" -analyzer-tmp-file="maplemultiuip_analyzer_file" $BENCH
du -sh $PROOF_ANA
$DRAT_EXEC $BENCH $PROOF_ANA

echo "Run Standard"
$EXEC_DIR/glucose_release -drup-file="$PROOF_STD" $BENCH
du -sh $PROOF_STD
$DRAT_EXEC $BENCH $PROOF_STD


echo "Clean up"

rm $PROOF_ANA
rm $PROOF_STD

# cd $EXEC_DIR
# make clean
# cd -
# cd drat-trim
# make clean
# cd -

