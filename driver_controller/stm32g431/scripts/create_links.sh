#!/bin/bash

threadx_main_dir=$3
src_tree=($4)
project_dir=$2

if [ $1 == add ]; then
  for path in ${src_tree[@]}
  do      
    ln -sf $threadx_main_dir/${path}/* $project_dir/${path}/    
  done
fi

if [ $1 == del ]; then
  for path in ${src_tree[@]}
  do 
    rm -f $project_dir/${path}/*.c
    rm -f $project_dir/${path}/*.h
    rm -f $project_dir/${path}/*.S
  done
fi

if [ $1 == create ]; then
  for path in ${src_tree[@]}
  do 
    mkdir -p $project_dir/${path}/
  done
fi