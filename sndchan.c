#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "soundfunctions.h"



/*
	IMPORTANT:  USE PROPER FILE EXTENSIONS (e.g. .wav or .cs229).
				(e.g. "filename.wav" cannot be passed as "filename"  It must be passed as filename.wav 
				or this program will not recognize it.)

	This program reads sound files passed as arguments, and writes them to a single sound file that is the 
	concatenation of all the channels of the sounds files.  If no files are passed as args then read from standard input, 
	if no -o switch write to standard output.


	Switches: -h, -o filename, -w, -c n (This will output a sound file with ONLY that channel, all other channels lost.)
	NOTE: IF -c switch is used, then the output file is created, all channels EXCEPT n will be zeroed out.
			The spec is not clear on this, this is the way I interpreted it.  The output sound file only contains
			that channel specified in the -c switch.  But the file still reports the amount of channels that would
			exist without the switch.  So, if you play this on a multichannel setup, only the specific speaker will
			play the sound.  It will not play the one channel thru all the speakers.  So if you have only 2 channels and your
			file only has sound from channel 3, you will not hear anything.  Please use an adequate speaker setup for this.

			Example: 2 channels as a result, with -c 1 switch.  The file will have 2 channels, but the 2nd one is 
			zeroed out.  So, only sound will come out of the left speaker (channel 1) and the right speaker will
			be silent.  This is how I interpreted it and is intentional.

	Args: Inputfile1, Inputfile 2... Inputfile N. (Warning: too many files will make an output with a 
	ridiculous amount of channels... :D)

WARNING: If your output file only has 1 channel, most players will play that channel out of all channels. 
Also, if you use iTunes, it doesn't seem to want to play files with more than 2 channels.  Use another player
like VLC or Quicktime.

Since we use stdout if no -o switch is used, we print all user messages to stderr which goes to the console.  Stdout
will go to a file of your choice using > on the command line, or the console otherwise.  Output will only go to stdout
when the -o is NOT used.

*/



void help();

