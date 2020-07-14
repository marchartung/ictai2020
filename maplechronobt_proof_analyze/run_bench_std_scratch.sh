#!/bin/bash

if [ -z "$1" ]
then
    echo "No benchmark given"
    exit -1
fi

BENCH="$1"
EXEC_DIR="sources/simp"
DRAT_EXEC="drat-trim/drat-trim"
HOSTN=`hostname`

TMP_DIR="/nfs/scratch/bzchartu"
PROOF_DRAT_STD="$TMP_DIR/$HOSTN_proof_standard.drat"
PROOF_LRAT_STD="$TMP_DIR/$HOSTN_proof_standard.lrat"

echo "Running standard on $HOSTN the benchmark '$BENCH' Drat: '$PROOF_DRAT_STD' Lrat: '$PROOF_LRAT_STD'"

$EXEC_DIR/glucose_release -drup-file="$PROOF_DRAT_STD" $BENCH
du -sh $PROOF_STD
$DRAT_EXEC $BENCH $PROOF_DRAT_STD -L $PROOF_LRAT_STD
rm $PROOF_DRAT_STD
du -sh $PROOF_LRAT_STD


echo "Clean up"

rm $PROOF_STD

# cd $EXEC_DIR
# make clean
# cd -
# cd drat-trim
# make clean
# cd -

