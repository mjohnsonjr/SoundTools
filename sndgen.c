#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "soundfunctions.h"

/*
	This program takes in a wide variety of switches as command line arguments and uses them to generate a simple ADSR sound
	wave based on the switches you choose (see switches below).  This program does not read from standard input. If no -o switch exists,
	 we write to standard output.  All user messages are written to standard error.  You can pipe standard output data to 
	a file using "> filename" on the command line.  

	Switches: -h (display helpscreen), -o filename (output to this file), -w (output to wav instead of .cs229)
		--bits n : use a bit depth of n
		--sr n : use a sample rate of n
		-f r : use a frequency of r Hz
		-t r : total duration of r seconds
		-v p : Peak volume. 0 <= p <= 1
		-a r : attack time of r seconds
		-d r : decay time of r seconds
		-s p : sustain volume. 0 <= p <= 1
		-r r : release time of r seconds
		--sine : generate a sine wave
		--triangle : generate a triangle wave
		--sawtooth : generate a sawtooth wave
		--pulse : generate a pulse wave
		--pf p : Fraction of the time the pulse wave is "up"

	Args: None. This program only takes in switches listed above.

	More detailed information and examples are listed in the README file.

*/

void help();

int main(int argc, char** argv)
{

/* flags and vars */
char helpflag = 0; /* Show the helpscreen */
char wavflag =0; /* Output to wav instead of cs229 */
char outflag = 0;	/*write to stdout or to this file if flag is up */


/* Snd gen flags */
char sineflag = 0;
char triangleflag = 0;
char sawtoothflag = 0;
char pulseflag = 0;

/* Snd gen Data. */
float frequency = 0;
double seconds = 0; 
float peakvolume = 0;

float attacktime = 0;
float decaytime = 0;
float sustainvolume = 0;
float releasetime = 0;
float pf = 0;

short bitdepth = 0;
int samplerate = 0;
/*************************/


FILE* fileout;	
	char *outfilename; char samplerateflag = 0; char bitflag = 0, attackflag = 0, decayflag = 0, sustainflag = 0, releaseflag = 0;
	char volumeflag = 0, pfflag = 0, frequencyflag = 0, timeflag = 0;
	outfilename = malloc(180); /* If you need more than 90 spots for fname... */
	long i;
	 /*l counts current samples, m counts total samples */
	

	
	
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

		else if(strcmp(argv[i], "-f") == 0 && frequencyflag == 0)
		{ 
			 errno = 0;
			 frequencyflag = 1;
			 frequency = strtod(argv[i+1], NULL);
			 if(errno != 0 || frequency < 0) fprintf(stderr, "\n%s: Invalid value for frequency (non-negative).  Aborting.\n\n", argv[i+1]);
			 i++;
		}

		else if(strcmp(argv[i], "-t") == 0 && timeflag == 0)
		{
			 errno = 0;
			 timeflag = 1;
			 seconds = strtod(argv[i+1], NULL);
			 if(errno != 0 || seconds < 0) fprintf(stderr, "\n%s: Invalid value for time (seconds).  Must be non-negative.  Aborting.\n\n", argv[i+1]);
			 i++;
		}

		else if(strcmp(argv[i], "-v") == 0 && volumeflag == 0)
		{
			 errno = 0;
			 volumeflag = 1;
			 peakvolume = strtod(argv[i+1], NULL);
			 if(errno != 0 || peakvolume < 0 || peakvolume > 1)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for volume (must be 0 <= t <= 1).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "-a") == 0 && attackflag == 0)
		{
			 errno = 0;
			 attackflag = 1;
			 attacktime = strtod(argv[i+1], NULL);
			 if(errno != 0 || attacktime < 0)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for attack time (non-negative).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "-d") == 0 && decayflag == 0)
		{
			 errno = 0;
			 decaytime = strtod(argv[i+1], NULL);
			 decayflag = 1;
			 if(errno != 0 || decaytime < 0)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for decay time (non-negative).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}
		
		else if(strcmp(argv[i], "-s") == 0 && sustainflag == 0)
		{
		 	errno = 0;
		 	 sustainflag = 1;
			 sustainvolume = strtod(argv[i+1], NULL);
			 if(errno != 0 || sustainvolume < 0 || sustainvolume > 1)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for sustain volume (must be 0 <= s <= 1).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "-r") == 0 && releaseflag == 0)
		{
			 errno = 0;
			 releaseflag = 1;
			 releasetime = strtod(argv[i+1], NULL);
			 if(errno != 0 || releasetime < 0)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for release time (non-negative).  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}

		else if(strcmp(argv[i], "--bits") == 0 && bitflag == 0)
		{
			errno = 0;
			bitflag = 1;
			 bitdepth = strtol(argv[i+1], NULL, 10);
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
			 samplerateflag = 1;
			 samplerate = strtol(argv[i+1], NULL, 10);
			 if(errno != 0 || samplerate > 44100 || samplerate < 1)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for samplerate.  No higher than 44100 is supported, and no lower than 1. Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;

		}

		else if(strcmp(argv[i], "--sine") == 0 && sineflag == 0)
		{
			 sineflag = 1;
		}

		else if(strcmp(argv[i], "--triangle") == 0 && triangleflag == 0)
		{
			 triangleflag = 1;
		}

		else if(strcmp(argv[i], "--sawtooth") == 0 && sawtoothflag == 0)
		{
			 sawtoothflag = 1;
		}


		else if(strcmp(argv[i], "--pulse") == 0 && pulseflag == 0)
		{
			 pulseflag = 1;
		}


		else if(strcmp(argv[i], "--pf") == 0 && pfflag == 0)
		{
			 errno = 0;
			 pfflag = 1;
			 pf = strtod(argv[i+1], NULL);
			 if(errno != 0 || pf < 0 || pf > 1)
			 {
			 	fprintf(stderr, "\n%s: Invalid value for pf (must be 0 <= pf <= 1.  Aborting.\n\n", argv[i+1]);
			 		exit(1);
			 } 
			 i++;
		}
		/* Any other unrecogznied switches. */
		else 
		{	
			fprintf(stderr, "\n%s: Unrecogznied/duplicate switch.  Please see help screen with -h.\n\n", argv[i]);
			exit(1);
		}
	}
	


	/*Switches done, now handle both cases */ 
	if(helpflag == 1) /*Print out the help screen and close.*/
	{
		help();
	}

	if(bitflag == 0 || samplerateflag == 0)
	{
		fprintf(stderr, "\nERROR!  Bitrate and/or samplerate not specified.  Use the necessary switches (see -h for help).\n\n");
		exit(1);
	}

	/* All error checks on switches/args */
	if(attackflag == 0) {fprintf(stderr, "\nERROR!  Attack time not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(decayflag == 0) {fprintf(stderr, "\nERROR!  Decay time not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(sustainflag == 0) {fprintf(stderr, "\nERROR!  Sustain volume level not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(releaseflag == 0) {fprintf(stderr, "\nERROR!  Release time not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(pfflag == 0 && pulseflag == 1) {fprintf(stderr, "\nERROR!  Pf value not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(timeflag == 0) {fprintf(stderr, "\nERROR!  Time of sound not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(frequencyflag == 0) {fprintf(stderr, "\nERROR!  Frequency not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}
	if(volumeflag == 0) {fprintf(stderr, "\nERROR!  Maximum volume level not specified.  Use the necessary switches (see -h for help).\n\n"); exit(1);}


	if(outflag == 0) outfilename = "Stdout";

	/* Need to create a function for each wave.  And generate a struct. */
	fileinfo file_info;

	file_info.filename = outfilename;
	char *extension = strrchr(outfilename, '.');
	if(extension == NULL) file_info.extension = "";
	else file_info.extension = extension + 1;

	file_info.numsamples = samplerate * seconds;
	file_info.numchannels = 1;
	file_info.samplerate = samplerate;
	file_info.seconds = seconds;
	file_info.bitdepth = bitdepth;

	/* Check if we need to shorten anything */
	char whitenoiseflag = 0;
	double sustaintime = seconds - attacktime - decaytime - releasetime;

	if(sustaintime < 0)
	{
		sustaintime = 0;
		decaytime = seconds - attacktime - releasetime;
		if(decaytime < 0)
		{
			decaytime = 0;
			attacktime = seconds - releasetime;
			if(attacktime < 0)
			{
				if(releasetime > seconds)
				{
					whitenoiseflag = 1;
				}
			}
		}
	}
	
	/* Malloc the required number of samples */
	if(NULL == (file_info.samples = malloc(file_info.numsamples * file_info.numchannels * sizeof(int) + 10))){ fprintf(stderr, "MALLOC FAILED! In sndgen.\n"); exit(1);}

	/* Calulate sample at end of attack */
	int lastattacksample = (samplerate * attacktime);
	int lastdecaysample = (samplerate * (attacktime + decaytime));
	int lastsustainsample = (samplerate * (attacktime + decaytime + sustaintime));
	int lastreleasesample = (samplerate * (attacktime + decaytime + sustaintime + releasetime));


	//float peakattackvolume = peakvolume * (pow(2, bitdepth -1) -1);
	/* Sustain volume is min decay volume */

	float attackslope = ((float)(peakvolume - 0)/(attacktime - 0));
	float decayslope = ((float)(sustainvolume * peakvolume - peakvolume)/((decaytime + attacktime) - attacktime));
	float releaseslope = ((float)(sustainvolume * peakvolume - 0)/((seconds - releasetime) - seconds));

	float samplespertime = ((float)seconds/file_info.numsamples);
	float timer = 0.0;

	float attackincrements = attackslope * samplespertime;
	float decayincrements = decayslope * samplespertime;
	float releaseincrements = releaseslope * samplespertime;

	int holder;

	for(i = 0; i < file_info.numsamples * file_info.numchannels; i++) file_info.samples[i] = 0;

	


	/* Sine wave */
	if(sineflag == 1 && sawtoothflag == 0 && pulseflag == 0 && triangleflag == 0 && whitenoiseflag == 0)
	{
			/* Attack stage (sin) */
		for(i = 0; i < lastattacksample; i++)
		{
			file_info.samples[i] = (float) (i * attackincrements) * sin(6.28 * frequency * timer) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;

		/* Decay Stage */
		for(i; i < lastdecaysample; i++)
		{
			file_info.samples[i] = (float) (peakvolume + (i - holder) * decayincrements) * sin(6.28 * frequency * timer) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}	
		holder = i;

		/* Sustain Stage */
		for(i; i < lastsustainsample; i++)
		{
			file_info.samples[i] = (float) (sustainvolume * peakvolume) * sin(6.28 * frequency * timer) * (pow(2, bitdepth) -1);
			timer += samplespertime;
		}
		holder = i;

		/* Release Stage */
		for(i; i < lastreleasesample; i++)
		{
			file_info.samples[i] = (float)(peakvolume * sustainvolume + (i - holder) * releaseincrements) * sin(6.28 * frequency * timer) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}

	}

	/* Sawtooth wave.  4th degree fourier series 2/3.14*/
	else if(sineflag == 0 && sawtoothflag == 1 && pulseflag == 0 && triangleflag == 0 && whitenoiseflag == 0)
	{
		/* Attack stage (sawtooth) */
		for(i = 0; i < lastattacksample; i++)
		{
			file_info.samples[i] = (float) (2/3.14)*((i * attackincrements) * sin(6.28 * frequency * timer) - ((i * attackincrements)) * sin(2 * (6.28 * frequency * timer)) + (1/3)*((i * attackincrements)) * sin(3 * (6.28 * frequency * timer)) - ((i * attackincrements)/4) * sin(4 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;

		/* Decay Stage */
		for(i; i < lastdecaysample; i++)
		{
			file_info.samples[i] = (float) (2/3.14)*((peakvolume + (i - holder) * decayincrements) * sin(6.28 * frequency * timer) - ((peakvolume + (i - holder) * decayincrements) * sin(2 * (6.28 * frequency * timer)) + (1/3)*((peakvolume + (i - holder) * decayincrements)) * sin(3 * (6.28 * frequency * timer)) - ((peakvolume + (i - holder) * decayincrements)/4) * sin(4 * (6.28 * frequency * timer)))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;	

		/* Sustain Stage */
		for(i; i < lastsustainsample; i++)
		{
			file_info.samples[i] = (float) (2/3.14)*((sustainvolume * peakvolume) * sin(6.28 * frequency * timer) - ((sustainvolume * peakvolume) * sin(2 * (6.28 * frequency * timer)) + (1/3)*((sustainvolume * peakvolume)) * sin(3 * (6.28 * frequency * timer)) - ((sustainvolume * peakvolume)/4) * sin(4 * (6.28 * frequency * timer)))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;	

		/* Release Stage */
		for(i; i < lastreleasesample; i++)
		{
			file_info.samples[i] = (float) (2/3.14)*((peakvolume * sustainvolume + (i - holder) * releaseincrements) * sin(6.28 * frequency * timer) - ((peakvolume * sustainvolume + (i - holder) * releaseincrements)) * sin(2 * (6.28 * frequency * timer)) + (1/3)*((peakvolume * sustainvolume + (i - holder) * releaseincrements)) * sin(3 * (6.28 * frequency * timer)) - ((peakvolume * sustainvolume + (i - holder) * releaseincrements)/4) * sin(4 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}


	}

	/* Pulse k = 4 fourier series.  Not super accurate, but accurate enough. 4/pi*/
	else if(sineflag == 0 && sawtoothflag == 0 && pulseflag == 1 && triangleflag == 0 && whitenoiseflag == 0)
	{
		/* Attack stage (pulse) */
		for(i = 0; i < lastattacksample; i++)
		{
			file_info.samples[i] = (float) ((i * attackincrements)*(2/3.14) * sin(pf* 3.14 * frequency) * cos(6.28 * frequency * timer) + (i * attackincrements)*(1/3.14) * sin(2 * pf* 3.14 * frequency) * cos(2 * 6.28 * frequency * timer) + (i * attackincrements)*(2/(3*3.14)) * sin(3 * pf* 3.14 * frequency) * cos(3 * 6.28 * frequency * timer) + (i * attackincrements)*(1/(2 * 3.14)) * sin(4* pf* 3.14 * frequency) * cos(4 * 6.28 * frequency * timer)) * (pow(2, bitdepth -1) -1); 	
			//file_info.samples[i] = (float) ((i * attackincrements) * sin(6.28 * frequency * timer) + ((i * attackincrements)/3) * sin(3 * (6.28 * frequency * timer)) + ((i * attackincrements)/5) * sin(5 * (6.28 * frequency * timer)) + ((i * attackincrements)/7) * sin(7 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;

		/* Decay Stage */
		for(i; i < lastdecaysample; i++)
		{
			file_info.samples[i] = (float) ((peakvolume + (i - holder) * decayincrements)*(2/3.14) * sin(pf* 3.14 * frequency) * cos(6.28 * frequency * timer) + (peakvolume + (i - holder) * decayincrements)*(1/3.14) * sin(2 * pf* 3.14 * frequency) * cos(2 * 6.28 * frequency * timer) + (peakvolume + (i - holder) * decayincrements)*(2/(3*3.14)) * sin(3 * pf * 3.14 * frequency) * cos(3 * 6.28 * frequency * timer) + (peakvolume + (i - holder) * decayincrements)*(1/(2 * 3.14)) * sin(4* pf* 3.14 * frequency) * cos(4 * 6.28 * frequency * timer)) * (pow(2, bitdepth -1) -1);
			//file_info.samples[i] = (float) ((peakvolume + (i - holder) * decayincrements) * sin(6.28 * frequency * timer) + ((peakvolume + (i - holder) * decayincrements)/3) * sin(3 * (6.28 * frequency * timer)) + ((peakvolume + (i - holder) * decayincrements)/5) * sin(5 * (6.28 * frequency * timer)) + ((peakvolume + (i - holder) * decayincrements)/7) * sin(7 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;	

		/* Sustain Stage */
		for(i; i < lastsustainsample; i++)
		{
			file_info.samples[i] = (float) ((sustainvolume * peakvolume)*(2/3.14) * sin(pf* 3.14 * frequency) * cos(6.28 * frequency * timer) + (sustainvolume * peakvolume)*(1/3.14) * sin(2 * pf* 3.14 * frequency) * cos(2 * 6.28 * frequency * timer) + (sustainvolume * peakvolume)*(2/(3*3.14)) * sin(3 * pf* 3.14 * frequency) * cos(3 * 6.28 * frequency * timer) + (sustainvolume * peakvolume)*(1/(2 * 3.14)) * sin(4* pf* 3.14 * frequency) * cos(4 * 6.28 * frequency * timer)) * (pow(2, bitdepth -1) -1);
			//file_info.samples[i] = (float) ((sustainvolume * peakvolume) * sin(6.28 * frequency * timer) + ((sustainvolume * peakvolume)/3) * sin(3 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume)/5) * sin(5 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume)/7) * sin(7 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;	

		/* Release Stage */
		for(i; i < lastreleasesample; i++)
		{
			file_info.samples[i] = (float) ((sustainvolume * peakvolume + (i - holder) * releaseincrements)*(2/3.14) * sin(pf* 3.14 * frequency) * cos(6.28 * frequency * timer) + (sustainvolume * peakvolume + (i - holder) * releaseincrements)*(1/3.14) * sin(2 * pf* 3.14 * frequency) * cos(2 * 6.28 * frequency * timer) + (sustainvolume * peakvolume + (i - holder) * releaseincrements)*(2/(3*3.14)) * sin(3 * pf* 3.14 * frequency) * cos(3 * 6.28 * frequency * timer) + (sustainvolume * peakvolume + (i - holder) * releaseincrements)*(1/(2 * 3.14)) * sin(4* pf* 3.14 * frequency) * cos(4 * 6.28 * frequency * timer)) * (pow(2, bitdepth -1) -1);
			//file_info.samples[i] = (float) ((sustainvolume * peakvolume + (i - holder) * releaseincrements) * sin(6.28 * frequency * timer) + ((sustainvolume * peakvolume + (i - holder) * releaseincrements)/3) * sin(3 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume + (i-holder) * releaseincrements)/5) * sin(5 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume + (i-holder) * releaseincrements)/7) * sin(7 * (6.28 * frequency * timer))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}

	}

	/* Triangle 4th degree fourier.  Accurate enough. 8/pi^2*/
	else if(sineflag == 0 && sawtoothflag == 0 && pulseflag == 0 && triangleflag == 1 && whitenoiseflag == 0)
	{
		/* Attack stage (triangle) */
		for(i = 0; i < lastattacksample; i++)
		{
			file_info.samples[i] = (float) (8/(3.14*3.14))*(((i * attackincrements) * sin(6.28 * frequency * timer) - ((i * attackincrements)/9) * sin(3 * (6.28 * frequency * timer)) + ((i * attackincrements)/25) * sin(5 * (6.28 * frequency * timer)) - ((i * attackincrements)/49) * sin(7 * (6.28 * frequency * timer)))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;

		/* Decay Stage */
		for(i; i < lastdecaysample; i++)
		{
			file_info.samples[i] = (float) (8/(3.14*3.14))*(((peakvolume + (i - holder) * decayincrements)) * sin(6.28 * frequency * timer) - ((peakvolume + (i - holder) * decayincrements)/9) * sin(3 * (6.28 * frequency * timer)) + ((peakvolume + (i - holder) * decayincrements)/25) * sin(5 * (6.28 * frequency * timer)) - ((peakvolume + (i - holder) * decayincrements)/49) * sin(7 * (6.28 * frequency * timer))) * (pow(2, bitdepth-1) -1);
			timer += samplespertime;
		}
		holder = i;	

		/* Sustain Stage */
		for(i; i < lastsustainsample; i++)
		{
			file_info.samples[i] = (float) (8/(3.14*3.14))*(((sustainvolume * peakvolume) * sin(6.28 * frequency * timer) - ((sustainvolume * peakvolume)/9) * sin(3 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume)/25) * sin(5 * (6.28 * frequency * timer)) - ((sustainvolume * peakvolume)/49) * sin(7 * (6.28 * frequency * timer)))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		holder = i;	
		
		/* Release Stage */
		for(i; i < lastreleasesample; i++)
		{
			file_info.samples[i] = (float) (8/(3.14*3.14))*(((sustainvolume * peakvolume + (i - holder) * releaseincrements)  * sin(6.28 * frequency * timer) - ((sustainvolume * peakvolume + (i - holder) * releaseincrements)/9) * sin(3 * (6.28 * frequency * timer)) + ((sustainvolume * peakvolume + (i - holder) * releaseincrements)/25) * sin(5 * (6.28 * frequency * timer)) - ((sustainvolume * peakvolume + (i - holder) * releaseincrements)/49) * sin(7 * (6.28 * frequency * timer)))) * (pow(2, bitdepth -1) -1);
			timer += samplespertime;
		}
		
	}
	/* release phase is longer than the total time */
	else if(whitenoiseflag == 1){}

	/* No waveform selected. Error. */
	else
	{
		fprintf(stderr, "\nYou have not selected a waveform type, or have selected too many.  Please see the help screen with -h.\n\n");
		exit(1);
	}


	
	/* Finished generating, now rebuild the output file*/
	if(wavflag == 1) /*output to wav format.*/
	{
		fileout = createWav(&file_info, outfilename);	

		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info.filename);
			exit(1);	
		}

		/* Flush to stdout, destroy the file we created. */
		if(outflag == 0)
		{
			fclose(fileout);
			FILE* tempfile = fopen(outfilename, "r");
			if(fileout == NULL)
			{
				fprintf(stderr, "\n%s On read-back, .cs229 file failed to open.\n\n", file_info.filename);
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
		fileout = createCS229(&file_info, outfilename);
		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info.filename);
			exit(1);	
		}

		/* Flush to stdout, and destroy the file we created. */
		if(outflag == 0)
		{
			fclose(fileout);
			FILE* tempfile = fopen(outfilename, "r");
			if(fileout == NULL)
			{
				fprintf(stderr, "\n%s On read-back, .cs229 file failed to open.\n\n", file_info.filename);
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

	fprintf(stderr, "\nSuccess! %s was written with the specified properties.\n\n", file_info.filename);

	return 0;
}



void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n");
	printf("\tSndgen\n");
	printf("*****************************\n");
	printf("\tSupported Switches:");
	printf("-h (help), -w (output wav file instead of cs229), \n\t-o filename (output to specified filename, otherwise stdout)");
	printf("\n\tMore Switches: \n\t--bits n : use a bit depth of n\n\t--sr n : use a sample rate of n\n\t-f r : use a frequency of r Hz\n\t-t r : total duration of r seconds\n\t-v p : Peak volume. 0 <= p <= 1\n\t-a r : attack time of r seconds\n\t-d r : decay time of r seconds\n\t-s p : sustain volume. 0 <= p <= 1\n\t-r r : release time of r seconds\n\t--sine : generate a sine wave\n\t--triangle : generate a triangle wave\n\t--sawtooth : generate a sawtooth wave\n\t--pulse : generate a pulse wave\n\t--pf p : Fraction of the time the pulse wave is up\n\t");
	printf("\tMult must be a value specified between -10 and 10, and Inputfile must be a valid file path.\n");
	printf("\tOnce all desired switches are set, sndgen constructs your sound file.\n\n");
	printf("\tIMPORTANT NOTE: PLEASE ENSURE YOU SPECIFY ALL SWITCHES FOR ADSR. \n\tPLEASE MAKE SURE TO SET ALL DESIRED SWITCHES! There are quite a few :D.\n\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n");
	printf("\tThis program will exit after writing the new sound file containing the desired audio file that conforms to the user specified ADSR envelope.\n");
	printf("\nPlease view the README file for more detailed information.\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}

