#! /bin/sh

MD_DIR=/home/chester/.local/share/md

if [ -s $MD_DIR/queue ]
then
	echo "start $(head -n 1 $MD_DIR/queue)" >> $MD_DIR/control
	sed -i -e '1d' $MD_DIR/queue
fi
