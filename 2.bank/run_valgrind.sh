#!/bin/bash

# Remove old file if it exists
rm -f check.txt

# Loop 100 times
for i in {1..100}; do
    echo "===== Run #$i =====" >> check.txt
    valgrind --leak-check=full --track-origins=yes ./bank ATM1.txt ATM2.txt ATM3.txt ATM4.txt ATM5.txt >> check.txt 2>&1
    echo -e "\n\n" >> check.txt
done