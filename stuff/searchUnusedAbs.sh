#!/bin/bash

MAINDIR=..
OUTFILE=unused-abstractions-`date +%Y-%m-%d`.txt
rm -f $OUTFILE
TESTDIR=$MAINDIR/0*.pd

blacklist=(0FeuCamp)

function containsElement () {
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

function isAbsUsed()
{
	local absbasename=`basename -s .pd $1`
	local parent
	echo searching $absbasename
	containsElement $absbasename "${blacklist[@]}"
	if [ $? == 0 ] ; then return 1 ; fi
	for parent in `grep -lrs -E "^#X obj [0-9]+ [0-9]+( clone( [0-9]+)?( -s [0-9])?)? (([^/ ]+/)+)?$absbasename *" $MAINDIR/*.pd $MAINDIR/* $MAINDIR/*/*` ; do
		echo "in $parent: "
		isAbsUsed $parent
		if [ $? == 1 ] ; then echo "true" ; return 1 ; fi
		done
	return 0
}

(
for abs in `find $MAINDIR -name "*.pd" `; do
	echo ------ processing $abs -------
	isAbsUsed $abs
	if [ $? == 0 ] ; then echo ${abs:${#MAINDIR}+1:1000} >> $OUTFILE; fi
	echo
	done
)

