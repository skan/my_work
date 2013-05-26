#!/bin/bash
# Shell script to reformat csv file to have only one column for both credit and debit amounts 
# 
# input: formatted CSV file from amennet xls. format is: 
#account_num; Date_operation;Description;number_piece;Date_value;debit;credit
# output QIF file for winancial
# 

if [ $# -lt 2 ] ; then
    echo Usage: $0 input_amen.csv output_winancial.qif
    exit 0
fi

DEFAULT_CSV=$1
FORMATTED_QIF=$2

echo Converting $DEFAULT_CSV, writing to $FORMATTED_QIF...

awk -F'"' '{gsub(/,/,"|",$1);gsub(/,/,"|",$3);} 1' $DEFAULT_CSV > temp_formatted.csv

DEFAULT_CSV=temp_formatted.csv
export IFS="|"

cat $DEFAULT_CSV | while read account_num date_op description num date_val debit credit 
do 
   echo "D$date_op" >> $FORMATTED_QIF
   echo "M$description" >> $FORMATTED_QIF
   echo "N$num" >> $FORMATTED_QIF
   if [ $debit ] ; then 
      echo "T-$debit" | sed -e "s/ //g" >> $FORMATTED_QIF
   else
      echo "T$credit" | sed -e "s/ //g" >> $FORMATTED_QIF
   fi 
   echo "^" >> $FORMATTED_QIF
done

rm temp_formatted.csv

echo Done!
exit 1
