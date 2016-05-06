#/bin/bash

million=1000000
billion=100000000

./datagen 4_1Million 4 "$million"
./datagen 8_1Million 8 "$million"
./datagen 9_1Million 9 "$million"
./datagen 12_1Million 12 "$million"
./datagen 16_1Million 16 "$million"
./datagen 32_1Million 32 "$million"


echo datagen billions

./datagen 4_1Billion 4 "$billion"
./datagen 8_1Billion 8 "$billion"
./datagen 9_1Billion 9 "$billion"
./datagen 12_1Billion 12 "$billion"
./datagen 16_1Billion 16 "$billion"
./datagen 32_1Billion 32 "$billion"


echo 4Bits
~/vis_scan/4bits 4_1Million 5 >> 4_Million_Results
~/vis_scan/4bits 4_1Million 5 >> 4_Million_Results
~/vis_scan/4bits 4_1Million 5 >> 4_Million_Results
~/vis_scan/4bits 4_1Million 5 >> 4_Million_Results
~/vis_scan/4bits 4_1Million 5 >> 4_Million_Results

echo 8Bits
~/vis_scan/8bits 8_1Million 50 >> 8_Million_Results
~/vis_scan/8bits 8_1Million 50 >> 8_Million_Results
~/vis_scan/8bits 8_1Million 50 >> 8_Million_Results
~/vis_scan/8bits 8_1Million 50 >> 8_Million_Results
~/vis_scan/8bits 8_1Million 50 >> 8_Million_Results

echo 9Bits
~/vis_scan/9bits 9_1Million 100 >> 9_Million_Results
~/vis_scan/9bits 9_1Million 100 >> 9_Million_Results
~/vis_scan/9bits 9_1Million 100 >> 9_Million_Results
~/vis_scan/9bits 9_1Million 100 >> 9_Million_Results
~/vis_scan/9bits 9_1Million 100 >> 9_Million_Results

echo 12Bits
~/vis_scan/12bits 12_1Million 110 >> 12_Million_Results
~/vis_scan/12bits 12_1Million 110 >> 12_Million_Results
~/vis_scan/12bits 12_1Million 110 >> 12_Million_Results
~/vis_scan/12bits 12_1Million 110 >> 12_Million_Results
~/vis_scan/12bits 12_1Million 110 >> 12_Million_Results

echo 16Bits
~/vis_scan/16bits 16_1Million 200 >> 16_Million_Results
~/vis_scan/16bits 16_1Million 200 >> 16_Million_Results
~/vis_scan/16bits 16_1Million 200 >> 16_Million_Results
~/vis_scan/16bits 16_1Million 200 >> 16_Million_Results
~/vis_scan/16bits 16_1Million 200 >> 16_Million_Results

echo 32Bits
~/vis_scan/32bits 32_1Million 10000 >> 32_Million_Results
~/vis_scan/32bits 32_1Million 10000 >> 32_Million_Results
~/vis_scan/32bits 32_1Million 10000 >> 32_Million_Results
~/vis_scan/32bits 32_1Million 10000 >> 32_Million_Results
~/vis_scan/32bits 32_1Million 10000 >> 32_Million_Results

echo 4Bil
~/vis_scan/4bits 4_1Billion 5 >> 4_Billion_Results
~/vis_scan/4bits 4_1Billion 5 >> 4_Billion_Results
~/vis_scan/4bits 4_1Billion 5 >> 4_Billion_Results
~/vis_scan/4bits 4_1Billion 5 >> 4_Billion_Results
~/vis_scan/4bits 4_1Billion 5 >> 4_Billion_Results

echo 5Bil
~/vis_scan/8bits 8_1Billion 50 >> 8_Billion_Results
~/vis_scan/8bits 8_1Billion 50 >> 8_Billion_Results
~/vis_scan/8bits 8_1Billion 50 >> 8_Billion_Results
~/vis_scan/8bits 8_1Billion 50 >> 8_Billion_Results
~/vis_scan/8bits 8_1Billion 50 >> 8_Billion_Results

echo 9Bil
~/vis_scan/9bits 9_1Billion 100 >> 9_Billion_Results
~/vis_scan/9bits 9_1Billion 100 >> 9_Billion_Results
~/vis_scan/9bits 9_1Billion 100 >> 9_Billion_Results
~/vis_scan/9bits 9_1Billion 100 >> 9_Billion_Results
~/vis_scan/9bits 9_1Billion 100 >> 9_Billion_Results

echo 12Bil
~/vis_scan/12bits 12_1Billion 110 >> 12_Billion_Results
~/vis_scan/12bits 12_1Billion 110 >> 12_Billion_Results
~/vis_scan/12bits 12_1Billion 110 >> 12_Billion_Results
~/vis_scan/12bits 12_1Billion 110 >> 12_Billion_Results
~/vis_scan/12bits 12_1Billion 110 >> 12_Billion_Results

echo 16Bil
~/vis_scan/16bits 16_1Billion 200 >> 16_Billion_Results
~/vis_scan/16bits 16_1Billion 200 >> 16_Billion_Results
~/vis_scan/16bits 16_1Billion 200 >> 16_Billion_Results
~/vis_scan/16bits 16_1Billion 200 >> 16_Billion_Results
~/vis_scan/16bits 16_1Billion 200 >> 16_Billion_Results

echo 32Bil
~/vis_scan/32bits 32_1Billion 10000 >> 32_Billion_Results
~/vis_scan/32bits 32_1Billion 10000 >> 32_Billion_Results
~/vis_scan/32bits 32_1Billion 10000 >> 32_Billion_Results
~/vis_scan/32bits 32_1Billion 10000 >> 32_Billion_Results
~/vis_scan/32bits 32_1Billion 10000 >> 32_Billion_Results


