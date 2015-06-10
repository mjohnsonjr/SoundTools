#ifndef SOUNDFUNCTIONS
#define SOUNDFUNCTIONS


typedef unsigned char BYTE;

/* All data for any sound file format */ 
typedef struct 
{
	char *filename;
	char *extension;
	int samplerate;
	char bitdepth; /*these will fit in 1 byte..*/
	char numchannels;
	long numsamples;
	double seconds;
	signed int* samples; /* Depends on bit depth (8, 16, 32) */
	
} fileinfo;

/* Contains specific data for an instrument contained in a .abc229 file. */
typedef struct 
{
	char waveform[15];
	float volume;
	float attack;
	float decay;
	float sustain; 
	float release;
	float pulsefrac;
	int numnotes;
	signed int* notes;
	
} instrument;


/* Specific information about an abc229 file. */
typedef struct 
{
	int numinstruments;
	int tempo;
	instrument* instruments;
	
} abc229;

/* Prototypes */ 
void printfileinfo(fileinfo* file_info);

int fillWavInfo(fileinfo* fileinfo, FILE* file);

int fillCS229Info(fileinfo* file_info, FILE* file);

FILE* createWav(fileinfo* file_info, char* path);

FILE* createCS229(fileinfo* file_info, 	char* path);

int fillABC229Info(abc229* abc, FILE* file);

#endif