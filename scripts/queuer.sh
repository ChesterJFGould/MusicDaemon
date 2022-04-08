#! /bin/sh

MD_DIR=/home/chester/.local/share/md

rm -f $MD_DIR/queue
touch $MD_DIR/queue

play_next()
{
	echo "start $(head -n 1 $MD_DIR/queue)" >> $MD_DIR/control
	sed -i -e '1d' $MD_DIR/queue
}

while [ true ]
do
	if [ -s $MD_DIR/queue ]
	then
		case "$(grep -e ^starting -e ^done $MD_DIR/events | tail -n 1)" in
			'done' | '')
				play_next
				;;
			*)
				stdbuf -o0 tail -n 0 -f $MD_DIR/events | stdbuf -o0 grep '^done' | head -n 0
				;;
		esac
	else
		tail -f $MD_DIR/queue | head -n 1 > /dev/null
	fi
done
