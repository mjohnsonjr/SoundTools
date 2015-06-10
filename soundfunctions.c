#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "soundfunctions.h"


/* This file contains helper functions used by all of the programs required for this project.
Features:
	- fill a fileinfo struct with both .cs229 and .wav data.
	- print out file information for both .cs229 and .wav
	- create a .wav or .cs229 file from a fileinfo struct
	- fill an abc229 struct with data from a valid .abc229 file.

The struct fileinfo is declared and defined in soundfunctions.h header file.

/***************************************************************/




/* This function prints information about the sound file.. there are NO ERROR CHECKS HERE.  This function
is only called by me as a helper function so it's okay. */

void printfileinfo(fileinfo* file_info)
{
	printf("File name: %s\n", file_info->filename);
	printf("File type: %s\n", file_info->extension);
	printf("Sample Rate: %d\n", file_info->samplerate);
	printf("Bit Depth: %d\n", file_info->bitdepth);
	printf("Number of Channels: %d\n", file_info->numchannels);
	printf("Number of Samples: %ld\n", file_info->numsamples);
	printf("Length: %.2lf seconds\n\n", file_info->seconds);

}

/* Fills the struct pointer with information about the wav file.
* The int return type is for error checking, if -1, an error has occured
* reading the file (it is corrupt or not formatted correctly).  Otherwise
* function returns 0 for a successful read.
*/
int fillWavInfo(fileinfo* file_info, FILE* file)
{
		char invalidflag = 0; /* ADD A CHECK TO FILE EXTENSION */
			
			/*Start reading bytes */
			BYTE* buffer;
			
			if((buffer = malloc(20)) == NULL) {fprintf(stderr, "MALLOC FAILED! Aborting.."); exit(1);} 
			/* All items are 2-4 byte. Can skip up to 20. */ 
			
			/*Check DATA Field for validity. */	
			fseek(file, 36, SEEK_SET);
			fread(buffer, 4, 1, file);
			buffer[4] = 0;
			if(strcasecmp(buffer, "DATA") != 0) invalidflag = 1;
			
			/* RIFF */
			rewind(file);
			fread(buffer, 4, 1, file);
			/*string convert.*/
			buffer[4] = 0;
			if(strcasecmp(buffer, "RIFF") != 0) invalidflag = 1;
	
			/*Two more reads (BYTE 8) should say (WAVE).*/
			fseek(file, 8, SEEK_SET);
			fread(buffer, 4, 1, file);
			buffer[4] = 0;
			if(strcasecmp(buffer, "WAVE") != 0) invalidflag = 1;
			/*Final checks if file is bad before writing */
			
			if(invalidflag == 1) return -1;  
			
		/*Read byte 22, 23 to obtain numchannels */
			fseek(file, 22, SEEK_SET); /*skip over 10 bytes */
			fread(buffer, 2, 1, file);

			file_info->numchannels = buffer[0] + (buffer[1] << 8);
			
		/* Sample rate (byte 24) */
			fseek(file, 24, SEEK_SET);
			fread(buffer, 4, 1, file);
			file_info->samplerate = ((buffer[0]) + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24));
			
		/* Read bitdepth seek 6 bytes (BYTE 34) */
			fseek(file, 34, SEEK_SET);
			fread(buffer, 2, 1, file);
			file_info->bitdepth = buffer[0] + (buffer[1] << 8);

			/* Enforce 8, 16, or 32 bit policy */
			if(file_info->bitdepth != 8 && file_info->bitdepth != 16 && file_info->bitdepth != 32) return -1;
			
		/*Determine chunk size (BYTE 40) to calculate the other fields. */
			fseek(file, 40, SEEK_SET);
			fread(buffer, 4, 1, file);
			int subchunksize = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
			file_info->numsamples = subchunksize/(file_info->numchannels * (file_info->bitdepth/8));
			/* I consider in my code that a sample counts for each channel, so 2 chans: left is 1, right is another */

		/* Time is simple algebra, calculating. */
			file_info->seconds = ((double)file_info->numsamples)/file_info->samplerate;
			free(buffer);
			
		/* Read actual samples TRYING!*/
		
		buffer = malloc(file_info->bitdepth/8 +1);
														//THIS MAY STILL BE WRONG.
		file_info->samples = malloc(file_info->numsamples * file_info->numchannels * sizeof(int) + 10); 
		
		if(file_info->samples == NULL) 
		{
			fprintf(stderr, "MALLOC FAILED in fillWavInfo(). Aborting.\n\n");
			exit(1);
		}
		
		fseek(file, 44, SEEK_SET);
		
		/* May consider separating, this below considers bitrate of WAV and breaks it up into individual samples
		that can be used by something else or to converted */ 
		int temp = 0, temp2; 
		long k,l;
		/* This writes byte by byte, so we need sizeof(int), and numsamples (signed ints)/ numchannels to accurately write. */	
		for( k = 0; k < (file_info->numsamples) * file_info->numchannels; k++)//=file_info->bitdepth/8)
		{	
			fread(buffer, (file_info->bitdepth)/8, 1, file); /*Reads x bytes */
			for(l = 0; l < (file_info->bitdepth/8); l++)
			{	
				temp2 = ((int)buffer[l]);
				/* this below makes it become signed by killing any leading zeros.. */
				temp = (((temp2 << (l * 8)) | temp) << (sizeof(int) * 8 - file_info->bitdepth)) >> (sizeof(int)*8 - file_info->bitdepth); 
				/* that char is to convert to signed integer.. needed for cs229.*/
			}					
			/* may need looking at again */ 
			file_info->samples[k] = temp;
			temp = 0;
		}
		
		/* All data including samples filled to and equal medium for conversion. */
			free(buffer);
			return 0;
		
}

