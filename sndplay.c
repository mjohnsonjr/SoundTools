#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "soundfunctions.h"


/* 
	Sndplay:

	NOTES:  The spec does not specify or require input from standardinput, so support is omitted here.  Standard output support
	still exists.  You may redirect stdout to a file with "> filename" on the unix command line.  

	Sndplay takes a .abc229 file in via the command line (only one at a time), and creates a sound file that plays
	the listed instruments in the file. Please ensure the abc229 is properly formatted.  If information is missing an error will be 
	thrown.  Make sure all data fields are listed. 

	Supported switches: --sr n (sample rate of the file to be created), 
						--bits n (bitdepth of the file to be created),
						-o filename (the filename the output will be directed to), 
						--mute n (mutes the specified instrument number),
						-h (display the help screen), -w (output to wav file instead of cs229).

						Note: Instruments are numbered from 0 - n.  Also, there are no error checks on --mute.  If you mute an instrument
						that does not exist, nothing happens.

	Supported Arguments: ONE .abc229 file.

	Note: If an instrument is muted, and it also plays the longest of each instrument.  The audio file will have a lot of silence at 
	the end of it.  If you unmute the instrument that is longest, it will play as normal.  This is because muted instruments are just zeroed 
	out, but the calculated file length is still the same.
*/


void help();

