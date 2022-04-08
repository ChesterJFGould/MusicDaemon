#! /bin/sh

MD_DIR=/home/chester/.local/share/md


case "$(grep -e ^playing -e ^pausing -e ^starting $MD_DIR/events | tail -n 1)" in
pausing)
	echo play >> $MD_DIR/control
	;;
*)
	echo pause >> $MD_DIR/control
	;;
esac