/* Fills the struct pointer with information about the .cs229 file.
* The int return type is for error checking, if -1, an error has occured
* reading the file (it is corrupt or not formatted correctly).  Otherwise
* function returns 0 for a successful read.
*/
int fillCS229Info(fileinfo* file_info, FILE* file)
{
	char line[160]; char dataflag = 0, sampleflag = 0, channelflag = 0, bitresflag =0, samplerateflag = 0; 
	char* token; long samplecounter = 0; long k = 0; char invalidflag = 1; char mallocflag = 0;
			token = malloc(160); int channelcheck = 0;
			
			// /* Get filesize */
			// fseek(file, 0, SEEK_END);
			// long filesize = ftell(file);
			// rewind(file);
			
			if((token) == NULL) {fprintf(stderr, "MALLOC FAILED IN fillCS229Info()!  Aborting.\n");  exit(1);}
			
			while(fgets(line, 160, file) != NULL)
			{		
				if(strcasecmp(line, "cs229\n") == 0) {invalidflag = 0; continue;}/*beginning of file*/
				if(strcasecmp(line, "\n") == 0) continue;
				if(strcasecmp(line, "\0") == 0) continue;
				
				if(line[0] == '#') continue; /*cases we skip the line */
				if(strcasecmp(line, "Startdata\n") ==0) {dataflag =1; continue;} /* We've reached the data section. */
	
				/*we need to tokenize the string into 2 pieces before start data.*/
				if(dataflag != 1 && invalidflag == 0)
				{	
					token = strtok(line, " \t"); /*first string */
					errno = 0;
					if(strcasecmp(token, "samples") == 0)
					{
						sampleflag = 1; /* indicates that cs229 tells us the amt of samples */
						token = strtok(NULL, " \t");
						file_info->numsamples = strtol(token, NULL, 10);
						samplecounter = file_info->numsamples;
					}
					
					else if(strcasecmp(token, "channels") == 0)
					{
						channelflag = 1;
						token = strtok(NULL, " \t");
						file_info->numchannels = strtol(token, NULL, 10);

					}
					
					else if(strcasecmp(token, "bitres") == 0)
					{
						bitresflag = 1;
						token = strtok(NULL, " \t");
						file_info->bitdepth = strtol(token, NULL, 10);
					}
					
					else if(strcasecmp(token, "samplerate") == 0)
					{
						samplerateflag = 1;
						token = strtok(NULL, " \t");
						file_info->samplerate = strtol(token, NULL, 10);
					}	
					else
					{
						invalidflag = 1; return -1;
					}

					if(errno != 0)
					{
						invalidflag =1; return -1;
					}
				} 
			
				else if(dataflag == 1 && samplerateflag == 1 && bitresflag == 1 && channelflag == 1 && invalidflag == 0) /*we are in samples now, "Startdata\n"*/
				{
					
					if(sampleflag == 0) 
					{
						/* Counts the samples (lines and calcs them later) to determine length of file. */						
						while(fgets(line, 160, file) != NULL)
						{	
							samplecounter++;
						}
						
						rewind(file);
						while(strncasecmp(line, "StartDATA", 9) != 0) fgets(line, 160, file);
						fgets(line, 160, file);
						file_info->numsamples = samplecounter + 1;
						sampleflag = 1;
					}
					
				if(sampleflag != 1) fprintf(stderr, "FATAL ERROR!  Chain of trust broken?\n");	
					/*May need to revisit if I need specific sample data and tokenize x NUMBER OF CHANNELS! */
					/*For now we just need to count samples. */
				
				/* Stops us from mallocing a million times.. */
				if(mallocflag == 0 && sampleflag == 1)
				{ 																	//CHECK THIS AGAIN.
					file_info->samples = malloc(file_info->numsamples * file_info->numchannels * sizeof(int) +10); 
					mallocflag = 1;
				}
			
			
					token = strtok(line, " \t");
					
					/* Checks to ensure the samples are consistent with the channel # in the header */
					channelcheck = 0;
					
					while(token != NULL)
					{	errno = 0;
						file_info->samples[k] = (((int)(strtol(token, NULL, 10)) << ((sizeof(int)*8)-file_info->bitdepth)) >> ((sizeof(int)*8)-file_info->bitdepth));
						token = strtok(NULL, " \t");
						k++; /* Must increment per sample, not per line.  Samples = line * channels*/
						if(errno != 0) return -1;
						
						/* More cs229 Error Checks */						
						channelcheck++;
						if(channelcheck > file_info->numchannels) return -1;
						//if(k/file_info->numsamples -1 > samplecounter) return -1;
					}
				}
				else
				{
					/* Corrupt or invalid .cs229 */
					return -1;
				}
			}

			/* Check for just partial header */
			if(!(dataflag == 1 && samplerateflag == 1 && bitresflag == 1 && channelflag == 1)) return -1;
			
			/*compute seconds */
			double time = ((double)(file_info->numsamples))/(file_info->samplerate);
			file_info->seconds = time;
			sampleflag = bitresflag = samplerateflag = channelflag = dataflag = 0;
			free(token);
			return 0; /* Success if zero. */
}