int main(int argc, char** argv)
{

/* flags and vars */
char helpflag = 0; /* Show the helpscreen */
char wavflag =0; /* Output to wav instead of cs229 */
char outflag = 0;	/*write to stdout or to this file if flag is up */
char inputfileflag = 0; /* Triggered when one abc229 file is detected.  This program only supports one abc229 input per run */

double seconds = 0; 

short bitdepth = 0;
int samplerate = 0;
int numnotes = 0;
int notespersecond = 0;
/*************************/
//float pf = 1;

FILE* fileout;	
	char *outfilename;
	char inputfilename[160];
	outfilename = malloc(180); 
	long i, k, j;
	char muted[argc]; /* Used to determine if a specific instrument has been muted. */

	/* Fill with zeros */
	for(i = 0; i < argc; i++) muted[i] = 0;

		char bitsflag = 0; char samplerateflag = 0;

	
	/* Decode Switches */
	for(i =1; i < argc; i++)
	{
		
		if(strcmp(argv[i], "-h") == 0 && helpflag == 0)  helpflag = 1; //set help flag.
		
		else if(strcmp(argv[i], "-w") == 0 && wavflag == 0) wavflag = 1; //wav flag
		
		else if(strcmp(argv[i], "-o") == 0 && outflag == 0)
		{
			 outflag = 1; 
			 outfilename = argv[i+1];
			 i++;
		}

		else if(strcmp(argv[i], "--bits") == 0 && bitsflag == 0)
		{
			errno = 0;
			 bitdepth = strtol(argv[i+1], NULL, 10);
			 bitsflag = 1;
			 if(errno != 0 || (bitdepth != 8 && bitdepth != 16 && bitdepth != 32))
			 {
			 	fprintf(stderr, "\n%s: Invalid value for bit depth (8, 16, or 32 only).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "--sr") == 0 && samplerateflag == 0)
		{
			 errno = 0;
			 samplerate = strtol(argv[i+1], NULL, 10);
			 samplerateflag = 1;
			 if(errno != 0 || samplerate > 44100 || samplerate < 1)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for samplerate. No larger than 44100 is supported, no lower than 1.  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "--mute") == 0)
		{
			 errno = 0;
			 muted[strtol(argv[i +1], NULL, 10)] = 1;
			 
			 if(errno != 0)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for mute.  Aborting.\n\n", argv[i+1]);
			 	exit(1);
			 } 
			 i++;

		}

		else if(argv[i][0] != '-' && inputfileflag == 0)
		{
			strcpy(inputfilename, argv[i]);
			inputfileflag = 1;
		}

		/* Any other unrecogznied switches. Don't use here*/
		else 
		{	
			fprintf(stderr, "\n%s: Unrecogznied/duplicate switch.  Please see help screen with -h.\n\n", argv[i]);
			exit(1);
		}
	}

	if(helpflag == 1) /*Print out the help screen and close.*/
	{
		help();
	}

	if(bitsflag == 0 || samplerateflag == 0)
	{
		fprintf(stderr, "\nERROR! Samplerate and bitdepth switches not detected.  Please specify each switch.  Aborting.\n\n");
		exit(1);
	}

	if(outflag == 0)
	{
		outfilename = "Stdout";
	}

	/*Switches done, now handle both cases */ 


	if(inputfileflag == 0)
	{
		fprintf(stderr, "\nERROR! Please specify an input .abc229 file as an argument.  This program doesn't read stdin.\n\n");
		exit(1);
	}

	FILE* file;

	if((file = fopen(inputfilename, "r")) == NULL)
	{
		fprintf(stderr, "\nERROR! Couldn't open the .abc229 file specified.\n\n");
		exit(1);
	}

	abc229 abc;


	char err = fillABC229Info(&abc, file);
	char *extension2 = strrchr(inputfilename, '.');
	if(extension2 != NULL && strcasecmp(extension2 + 1, "abc229") == 0) {}
	else err = -1;

	if(err < 0)
	{
		fprintf(stderr, "\nERROR!  Abc229 file specified is invalid or is missing info about an instrument.\nOr, this isn't even a proper .abc229 file.  Please ensure it has the proper .abc229 extension.\n\n");
		exit(1);
	}
	
	/* Need to create a function for each wave.  And generate a struct. */
	
	fileinfo file_info[abc.numinstruments + 1];

	file_info[0].filename = outfilename;
	char *extension = strrchr(outfilename, '.');
	if(extension == NULL) file_info[0].extension = "";
	else file_info[0].extension = extension + 1;


	/* Find highest note count. */
	for(i = 0; i < abc.numinstruments; i++)
	{
		if(abc.instruments[i].numnotes > numnotes) numnotes = abc.instruments[i].numnotes;
	}

	/* Determine sound file length */
	notespersecond = abc.tempo/60;

	/* From notes per second, times notes = seconds.*/
	seconds = (double)numnotes/notespersecond;

	for(i = 0; i < abc.numinstruments + 1; i++) file_info[i].numsamples = samplerate * seconds;
	for(i = 0; i < abc.numinstruments + 1; i++) file_info[i].numchannels = 1;
	for(i = 0; i < abc.numinstruments + 1; i++) file_info[i].samplerate = samplerate;
	for(i = 0; i < abc.numinstruments + 1; i++) file_info[i].seconds = seconds;
	for(i = 0; i < abc.numinstruments + 1; i++) file_info[i].bitdepth = bitdepth;

		
	for(i = 0; i < abc.numinstruments + 1; i++)
	{
		if(NULL == (file_info[i].samples = malloc(file_info[i].numsamples * file_info[i].numchannels * sizeof(int) + 10)))
		{ 
			fprintf(stderr, "MALLOC FAILED! In sndplay.\n"); 
			exit(1);
		}
		//fprintf(stderr, "%d\n", file_info[i].numsamples * file_info[i].numchannels * sizeof(int) + 10);
	}

	/* Start with zeroed out samples */
	for(i = 0; i < file_info[0].numsamples * file_info[0].numchannels; i++) file_info[0].samples[i] = -1;

int lastattacksample = 0; int lastdecaysample = 0; int lastsustainsample = 0; int lastreleasesample =0; float peakattackvolume;
float attackslope; float decayslope; float releaseslope; float samplespertime; float timer, oldtimer = 0.0; float attackincrements;
float decayincrements; float releaseincrements; int holder = 0, holder2 = 0; float sustaintime; char whitenoiseflag = 0;



i = 0;
for(k = 1; k < abc.numinstruments + 1; k++)
{
	whitenoiseflag = 0;
		/* Check if any trimming is required */
		sustaintime = seconds/numnotes - abc.instruments[k -1].attack - abc.instruments[k -1].decay - abc.instruments[k -1].release;

		if(sustaintime < 0)
		{
			sustaintime = 0;
			abc.instruments[k -1].decay = seconds - abc.instruments[k -1].attack - abc.instruments[k -1].release;
			
			if(abc.instruments[k -1].decay < 0)
			{
				abc.instruments[k -1].decay = 0;
				abc.instruments[k -1].attack = seconds - abc.instruments[k -1].release;
				if(abc.instruments[k -1].attack < 0)
				{
					if(abc.instruments[k -1].release > seconds)
					{
						whitenoiseflag = 1;
					}
				}
			}
		}

		 peakattackvolume = abc.instruments[k -1].volume * (pow(2, bitdepth -1) -1);
		/* Sustain volume is min decay volume */	

		 attackslope = ((float)(abc.instruments[k -1].volume - 0)/(abc.instruments[k -1].attack - 0));
		 decayslope = ((float)(abc.instruments[k -1].sustain * abc.instruments[k -1].volume - abc.instruments[k -1].volume)/((abc.instruments[k -1].decay + abc.instruments[k -1].attack) - abc.instruments[k -1].attack));
		 releaseslope = ((float)(abc.instruments[k -1].sustain * abc.instruments[k -1].volume - 0)/((seconds/numnotes - abc.instruments[k -1].release) - seconds/numnotes));	

		 samplespertime = ((float)seconds)/file_info[k -1].numsamples;
		 //timer = 0.0;

		 attackincrements = attackslope * samplespertime;
		 decayincrements = decayslope * samplespertime;
		 releaseincrements = releaseslope * samplespertime;
		 
		 holder2 = 0;
		 i=0; timer = 0.0;
		 lastattacksample = lastdecaysample = lastsustainsample = lastreleasesample = 0;


	for(j = 0; j < abc.instruments[k -1].numnotes; j++)
	{
		 lastattacksample = lastattacksample + (samplerate * abc.instruments[k-1].attack)/numnotes;
		 lastdecaysample = lastdecaysample + (samplerate * (abc.instruments[k-1].attack + abc.instruments[k -1].decay))/numnotes;
		 lastsustainsample = lastsustainsample + (samplerate * (abc.instruments[k -1].attack + abc.instruments[k -1].decay + sustaintime))/numnotes;
		 lastreleasesample = lastreleasesample + (samplerate * (abc.instruments[k -1].attack + abc.instruments[k -1].decay + sustaintime + abc.instruments[k -1].release))/numnotes;

		/* Sine wave */ 
		if(strcasecmp(abc.instruments[k -1].waveform, "Sine") == 0 && whitenoiseflag == 0)
		{	
			//fprintf(stderr, "%ld, %ld, %d, %d, %d, %d, %ld, %d\n",k , i, lastattacksample, lastdecaysample, lastsustainsample, lastreleasesample, j, abc.instruments[k -1].notes[j]);
			/* Attack stage (sin) */
			for(i = holder2; i < lastattacksample; i++)
			{	
				file_info[k].samples[i] = (float) (i * attackincrements) * sin(6.28 * abc.instruments[k -1].notes[j] * timer) * (pow(2, bitdepth -1) -1);
					timer += samplespertime;
			}
			
			holder = i;

			/* Decay Stage */
			for(i; i < lastdecaysample; i++)
			{
				file_info[k].samples[i] = (float) (abc.instruments[k -1].volume + (i - holder) * decayincrements) * sin(6.28 * abc.instruments[k -1].notes[j] * timer) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder = i;

			/* Decay Stage */
			for(i; i < lastsustainsample; i++)
			{
				file_info[k].samples[i] = (float) (abc.instruments[k -1].sustain * abc.instruments[k -1].volume) * sin(6.28 * abc.instruments[k -1].notes[j] * timer) * (pow(2, bitdepth) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Decay Stage */
			for(i; i < lastreleasesample; i++)
			{
				file_info[k].samples[i] = (float)(abc.instruments[k -1].volume * abc.instruments[k].sustain + (i - holder) * releaseincrements) * sin(6.28 * abc.instruments[k -1].notes[j] * timer) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder2 = i;
			oldtimer = timer;
		}

		/* Pulse wave */ 
		else if(strcasecmp(abc.instruments[k -1].waveform, "Pulse") == 0 && whitenoiseflag == 0)
		{	
			//fprintf(stderr, "%f\n", abc.instruments[k-1].pulsefrac);
			/* Attack stage */
			for(i = holder2; i < lastattacksample; i++)
			{	
				file_info[k].samples[i] = (float) ((i * attackincrements)*(2/3.14) * sin(abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(6.28 * abc.instruments[k-1].notes[j] * timer) + (i * attackincrements)*(1/3.14) * sin(2 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k-1].notes[j]) * cos(2 * 6.28 * abc.instruments[k-1].notes[j] * timer) + (i * attackincrements)*(2/(3*3.14)) * sin(3 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k-1].notes[j]) * cos(3 * 6.28 * abc.instruments[k-1].notes[j] * timer) + (i * attackincrements)*(1/(2 * 3.14)) * sin(4* abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k-1].notes[j]) * cos(4 * 6.28 * abc.instruments[k-1].notes[j] * timer)) * (pow(2, bitdepth -1) -1); 	
				timer += samplespertime;
			}
			
			holder = i;

			/* decay stage */
			for(i; i < lastdecaysample; i++)
			{
				file_info[k].samples[i] = (float) ((abc.instruments[k -1].volume + (i - holder) * decayincrements)*(2/3.14) * sin(abc.instruments[k-1].pulsefrac* 3.14 * abc.instruments[k -1].notes[j]) * cos(6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].volume + (i - holder) * decayincrements)*(1/3.14) * sin(2 * abc.instruments[k-1].pulsefrac* 3.14 * abc.instruments[k-1].notes[j]) * cos(2 * 6.28 * abc.instruments[k-1].notes[j] * timer) + (abc.instruments[k-1].volume + (i - holder) * decayincrements)*(2/(3*3.14)) * sin(3 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k-1].notes[j]) * cos(3 * 6.28 * abc.instruments[k-1].notes[j] * timer) + (abc.instruments[k-1].volume + (i - holder) * decayincrements)*(1/(2 * 3.14)) * sin(4* abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k-1].notes[j]) * cos(4 * 6.28 * abc.instruments[k-1].notes[j] * timer)) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder = i;

			/* Sustain stage */
			for(i; i < lastsustainsample; i++)
			{
				file_info[k].samples[i] = (float) ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)*(2/3.14) * sin(abc.instruments[k-1].pulsefrac* 3.14 * abc.instruments[k -1].notes[j]) * cos(6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume)*(1/3.14) * sin(2 * abc.instruments[k-1].pulsefrac* 3.14 * abc.instruments[k -1].notes[j]) * cos(2 * 6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume)*(2/(3*3.14)) * sin(3 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(3 * 6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume)*(1/(2 * 3.14)) * sin(4* abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(4 * 6.28 * abc.instruments[k -1].notes[j] * timer)) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Release stage */
			for(i; i < lastreleasesample; i++)
			{
				file_info[k].samples[i] = (float) ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)*(2/3.14) * sin(abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)*(1/3.14) * sin(2 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(2 * 6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)*(2/(3*3.14)) * sin(3 * abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(3 * 6.28 * abc.instruments[k -1].notes[j] * timer) + (abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)*(1/(2 * 3.14)) * sin(4* abc.instruments[k-1].pulsefrac * 3.14 * abc.instruments[k -1].notes[j]) * cos(4 * 6.28 * abc.instruments[k -1].notes[j] * timer)) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder2 = i;
			oldtimer = timer;
		}

		/* Triangle wave */ 
		else if(strcasecmp(abc.instruments[k-1].waveform, "Triangle") == 0 && whitenoiseflag == 0)
		{	
			/* Attack stage */
			for(i = holder2; i < lastattacksample; i++)
			{	
				file_info[k].samples[i] = (float) (8/(3.14*3.14))*(((i * attackincrements) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((i * attackincrements)/9) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + ((i * attackincrements)/25) * sin(5 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((i * attackincrements)/49) * sin(7 * (6.28 * abc.instruments[k-1].notes[j] * timer)))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Decay stage */
			for(i; i < lastdecaysample; i++)
			{
				file_info[k].samples[i] = (float) (8/(3.14*3.14))*(((abc.instruments[k -1].volume + (i - holder) * decayincrements)) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].volume + (i - holder) * decayincrements)/9) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + ((abc.instruments[k -1].volume + (i - holder) * decayincrements)/25) * sin(5 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].volume + (i - holder) * decayincrements)/49) * sin(7 * (6.28 * abc.instruments[k-1].notes[j] * timer))) * (pow(2, bitdepth-1) -1);
				timer += samplespertime;
			}
			holder = i;

			/* Sustain stage */
			for(i; i < lastsustainsample; i++)
			{
				file_info[k].samples[i] = (float) (8/(3.14*3.14))*(((abc.instruments[k -1].sustain * abc.instruments[k -1].volume) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)/9) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)/25) * sin(5 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)/49) * sin(7 * (6.28 * abc.instruments[k-1].notes[j] * timer)))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Release stage */
			for(i; i < lastreleasesample; i++)
			{
				file_info[k].samples[i] = (float) (8/(3.14*3.14))*(((abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)  * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)/9) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)/25) * sin(5 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume + (i - holder) * releaseincrements)/49) * sin(7 * (6.28 * abc.instruments[k-1].notes[j] * timer)))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder2 = i;
			oldtimer = timer;
		}

		/* Sawtooth wave */ 
		else if(strcasecmp(abc.instruments[k-1].waveform, "Sawtooth") == 0 && whitenoiseflag == 0)
		{	
			/* Attack stage */
			for(i = holder2; i < lastattacksample; i++)
			{	
				file_info[k].samples[i] = (float) (2/3.14)*((i * attackincrements) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((i * attackincrements)) * sin(2 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + (1/3)*((i * attackincrements)) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((i * attackincrements)/4) * sin(4 * (6.28 * abc.instruments[k-1].notes[j] * timer))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Decay stage */
			for(i; i < lastdecaysample; i++)
			{
				file_info[k].samples[i] = (float) (2/3.14)*((abc.instruments[k -1].volume + (i - holder) * decayincrements) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].volume + (i - holder) * decayincrements) * sin(2 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + (1/3)*((abc.instruments[k -1].volume + (i - holder) * decayincrements)) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].volume + (i - holder) * decayincrements)/4) * sin(4 * (6.28 * abc.instruments[k-1].notes[j] * timer)))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder = i;

			/* Sustain stage */
			for(i; i < lastsustainsample; i++)
			{
				file_info[k].samples[i] = (float) (2/3.14)*((abc.instruments[k -1].sustain * abc.instruments[k -1].volume) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume) * sin(2 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + (1/3)*((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].sustain * abc.instruments[k -1].volume)/4) * sin(4 * (6.28 * abc.instruments[k-1].notes[j] * timer)))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			
			holder = i;

			/* Release stage */
			for(i; i < lastreleasesample; i++)
			{
				file_info[k].samples[i] = (float) (2/3.14)*((abc.instruments[k -1].volume * abc.instruments[k -1].sustain + (i - holder) * releaseincrements) * sin(6.28 * abc.instruments[k-1].notes[j] * timer) - ((abc.instruments[k -1].volume * abc.instruments[k -1].sustain + (i - holder) * releaseincrements)) * sin(2 * (6.28 * abc.instruments[k-1].notes[j] * timer)) + (1/3)*((abc.instruments[k -1].volume * abc.instruments[k -1].sustain + (i - holder) * releaseincrements)) * sin(3 * (6.28 * abc.instruments[k-1].notes[j] * timer)) - ((abc.instruments[k -1].volume * abc.instruments[k -1].sustain + (i - holder) * releaseincrements)/4) * sin(4 * (6.28 * abc.instruments[k-1].notes[j] * timer))) * (pow(2, bitdepth -1) -1);
				timer += samplespertime;
			}
			holder2 = i;
			oldtimer = timer;
		}


		else if(whitenoiseflag == 1){} /* Do nothing, no sound */

		else
		{
			fprintf(stderr, "\nERROR! Unable to create a wave of type: %s. Aborting.\nPlease edit the .abc229 file and remove any errors.\n\n", abc.instruments[k-1].waveform);
			exit(1);
		}


	}

}

