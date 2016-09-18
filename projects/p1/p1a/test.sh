
numb=500000
maxn=10000

#./genvar -s 0 -n $numb -o /tmp/data.dat -m $maxn
./genvar -s 0 -n $numb -o data.dat -m $maxn
#time ./varsort -i /tmp/data.dat -o /tmp/data.dat.out
time ./varsort -i data.dat -o data.dat.out


#ll /tmp/data.dat
#rm -f /tmp/data.dat /tmp/data.dat.out
ll data.dat
rm -f data.dat data.dat.out