/* Creates a wav file from the information inside of the file_info struct, returns NULL on failure. */
FILE* createWav(fileinfo* file_info, char* path)
{
	char charflag = 0;
	/* Safety checks..*/
	if(file_info == NULL) return NULL;
	

	/* done safety checks */
	FILE* file;
	if((file = fopen(path, "w")) == NULL) /*create our file */
	{
		return NULL; /* Failure */
	}
	/* Now we need to fill in binary data at each LOC, including RIFF/DATA fields
		Now it's time to build a .wav from scratch... here we go byte by byte. */ 
	long i, k, l;
	
	/*Chunk ID */
	char riff[4] = "RIFF";
	fwrite(&riff[0], 1, 1, file);
	fwrite(&riff[1], 1, 1, file);
	fwrite(&riff[2], 1, 1, file);
	fwrite(&riff[3], 1, 1, file);
	
	/*Chunk Size (little endian)*/
	BYTE* wavbuffer;
	wavbuffer = malloc(8 +1); 
	int subchunk1 = 16; /* constant */
	long subchunk2 = file_info->numsamples * file_info->numchannels * (file_info->bitdepth/8);
	for(i = 0; i < 4; i++)
	wavbuffer[i] = (BYTE)((4 + (8 + subchunk1) + (8 + subchunk2)) >> (i * 8));

	for(i = 0; i < 4; i++)
	fwrite(&wavbuffer[i], 1, 1, file);
	/* Done Chunk size, looks good*/
	
	/* Format (WAVEfmt ) (big endian) */
	char wave[9] = "WAVEfmt ";
	for(i = 0;i < 8; i++)
	fwrite(&wave[i], 1, 1, file);
	
	/* SBCHNK1 Size  */
	wavbuffer[0] = 16;
	for(i = 1; i < 4; i++) wavbuffer[i] = 0;
	for(i = 0; i < 4; i++)
	fwrite(&wavbuffer[i], 1, 1, file);
	
	/* Audio format (1) */
	wavbuffer[4] = 1;
	wavbuffer[5] = 0;
	fwrite(&wavbuffer[4], 1,1,file);
	fwrite(&wavbuffer[5], 1,1,file);
	
	/* Num Channels */
	for(i = 0; i < 2; i++)
	wavbuffer[i] = (BYTE)((file_info->numchannels) >> (i*8));
	
	for(i = 0; i < 2; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	/* Sample Rate */
	for(i = 0; i < 4; i++)
	wavbuffer[i] = (BYTE)((file_info->samplerate) >> (i*8));
	
	for(i = 0; i < 4; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	/* Byte rate = Samplerate * numchannels * bitdepth/8 */
	long byterate = file_info->samplerate * file_info->numchannels * file_info->bitdepth/8;
	for(i = 0; i < 4; i++)
	wavbuffer[i] = (BYTE)((byterate) >> (i*8));
	
	for(i = 0; i < 4; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	/* Block Align */ 
	int blockalign = file_info->numchannels * file_info->bitdepth/8;
	for(i = 0; i < 2; i++)
	wavbuffer[i] = (BYTE)((blockalign) >> (i*8));
	
	for(i = 0; i < 2; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	/* Bit depth */
	int bitd = file_info->bitdepth;
	for(i = 0; i < 2; i++)
	wavbuffer[i] = (BYTE)((bitd) >> (i*8));
	
	for(i = 0; i < 2; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	/* Data fields */
	char data[5] = "data";
	for(i = 0; i < 4; i++)
	fwrite(&data[i], 1,1, file);
	
	/*Subchunk2 Size */
	for(i = 0; i < 4; i++)
	wavbuffer[i] = (BYTE)((subchunk2) >> (i*8));
	
	for(i = 0; i < 4; i++)
	fwrite(&wavbuffer[i], 1,1, file);
	
	
	/* Raw Sound Data.  Remember, unsigned char (BYTE) for 8bit, 16 and 32 are SIGNED integers */
	
	if(file_info->bitdepth == 8) charflag = 1; /* 8 bit */
	int temp2, temp = 0;
	
		for( k = 0; k < file_info->numsamples * file_info->numchannels; k++)//=file_info->numchannels)
		{	
			temp2 = file_info->samples[k]; /* int */
			/* bit depth */
			for(l = 0; l < (file_info->bitdepth)/8; l++)
			{	
				/* Little endian byte writing */
				temp = (temp2 >> (l * 8));
				if(charflag == 1) wavbuffer[l] = (BYTE)temp;
				else
				{
					wavbuffer[l] = temp;
				}
				fwrite(&wavbuffer[l], 1, 1, file); /*Write 1 bytes */
			}
			
			temp = 0;
		}
		free(wavbuffer);
		return file;
}

/* Creates a wav file from the information inside of the file_info struct, returns Null pointer
if the file_info struct is null */
FILE* createCS229(fileinfo* file_info, char* path)
{
	/* Safety checks.. ensure there is data. */
	if(file_info == NULL) return NULL;
	char buffer[160], stringbuff[160] = "";

	/* done safety checks, create file */
	FILE* file;
	if((file = fopen(path, "w")) == NULL)
	{
		return NULL; /* Failure to create */
	} 
	
	/* ERROR CHECK THIS */
	/* First we do cs229 */
	fputs("CS229\n", file);
	
	/*Samples*/
	sprintf(buffer, "Samples\t%ld\n", file_info->numsamples);
	fputs(buffer, file);
	
	/*BitDepth*/
	sprintf(buffer, "Bitres\t%d\n", file_info->bitdepth);
	fputs(buffer, file);
	
	/*Channels*/
	sprintf(buffer, "Channels\t%d\n", file_info->numchannels);
	fputs(buffer, file);
	
	/*Samples*/
	sprintf(buffer, "SampleRate\t%d\n", file_info->samplerate);
	fputs(buffer, file);
	
	/* Now time for Sample data */
	fputs("StartDATA\n", file);
	
	long i,j = 1;
	int toAdd = 0;

	for(i = 0; i < file_info->numsamples * file_info-> numchannels; i++)//=file_info->bitdepth/8)
	{	
		/* -128 / out of bounds check for .cs229 format. */
		toAdd = file_info->samples[i];
		//if(toAdd == -pow(2, (file_info->bitdepth - 1))) toAdd = file_info->samples[i] + 1;

		if(toAdd == -(2 << (file_info->bitdepth -2))) toAdd = file_info->samples[i] + 1;

		if((j) < (file_info->numchannels)) /* j var counter is merely for formatting based on numchannels */
		{	
			sprintf(buffer, "%d\t", toAdd/*(file_info->samples[i])*/);	/* point to int*, bitdepth/8, reading byte by byte */
			strcat(stringbuff, buffer);
			
		}
		else
		{
			sprintf(buffer, "%d\n", toAdd/*(file_info->samples[i])*/);
			strcat(stringbuff, buffer);
			j = 0;
		}
		fputs(stringbuff, file);
		strcpy(stringbuff,"");
		j++;
		 /* increment pointer to next sample. */
	
	}
	
	return file;
}


/* This will fill a abc229 struct with data read in from a .abc229 file.  The structs are defined in the header file.  Some of the
things include the tempo, instruments, and specific notes for each instrument.  Please note, .abc229 files must contain all
necessary information (tempo, attack, decay, etc. for each instrument).  If they do not, an error will be thrown.  Values exhibit
undefined behavior if not specified.  So the user must have an .abc229 file that is completely valid or it will throw an error. This
function returns 0 on success, and returns -1 on failure (usually due to a corrupt or invalid .abc229 file). */

int fillABC229Info(abc229* abc, FILE* file)
{
		char* token = malloc(180); char invalidflag = 1; char tempoflag = 0; char releaseflag = 0; char attackflag = 0;
		char line[160]; long i, j;	char sustainflag = 0; char decayflag =0; char volumeflag =0; char waveformflag = 0, pulsefracflag = 0;

		abc->instruments = malloc(sizeof(instrument) + 1); /* Spot holder for realloc */
		
		/* Need init 0 instruments.. no idea how it worked without this until I tried pyrite. */
		abc->numinstruments = 0;
	
		if((token) == NULL) {fprintf(stderr, "MALLOC FAILED IN fillABC229Info()!  Aborting.\n");  exit(1);}
			
			while(fgets(line, 160, file) != NULL)
			{	errno = 0;	
				
				if(strcasecmp(line, "abc229\n") == 0) {invalidflag = 0; continue;}/*beginning of file*/
				if(strcasecmp(line, "\n") == 0) continue;
				if(strcasecmp(line, "\0") == 0) continue;
				
				if(line[0] == '%') continue; /*cases we skip the line */

				
				/*we need to tokenize the string into 2 pieces before start data.*/
				if(invalidflag == 0)
				{	
					token = strtok(line, " \t"); /*first string */
					
					if(strcasecmp(token, "tempo") == 0 && tempoflag == 0)
					{
						tempoflag = 1; /* indicates that cs229 tells us the amt of samples */
						token = strtok(NULL, " \t");
						abc->tempo = strtol(token, NULL, 10);
					}

					else if(strcasecmp(token, "instrument") == 0)
					{
						token = strtok(NULL, " \t");
						abc->numinstruments++;
						abc->instruments = realloc(abc->instruments, abc->numinstruments * sizeof(instrument) + 1);
						abc->instruments[abc->numinstruments -1].pulsefrac = 1; /* default pulse frac */ 
					}
					
					else if(strcasecmp(token, "waveform") == 0)
					{
						token = strtok(NULL, " \t");
						token[strlen(token) -1] = 0; /* Kill newline */
						strcpy(abc->instruments[abc->numinstruments -1].waveform, token);	
						waveformflag = 1;
					}

					else if(strcasecmp(token, "volume") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].volume = strtod(token, NULL);
						volumeflag = 1;
					}
					else if(strcasecmp(token, "attack") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].attack = strtod(token, NULL);
						attackflag =1;
					}
					else if(strcasecmp(token, "decay") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].decay = strtod(token, NULL);
						decayflag = 1;
					}
					else if(strcasecmp(token, "sustain") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].sustain = strtod(token, NULL);
						sustainflag = 1;
					}
					else if(strcasecmp(token, "release") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].release = strtod(token, NULL);
						releaseflag = 1;
					}
					else if(strcasecmp(token, "pulsefrac") == 0)
					{
						token = strtok(NULL, " \t");
						abc->instruments[abc->numinstruments -1].pulsefrac = strtod(token, NULL);
						pulsefracflag = 1;
					}

					/* notes/frequencies below.  Also checks , and ' for octave shift */
					else if(strncasecmp(token, "score", 5) == 0)
					{		
						/* Error check */
						if(releaseflag == 1 && sustainflag == 1 && decayflag == 1 && attackflag == 1 && volumeflag == 1 && waveformflag == 1 && tempoflag == 1){}
						else {return -1;}

						while(fgets(line, 160, file) && line[0] != '['){}
						
						abc->instruments[abc->numinstruments -1].notes = malloc(sizeof(int) + 1);
						abc->instruments[abc->numinstruments -1].numnotes = 0;

						while(fgets(line, 160, file) && line[0] != ']')
						{	
							for(i = 0; line[i] != '\n' && line[i] != 0; i++)
							{	//printf("%c,", line[i]);
								if(line[i] == '\'')
								{
									abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes -1] *= 2;
								}
								else if(line[i] == ',')
								{
									abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes -1] /= 2;
								}
									/* Number from 1 - 9 */
								else if(line[i] >= 48 && line[i] <= 57)
								{
									for(j = 0; j < (line[i] - 48); j++)
									{	if(line[i + 1] == '/')
										{
											//while(!((line[i] >= 65 && line[i] <= 90) || (line[i] >= 97 || line[i] <= 122))) i++;
											break;
										}
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes -1];
									} 
								}
								else if(line[i] == 'a')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 466;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 440;	
									}
								}
								
								else if(line[i] == 'b')
								{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 493;	
								}

								else if(line[i] == 'c')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 554;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 523;	
									}
								}
							
								else if(line[i] == 'd')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 622;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 587;	
									}
								}

								else if(line[i] == 'e')
								{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 659;	
								}

								else if(line[i] == 'f')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 740;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 698;	
									}
								}

								else if(line[i] == 'g')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 830;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 784;	
									}
								}

									/* Below these are one octave BELOW */

								else if(line[i] == 'A')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 233;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 220;	
									}
								}

								else if(line[i] == 'B')
								{
									abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
									abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 246;
								}

								else if(line[i] == 'C')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 277;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 261;	
									}
								}

								else if(line[i] == 'D')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 311;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 293;	
									}
								}

								else if(line[i] == 'E')
								{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 329;	
								}

								else if(line[i] == 'F')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 370;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 349;	
									}
								}

								else if(line[i] == 'G')
								{
									if(line[i + 1] == '#')
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 415;
										i++;
									}
									else
									{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 392;	
									}
								}

								else if(line[i] == 'z' || line[i] == 'Z')
								{
										abc->instruments[abc->numinstruments -1].notes = realloc(abc->instruments[abc->numinstruments -1].notes, sizeof(int) * abc->instruments[abc->numinstruments -1].numnotes + 1); 
										abc->instruments[abc->numinstruments -1].notes[abc->instruments[abc->numinstruments -1].numnotes++] = 0;
								}
							}
							releaseflag = sustainflag = decayflag = attackflag = volumeflag = waveformflag = 0;
						}
					}
				}
				else
				{
					invalidflag = 1; return -1;
				}

				if(errno != 0)
				{	
					if(errno == ENOMEM)
					{
						fprintf(stderr, "\nMALLOC/REALLOC FAILED! In fillABC229Info()\n\n");
						exit(1);
					} 
					invalidflag = 1; return -1;
				}
			} 

			return 0; /* Success if zero. */
}

