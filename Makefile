player: player.c config.h
	cc -Wall -Werror -std=c99 -o player -g -O3 -lao -lsndfile player.c
