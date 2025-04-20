#!/bin/bash

RESULTS_TMP=$(mktemp)

for alpha0 in $(seq 0.5 0.1 1.5); do
  for alpha1 in $(seq 3.0 0.1 12.0); do
    echo -ne "$alpha0:$alpha1:\t"
    output=$(scripts/run_vad.sh $alpha0 $alpha1 | grep "TOTAL")
    echo -e "$alpha0:$alpha1:\t$output" >> "$RESULTS_TMP"
    echo "$output"
  done
done

echo
echo "TOP 10 COMBINACIONS segons F-score TOTAL:"
sort -t ":" -k 3nr "$RESULTS_TMP" | head -n 10

# Opcional: desa resultats complets
cp "$RESULTS_TMP" scripts/resultats_alpha_totals.txt
rm "$RESULTS_TMP"
