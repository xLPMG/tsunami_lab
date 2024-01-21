#!/bin/bash
cat <<EoF
#!/bin/bash
#SBATCH --job-name=$1
#SBATCH --output=$2
#SBATCH --error=$3
#SBATCH --time=$4

echo STARTING SERVER
./build/tsunami_lab \$1
EoF
