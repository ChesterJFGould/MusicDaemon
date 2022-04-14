#include <stdio.h>
#include <string.h>
#include <ao/ao.h>
#include <sndfile.h>
#include <poll.h>

/*
 * Types
 */

struct song {
	void *internal;
	ao_device *device;
	char *path;
	int rate;
	int channels;
	int (*read_frames)(void *song, short *buffer, int num_frames);
	void (*free)(void *internal);
};

enum command_type {
	QUIT,
	PLAY,
	PAUSE,
	START
};

union command_data {
	struct song song;
};

struct command {
	enum command_type type;
	union command_data data;;
};

struct song_reader {
	char *extension; struct song* (*reader)(char *file_path, struct song *song); };

/*
 * Function Declarations
 */

struct command* try_parse_next_line(struct command* command);

struct command next_command(void);

struct command* try_next_command(struct command* command);

void free_song(struct song song);

void flow_chart(void);

struct song* read_sndfile(char *file_path, struct song *song);

int read_sndfile_frames(SNDFILE *sndfile, short *buffer, int num_frames);

void free_sndfile(SNDFILE *sndfile);

struct song* read_file(char *file_path, struct song *song);

int main(int argc, char **argv);

/*
 * Globals
 */

#include "config.h"

const int num_song_readers = sizeof(song_readers) / sizeof(song_readers[0]);

int driver_id;

/*
 * Function Definitions
 */

struct command*
try_parse_next_line(struct command* command)
{
	char input_buffer[buf_size];
	size_t input_length;
	if (fgets(input_buffer, buf_size, stdin) == NULL) {
		command->type = QUIT;
		return command;
	}
	
	input_length = strlen(input_buffer);
	
	// Remove newline
	input_buffer[input_length - 1] = 0;
	
	if (strcmp("play", input_buffer) == 0) {
		command->type = PLAY;
		return command;
	} else if (strcmp("pause", input_buffer) == 0) {
		command->type = PAUSE;
		return command;
	} else if (strncmp("start ", input_buffer, 6) == 0) {
		char *song_path;
		struct song tmp_song;

		song_path = &input_buffer[6];

		if (read_file(song_path, &tmp_song) == NULL) {
			fprintf(stderr, "Failed to open %s\n", song_path);
			return NULL;
		} else {
			command->type = START;
			command->data.song = tmp_song;
			command->data.song.path = malloc(buf_size * sizeof(char));
			strcpy(command->data.song.path, song_path);

			return command;
		}
	} else {
		return NULL;
	}
}

struct command
next_command()
{
	struct command command;

	while (try_parse_next_line(&command) == NULL) {}

	return command;
}

struct command*
try_next_command(struct command* command)
{
	struct pollfd pfds[1];

	pfds[0].fd = 0;
	pfds[0].events = POLLIN;

	if (poll(pfds, 1, 0) == -1) {
		fprintf(stderr, "Polling failed\n");
		return NULL;
	}

	if (pfds[0].revents & POLLIN) {
		return try_parse_next_line(command);
	} else {
		return NULL;
	}
}

void
free_song(struct song song)
{
	free(song.path);
	song.free(song.internal);
	ao_close(song.device);
}

void
flow_chart()
{
	struct song song;

	goto uninitialized;

	playing: {
		struct command command;
		size_t buffer_size = song.channels * (song.rate / response_time) * sizeof(short);
		char buffer[buffer_size];
		sf_count_t frames_read;

		if (try_next_command(&command) != NULL) {
			switch (command.type) {
			case QUIT:
				free_song(song);
				return;
			case PLAY:
				goto playing;
			case PAUSE:
				fprintf(stdout, "pausing\n");
				fflush(stdout);
				goto paused;
			case START:
				free_song(song);

				song = command.data.song;

				fprintf(stdout, "starting %s\n", command.data.song.path);
				fflush(stdout);

				goto playing;
			}
		}

		frames_read = song.read_frames(song.internal, (short*)buffer, song.rate / response_time);
		
		if (frames_read == 0) {
			free_song(song);

			fprintf(stdout, "done\n");
			fflush(stdout);

			goto uninitialized;
		}
		
		ao_play(song.device, buffer, frames_read * song.channels * sizeof(short));

		goto playing;
	}

	paused: {
		struct command command;

		command = next_command();

		switch (command.type) {
		case QUIT:
			free_song(song);
			return;
		case PLAY:
			fprintf(stdout, "playing\n");
			fflush(stdout);

			goto playing;
		case PAUSE:
			goto paused;
		case START:
			free_song(song);

			song = command.data.song;

			fprintf(stdout, "starting %s\n", command.data.song.path);
			fflush(stdout);

			goto playing;
		}
		/*
		 * NOTREACHED
		 */
	}

	uninitialized: {
		struct command command;

		command = next_command();

		switch (command.type) {
		case QUIT:
			return;
		case PLAY:
			goto uninitialized;
		case PAUSE:
			goto uninitialized;
		case START:
			fprintf(stdout, "starting %s\n", command.data.song.path);
			fflush(stdout);

			song = command.data.song;

			goto playing;
		}
		/*
		 * NOTREACHED
		 */
	}
	/*
	 * NOTREACHED
	 */
}

struct song*
read_sndfile(char *file_path, struct song *song)
{
	SF_INFO info;

	if ((song->internal = sf_open(file_path, SFM_READ, &info)) == NULL) {
		return NULL;
	}

	sf_command(song->internal, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
	
	song->rate = info.samplerate;
	song->channels = info.channels;

	song->read_frames = (int (*)(void *, short int *, int))read_sndfile_frames;
	song->free = (void (*)(void *))free_sndfile;

	return song;
}

int
read_sndfile_frames(SNDFILE *sndfile, short *buffer, int num_frames)
{
	return sf_readf_short(sndfile, buffer, num_frames);
}

void
free_sndfile(SNDFILE *sndfile)
{
	sf_close(sndfile);
}

struct song*
read_file(char *file_path, struct song *song)
{
	char *extension;

	if (!(extension = strrchr(file_path, '.'))) {
		return NULL;
	}

	extension++;

	for (int i = 0; i < num_song_readers; i++) {
		if (strcmp(extension, song_readers[i].extension) == 0) {
			if (song_readers[i].reader(file_path, song) != NULL) {
				ao_sample_format format;

				memset(&format, 0, sizeof(format));
				format.bits = 16;
				format.channels = song->channels;
				format.rate = song->rate;
				format.byte_format = AO_FMT_NATIVE;

				song->device = ao_open_live(driver_id, &format, NULL);
				if (song->device == NULL) {
					fprintf(stderr, "Error opening device.\n");
					return NULL;
				} else {
					return song;
				}
			}
		}
	}

	return NULL;
}

int
main(int argc, char **argv)
{
	ao_initialize();

	if ((driver_id = ao_driver_id(AO_DRIVER)) < 0) {
		fprintf(stderr, "No driver named %s exists\n", AO_DRIVER);
		return 1;
	}

	flow_chart();

	ao_shutdown();

  return (0);
}
