#!/bin/bash
# Shell script to take a CSV file and convert to the
# necessary QIF file for input into GNUCASH
# 
# Assumes CSV in the format of: 
#     Date,Transaction Type,Check Number,Description,Amount
# 
# Formatting codes:
# D - Date (example: D01/11/2012)
# T - Amount of item (example: T-21.99)
# C - Cleared status (example: C*)
# N - Number of check (example: N101)
# P - Payee (example: PDescription of debit)
# L - Category/transfer class (example: LExpense:Auto)
#
#      echo "$date_op;$description;$num;$date_val;-$debit"
# TODO: Add automatic categorization of common items (grocery, etc)

if [ $# -lt 3 ] ; then
    echo Usage: $0 ACCOUNT.QIF CSV_INPUT QIF_OUTPUT
    exit 0
fi

ACCOUNT=$1
CSV=$2
QIF=$3

echo Converting $CSV, prepending $ACCOUNT, writing to $QIF...

cat $ACCOUNT > $QIF

echo '!Type:Bank' >> $QIF

sed -e 's/(\$/-/g' -e 's/)//g' -e 's/\$//g' $CSV \
    | awk -F, 'NR>1{ print "D" $1; \
            print "T" $4; \
            print "M" $2; \
            print "N" $3; \
            print "^"; }' >> $QIF

echo Done!
exit 1
