struct song_reader song_readers[] = {
	{ "wav", read_sndfile },
	{ "ogg", read_sndfile },
};

const char *AO_DRIVER = "pulse";

const int response_time = 8;

const size_t buf_size = 4096;
