#!/bin/bash

echo "PUTS" >> results/10clients.txt
for i in 100 1000 10000 100000
do
   echo "$i" >> results/10clients.txt
   for j in {0..5}
   do
      ./bin/multiclient.exe p 10 $i >> results/10clients.txt
   done
   echo "" >> results/10clients.txt
done

echo "GETS" >> results/10clients.txt
for i in 100 1000 10000 100000
do
   echo "$i" >> results/10clients.txt
   for j in {0..5}
   do
      ./bin/multiclient.exe g 10 $i >> results/10clients.txt
   done
   echo "" >> results/10clients.txt
done

echo "PUTS" >> results/100clients.txt
for i in 10 100 1000 10000
do
   echo "$i" >> results/100clients.txt
   for j in {0..5}
   do
      ./bin/multiclient.exe p 100 $i >> results/100clients.txt
   done
   echo "" >> results/100clients.txt
done

echo "GETS" >> results/100clients.txt
for i in 10 100 1000 10000 100000
do
   echo "$i" >> results/100clients.txt
   for j in {0..5}
   do
      ./bin/multiclient.exe g 100 $i >> results/100clients.txt
   done
   echo "" >> results/100clients.txt
done