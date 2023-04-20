#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $SCRIPT_DIR

BIN_LIST=$(echo "bin1.fa bin2.fa bin3.fa bin4.fa"$foo{1..16})

echo "$BIN_LIST"

rm ./raptor_23_19.index

./raptor build --window 23 --kmer 19 --size 8k --hash 2 --output raptor_23_19.index ${BIN_LIST}
