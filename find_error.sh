#! /usr/bin/sh

printf "" > random.input
printf "" > random.processed

while diff random.input random.processed; do
  head -c $1 /dev/urandom > random.input
  ./lzw -e random.input > random.compressed
  ./lzw -d random.compressed > random.processed
done
