#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh

# Establecemos que el código de retorno de un pipeline sea el del último programa con código de retorno
# distinto de cero, o cero si todos devuelven cero.
set -o pipefail

alpha0=${1:-5.0}
alpha1=${2:-3.0}

# Write here the name and path of your program and database
DIR_P2=$HOME/PAV/P2
DB=$DIR_P2/db.v4
CMD="$DIR_P2/bin/vad"  # Aquí assignem finalment la comanda

# Loop per tots els .wav de la base de dades
for filewav in $DB/*/*wav; do
    echo "**************** $filewav ****************"

    if [[ ! -f $filewav ]]; then 
        echo "Wav file not found: $filewav" >&2
        exit 1
    fi

    filevad=${filewav/.wav/.vad}
    $CMD -i "$filewav" -o "$filevad" $alpha0 $alpha1 || exit 1
done


# Avaluació
scripts/vad_evaluation.pl $DB/*/*lab

exit 0
