#!/bin/tcsh -f

if ( $#argv < 2 ) then
	echo "Usage: atb_extrac streams_to_extract stream_list" 
	echo "output is atb_extract.txt" 
	exit 1
endif

echo $1
echo $2
set lines=`cat $1`
set i=1
set STREAM = ""
rm -rf "atb_extract.txt"
while ( $i <= $#lines )
   #echo $lines[$i]
   #echo $i
   set STREAM = `more $2 |grep $lines[$i]`
   echo $STREAM 
   echo $STREAM >>  "atb_extract.txt"
   @ i = $i + 1
end

