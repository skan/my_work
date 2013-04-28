#! /bin/bash

AWK='
function Fix( t, Local, f, k, V) 
{
    while (match (t, /"[^"]*"/)) 
    {
        k = sprintf ("\034%.2d", ++f);
        V [k] = substr (t, RSTART, RLENGTH);
        t = substr (t, 1, RSTART - 1) k substr (t, RSTART + RLENGTH);
    }
    for (k in V) gsub (/,/, "\\,", V [k]);
    for (k in V) sub (k, V [k], t);
    return (t);
}
{ print Fix( $0); }
'
    awk "${AWK}" < small.csv
