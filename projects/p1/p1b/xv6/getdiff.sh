
for x in `ls -1 */*.[chS]` 
do
    echo  $x
    diff /u/h/a/haiyun/private/cs537/projects/p2/p2b/xv6/newxv6/$x $x
    echo 
done
