USERN=$(whoami)
DEVICE=/dev/pmem0
MOUNTPOINT=/mnt/pmemblock
echo $USERN

if [ $(mount | grep -c $MOUNTPOINT) != 1 ]
then
    sudo umount /mnt/pmemdir
    sudo umount $MOUNTPOINT
    sudo mkdir $MOUNTPOINT
    sudo mkfs -t ext4 -b 4096 $DEVICE
    sudo mount $DEVICE $MOUNTPOINT
    echo "$MOUNTPOINT is now mounted"
else
    echo "$MOUNTPOINT already mounted"
fi

sudo chown -R $USER $MOUNTPOINT                                                                                                                                       
