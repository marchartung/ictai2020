#!/bin/bash

ANA_DIR="ana_ssd_fast"
STD_DIR="std_ssd_fast"
PICO_DIR="picosat_ssd"



#generate csv files:

grep -h "OVERVIEW_CSV" $ANA_DIR/* > ana_raw.csv
python filter_ictai_output2020.py ana_raw.csv > ana.csv

grep -h "OVERVIEW_CSV" $STD_DIR/* > std_raw.csv
python filter_ictai_output2020.py std_raw.csv > std.csv

grep -h "OVERVIEW_CSV" $PICO_DIR/* > pico_raw.csv
python filter_ictai_output2020.py pico_raw.csv > pico.csv

# create runtime gnuplots:
python3 print_first_proof_runtime.py ana.csv > ana_first_proof.csv
python3 print_first_proof_runtime.py std.csv > std_first_proof.csv
python3 print_first_proof_runtime.py pico.csv > pico_first_proof.csv
gnuplot -e "outname='runtime_first_proof.pdf'" time_to_first_proof.gnu
cp runtime_first_proof.pdf ../figures


# create proof gnuplots:
python print_percent_proof_time_ana.py ana.csv > ana_percent_proof.csv
gnuplot -e "outname='percent_ana.pdf'" percent_proof_ana.gnu
python print_percent_proof_time_std.py std.csv > std_percent_proof.csv
gnuplot -e "outname='percent_std.pdf'" percent_proof_std.gnu

python3 print_cmp_overhead_proof.py > cmp_proof.csv
gnuplot -e "outname='proof_overhead.pdf'" proof_overhead2.gnu
cp proof_overhead.pdf ../figures

# create drat vs disk memory

# create solve time overhead
#python create_overhead_cmp.py > ana_std_overhead.csv
#gnuplot ana_std_overhead.gnu

# create disk memory overhead
python3 create_disk_drat_ana_trace.py ana.csv > ana_std_disk_overhead.csv
gnuplot -e "outname='drat_vs_disk.pdf'" create_disk_drat_ana_trace.gnu
cp drat_vs_disk.pdf ../figures

python3 create_disk_complete.py ana.csv > ana_std_disk_complete.csv
gnuplot -e "outname='disk_complete.pdf'" create_disk_complete_lrat.gnu


# generate data for discussion
cd discussed_examples
grep -h "OVERVIEW_CSV" ana/* > ana_raw.csv
python ../filter_ictai_output2020.py ana_raw.csv > ana.csv

grep -h "OVERVIEW_CSV" std/* > std_raw.csv
python ../filter_ictai_output2020.py std_raw.csv > std.csv

python prep_table_data.py > bench_discussion.txt
cp bench_discussion.txt ../figures


cd ..
