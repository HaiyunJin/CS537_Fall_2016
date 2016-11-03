
for x in `ps | grep stats_client | awk '{print $1}'`
do
    kill $x;
done
