#! /bin/sh

MUSIC_DIR='/home/chester/Music'
MD_DIR='/home/chester/.local/share/md'
DISPLAY_RELATIVE_TO='\/home\/chester\/'

song=$(find $MUSIC_DIR | sort -V | sed "s/$DISPLAY_RELATIVE_TO//g" | dmenu -p 'Select Song: ' -l 30 -i | sed "s/^/$DISPLAY_RELATIVE_TO/g")

if [ -f "$song" ]
then
	echo "$song" >> $MD_DIR/queue
elif [ -d "$song" ]
then
	find "$song" -type f | sort -V >> $MD_DIR/queue
fi