int main(int argc, char** argv)
{

/* flags and vars */
char helpflag = 0;
char wavflag =0;
char outflag = 0;	/*write to stdout or to this file if flag is up */
char fileflag = 0; /* determines if this is file1 or file2*/
char channelflag = 0; /* -c switch initiated */
int usechannel = -1;  /* If -1, all channels, else only that channel specifically */

FILE* fileout;

	int numfiles = 1;
	FILE* files[argc]; /* max number of files, obviously slightly more than needed. */
	
	char *outfilename;
	char* inputfilenames[80], command[100];
	inputfilenames[0] = malloc(4);
	outfilename = malloc(90); /* If you need more than 90 spots for fname... */
	int i, j = 0; char *tok;


	long m = 0; /*l counts current samples, m counts total samples */
	tok = malloc(90);
	
	
	for(i =1; i < argc; i++)
	{
		tok = *(argv + i);/*used to get the first char of the string pointer to check for non-switched args */
		
		if(strcmp(argv[i], "-h") == 0 && helpflag == 0)  helpflag = 1; //set help flag.
		
		else if(strcmp(argv[i], "-w") == 0 && wavflag == 0) wavflag = 1; //wav flag
		
		else if(strcmp(argv[i], "-o") == 0 && outflag == 0)
		{
			 outflag = 1; 
			 outfilename = argv[i+1];
			 i++;
		}
		/* Determines if we want to only write a specific channel. */
		else if(strcmp(argv[i], "-c") == 0 && channelflag == 0)
		{	errno = 0;
			 channelflag = 1; //output file flag
			 usechannel = strtol(argv[i+1], NULL, 10);
			 if( usechannel == 0 || errno != 0 || usechannel < 1)
				{
					fprintf(stderr, "\nPlease use a valid channel number for -c.  Aborting.\n\n");
					exit(1);
				}
			 i++;
		}
		else if(tok[0] != '-')
		{	
			fileflag = 1;
			if((files[numfiles]=fopen(argv[i], "r")) == NULL)
			{
				fprintf(stderr, "\nFailed to open \"%s\"...Skipping this file.\n\n", argv[i]);
				continue;
			}
			inputfilenames[numfiles] = malloc(strlen(argv[i]) +1);	
			inputfilenames[numfiles] = argv[i];
			numfiles++;
		}
		else
		{
			fprintf(stderr, "\nUnknown/duplicate switch: \"%s\" Please use -h for help screen.\n\n", argv[i]);
			exit(1);
		}

	}
	
	/*Switches done, now handle both cases */ 
	if(helpflag == 1) /*Print out the help screen and close.*/
	{
		help();
	}

		/* Read a file from stdin */
	if(numfiles == 1)
	{
		fprintf(stderr, "Reading a file from standard input (type or pipe filename in):\n");
		fgets(command, 160, stdin);
		command[strlen(command) -1] = 0; /* Kill newline */

		if((files[numfiles]=fopen(command, "r")) == NULL)
		{
				fprintf(stderr, "\n\nFailed to open \"%s\" from standard input...Aborting.\n\n", argv[i]);
				exit(1);
		}

		inputfilenames[numfiles] = malloc(strlen(command) +1);
		inputfilenames[numfiles] = command;
		numfiles++;
	}

	/* Check stdout conditions */
	if(outflag == 0)
	{
		outfilename = "Stdout";
	}
	
	/* Create my structs for each file, identifying what extension it is.  Extensions have already been safety checked. */
	fileinfo file_info[numfiles];
	
	for(i = 1; i < numfiles; i++)
	{	
		file_info[i].filename = inputfilenames[i];
		char *extension = strrchr(inputfilenames[i], '.') + 1;
		file_info[i].extension = extension;
		
		/* Setup files and load data into structs, error checks */
		if(strcmp(extension, "wav") == 0)
		{
			char err = fillWavInfo(&file_info[i], files[i]);
			if(err < 0)
			{
				fprintf(stderr, "ERROR, %s is an invalid file of type %s\n", file_info[i].filename, file_info[i].extension);	
				exit(1);
			} 
		}
		else if(strcmp(extension, "cs229") == 0)
		{
			char err = fillCS229Info(&file_info[i], files[i]);
			if(err < 0)
			{
				fprintf(stderr, "ERROR, %s is an invalid file of type %s\n", file_info[i].filename, file_info[i].extension);
				exit(1);
			} 
		}
	}

	/*for(i = 1; i < numfiles; i++)
	printfileinfo(&file_info[i]);*/

	
	/* Check for differences and scale bitdepths, use higher one.. Channels.. fill extra with zero, use higher. */
	int bitdepth = 0, numchannels = 0; long numsamples = 0; double seconds = 0.0;
	for(i = 1; i < numfiles; i++) if(file_info[i].bitdepth > bitdepth) bitdepth = file_info[i].bitdepth;
	

	/* Add all the channels together in the new file */
	for(i = 1; i < numfiles; i++) numchannels = numchannels + file_info[i].numchannels;
	

	/* Number of bytes in the file will be smaller than this is we cut channels, but we're still mallocing all */
	for(i = 1; i < numfiles; i++) if(numsamples < file_info[i].numsamples) numsamples = file_info[i].numsamples;
	//for(i = 1; i < numfiles; i++) seconds = seconds + file_info[i].seconds;

	/* Determine the time (seconds) for the new file */
	for(i = 1; i < numfiles; i++) if(seconds < file_info[i].seconds) seconds = file_info[i].seconds;
	

	/* Sample rate check.  Sample rates must be the same or the program will terminate. */
	for(i = 1; i < numfiles; i++) 
	{
		if(file_info[1].samplerate != file_info[i].samplerate){ fprintf(stderr, "\nERROR, %s is not the same sample rate as the first file.  Terminating.\n\n", file_info[i].filename); 
		exit(1);}
	}
	file_info[0].samplerate = file_info[1].samplerate; /* IT IS ASSUMED SAMPLERATES MATCH! */
	

	/* Construct the output file */
	file_info[0].filename = outfilename;
	char *extension = strrchr(outfilename, '.');
	if(extension == NULL) file_info[0].extension = ""; /* Check if a file extension is included */
	else file_info[0].extension = extension + 1;
	file_info[0].bitdepth = bitdepth;
	file_info[0].numchannels = numchannels;
	file_info[0].numsamples = numsamples;
	file_info[0].seconds = seconds;

	if(numchannels < usechannel)
	{
		fprintf(stderr, "\nERROR! The channel you picked with -c does not exist in the output file.  This file only has %d channels.\n\n", numchannels);
		exit(1);
	} 
	

	/*Print user information about the new file */
	if(usechannel == -1) fprintf(stderr, "\nCreating new file: %s. It contains all channels.\n\n", file_info[0].filename);
	else fprintf(stderr, "\nCreating new file: %s. It contains only channel: %d\n\n", file_info[0].filename, usechannel);
	//printfileinfo(&file_info[0]);
	
	/* Now with numsamples, malloc required memory for the actual samples */
	if((file_info[0].samples = malloc(numsamples * numchannels * sizeof(int) + 10)) == NULL) fprintf(stderr, "MALLOC FAILED! In Sndchan.c\n");
	

	long placeholders[numfiles];
	for(i = 0; i < numfiles; i++) placeholders[i] = 0;  /* init placeholders */


	/* Going to cycle through files and build the channels, ONLY IF ALL CHANNELS. */
	/* m is the final sample array, j is the # of chans for a subfile, i is the file counter */
	for(m = 0; m < numsamples * numchannels; 1)
	{	
		for(i = 1; i < numfiles; i++)
		{
			for(j = 0; j < file_info[i].numchannels; j++)
			{
				if(file_info[i].samples[placeholders[i] + j] < file_info[i].numsamples * file_info[i].numchannels)
				{
					file_info[0].samples[m] = file_info[i].samples[j + placeholders[i]];
					//printf("%d, %d, %d\n", m, file_info[0].samples[m], file_info[i].samples[placeholders[i] + j]);
					m++;
				}
			}
			placeholders[i] = (placeholders[i] + j);
		}			
	}


	/* Now that the resulting file has been created, we need to check the -c switch and zero
		out any channels if that switch has been utilized */

	if(usechannel != -1)
	{
		for(i = 0; i < numsamples * numchannels; i++)
		{
			if(i % numchannels +1 == usechannel){}
			else
			{
				file_info[0].samples[i] = 0; /* Zero out the sound */
			}
		}

	}

	

	/* Finished channel combining, now rebuild */
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
	else
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


	/* Free all allocated memory */
	for(i = 1; i < numfiles; i++)
	{
		fclose(files[i]);
	}

	fprintf(stderr, "Success!  Combined all channels of all of the valid specified files into new file: %s\n\n", file_info[0].filename);

	return 0;
}

void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n\n");
	printf("\tIMPORTANT: To merge channels in sndchan, sound files MUST have the same samplerate.\n");
	printf("\tALSO IMPORTANT: ALL FILES PASSED AS ARGUMENTS MUST HAVE PROPER EXTENSIONS (i.e. .wav or .cs229)\n");
	printf("\n*****************************\n");
	printf("\tSupported Switches:");
	printf("-h (help), -w (output wav file instead of cs229), \n\t-o filename (output to specified filename, otherwise stdout)\n\t-c n (n is only output that channel).\n");
	printf("\n\tSupported Arguments: Inputfile1, Inputfile2...Inputfilen  Will concat file1 -> file2...->filen\n\tSwitches must come before arguments!\n\tIf no arguments are listed, we will read from standard input.\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n\n");
	printf("\tThis program will exit after writing the new sound file containing all channels of the given argument sound files combined into one.\n\n");
	printf("\tWARNING: If your output file only has 1 channel, most players will play that channel out of all channels. Also, if you use iTunes,\n\tit doesn't seem to want to play files with more than 2 channels.  Use another player.\n");
	printf("For more information please see the included README file.\n\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}