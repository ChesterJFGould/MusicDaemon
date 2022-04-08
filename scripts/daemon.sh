#! /bin/sh

BIN_DIR=/home/chester/Projects/MusicDaemon
MD_DIR=/home/chester/.local/share/md

rm -f $MD_DIR/control $MD_DIR/events $MD_DIR/errors
touch $MD_DIR/control $MD_DIR/events $MD_DIR/errors

trap 'trap " " TERM; pkill -P $$' INT TERM

tail -f $MD_DIR/control | $BIN_DIR/player 1>> $MD_DIR/events 2>> $MD_DIR/errors &

$BIN_DIR/scripts/queuer.sh &

wait
