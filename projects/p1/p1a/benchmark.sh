

echo -n '' >| aa

for x in `seq 1 10` 
do
    ./runtests  smalldatacontest
    grep "Varsort completed" runtests.log >> aa
done
