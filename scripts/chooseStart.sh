#! /bin/sh

MUSIC_DIR='/home/chester/Music'
MD_DIR='/home/chester/.local/share/md'
DISPLAY_RELATIVE_TO='\/home\/chester\/'

song=$(find $MUSIC_DIR | sort -V | sed "s/$DISPLAY_RELATIVE_TO//g" | dmenu -p 'Select Song: ' -l 30 -i | sed "s/^/$DISPLAY_RELATIVE_TO/g")

if [ -f "$song" ]
then
	echo "start $song" >> $MD_DIR/control
elif [ -d "$song" ]
then
	songs="$(find $song -type f | sort -V)"

	echo "$songs" | head -n 1 | sed 's/^/start /' >> $MD_DIR/control

	echo "$songs" | tail -n +2 > $MD_DIR/queue
fi
