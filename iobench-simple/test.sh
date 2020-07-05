#/users/skannan/ssd/schedsp/simplebench/create_$1".sh"
#dd if=/dev/zero bs=4096 count=12582912 of=/mnt/$2/test
#time ./testread /mnt/$2/test 4096
rm -rf file1*

FILEPREFIX="file1"
IOSIZE=4096
REPEAT=1
THREADS=4
SEQACCESS=0
#FILESIZE= 

./testread $FILEPREFIX $IOSIZE $REPEAT $THREADS $SEQACCESS
rm -rf $FILEPREFIX*
