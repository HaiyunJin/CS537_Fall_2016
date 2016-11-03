
token=tmpdsLR_n

nums=`grep "$token/ " runtests.log   | awk '{print $2}' | sort -k1 -u`

for x in $nums ; do grep "$token/ " runtests.log   | sort -k 2 -n | grep $x | sort -k 1 -n ; echo ; done
