#!/bin/bash
#  File: fig_test.sh
#  Assemble and run a Pep/10 assembly language program
#  Take input from figxxxx_input.txt
#  Sends output to figxxxx_output.txt
#  Usage: ./fig_test.sh <source without .pep, which is assumed>

if [ -f $1\.pep ]
then
   echo "Compiling "$1".pep"
   /Users/warford/Developer/build-BUILD-ALL-Debug/pep10term/Pep10Term.app/Contents/MacOS/pep10term asm -s $1\.pep -o $1\.pepo
   if [ -f $1\_errLog.txt ]
   then
      echo
      echo "Assembly error. Error log:"
      echo "=========================="
      cat $1\_errLog.txt
      echo "=========================="
      rm $1\_errLog.txt
      echo
   else
      if [ -f $1\.pepo ]
      then
         echo "Running "$1".pepo"
         echo
         /Users/warford/Developer/build-BUILD-ALL-Debug/pep10term/Pep10Term.app/Contents/MacOS/pep10term run -s $1\.pepo -i $1\_input.txt -o $1\_output.txt
         echo
         rm $1\.pepo  
      fi
   fi
else
   echo "No file "$1".pep"
fi