/* Add them all up and check for muted instruments. */
for(i = 1; i < abc.numinstruments + 1; i++)
{
	for(j = 0; j < file_info[i].numsamples && muted[i -1] != 1; j++)
	{
		file_info[0].samples[j] += file_info[i].samples[j];
		//printf("%d, %d, %d\n", i, j, file_info[i].samples[j]);
	}
	k = k + j;
}

	
	/* Finished generating, now rebuild the output file*/
	if(wavflag == 1) /*output to wav format.*/
	{
		fileout = createWav(&file_info[0], outfilename);	

		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info[0].filename);
			exit(1);	
		}

		/* Flush to stdout, destroy the file we created. */
		if(outflag == 0)
		{
			fclose(fileout);
			FILE* tempfile = fopen(outfilename, "r");
			if(fileout == NULL)
			{
				fprintf(stderr, "\n%s On read-back, .cs229 file failed to open.\n\n", file_info[0].filename);
				exit(1);	
			}
			fseek(fileout, 1, SEEK_END);
			long size = ftell(fileout);
			rewind(fileout);

			BYTE buffer[size];

			fread(&buffer, size, 1, tempfile);
			fwrite(&buffer, size, 1, stdout);			
			remove(outfilename);
			fclose(tempfile);	
			/* With this, the user can use > on the command line to flush output from stdout to their own filename */		
		}
	}

	else /* output to .cs229 format */
	{	
		fileout = createCS229(&file_info[0], outfilename);
		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info[0].filename);
			exit(1);	
		}

		/* Flush to stdout, and destroy the file we created. */
		if(outflag == 0)
		{
			fclose(fileout);
			FILE* tempfile = fopen(outfilename, "r");
			if(fileout == NULL)
			{
				fprintf(stderr, "\n%s On read-back, .cs229 file failed to open.\n\n", file_info[0].filename);
				exit(1);	
			}
			fseek(fileout, 1, SEEK_END);
			long size = ftell(fileout);
			rewind(fileout);

			BYTE buffer[size];

			fread(&buffer, size, 1, tempfile);
			fwrite(&buffer, size, 1, stdout);			
			remove(outfilename);
			fclose(tempfile);	
			/* With this, the user can use > on the command line to flush output from stdout to their own filename */
		}
	}

	fprintf(stderr, "\nSuccess! %s was written with the specified properties.\n\n", file_info[0].filename);

	return 0;
}



void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n");
	printf("\tSndplay\n");
	printf("*****************************\n");
	printf("\tSupported Switches:");
	printf("-h (help), -w (output wav file instead of cs229), \n\t-o filename (output to specified filename, otherwise stdout)");
	printf("--mute n (mutes the instrument number n), --bits n (specify the bitdepth), \n\t--sr n (sets the samplerate to n)");
	printf("\n\tSndplay takes a .abc229 file in as a command line argument (ONLY ONE) and creates a sound file conforming to those specs.\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n");
	printf("\tThis program will exit after writing the new sound file conforming to the user specified .abc229 file.\n");
	printf("\tThis program supports writing to stdout (by not using the -o switch) but does not read from stdin.\n");
	printf("\nPlease view the README file for more detailed information.\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}


