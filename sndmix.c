#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "soundfunctions.h"



/*
	IMPORTANT:  USE PROPER FILE EXTENSIONS (e.g. .wav or .cs229).
				(e.g. "filename.wav" cannot be passed as "filename"  It must be passed as filename.wav 
				or this program will not recognize it.)

	This program reads sound files passed as arguments, and writes them to a single sound file that is a mix
	of the sounds files.  If no files are passed as args then the program terminates.  This program does not 
	require to read from standard input according to the spec, and doing so is pointless. If no -o switch exists, write
	to standard output.

	Switches: -h (display helpscreen), -o filename (output to this file), -w (output to wav instead of .cs229)

	Args: Mult1, Inputfile1, Mult2, Inputfile 2... MultN, Inputfile N.

	After multiplying each sound sample by the specified multiplier given on the command line, we sum all of these samples
	together.  If these samples go outside the specified bitdepth of the file, we will truncate it to the maximum. (i.e. if
	we have a file with 8bit depth, and a sample adds up to 400 after scaling, we will truncate it to 127.  That is the highest
	an 8 bit sample can bee.)
	
	Put switches first, arguments second.
*/



void help();

int main(int argc, char** argv)
{

/* flags and vars */
char helpflag = 0;
char wavflag =0;
char outflag = 0;	/*write to stdout or to this file if flag is up */
char fileflag = 0; /* determines if this is file1 or file2*/


FILE* fileout;

	int numfiles = 1;
	FILE* files[argc]; /* max number of files, obviously slightly more than needed. */
	
	char *outfilename;
	char* inputfilenames[80], command[100];
	inputfilenames[0] = malloc(4);
	outfilename = malloc(90); /* If you need more than 90 spots for fname... */
	int i, j = 0; char *tok;
	long m = 0; /*m counts total samples */
	tok = malloc(90);


	float multipliers[argc]; /* Snd mix scaling */
	
	/* Decode Switches */
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

		else /* A multiplier and a file */
		{	
			errno = 0;
			multipliers[numfiles] = strtod(argv[i], NULL);

			if(errno != 0 || (multipliers[numfiles] < -10 || multipliers[numfiles] > 10))
			{
				fprintf(stderr, "\n\nERROR! Invalid multiplier or switch at \"%s\"...Skipping this file. Please check the help screen with -h.\n", argv[i]);
				 i++;
				 continue;
			}
			i++;

			fileflag = 1;
			if((files[numfiles]=fopen(argv[i], "r")) == NULL)
			{
				fprintf(stderr, "\n\nFailed to open \"%s\"...Skipping this file.\nRemember that each file needs an extension [e.g. .wav or .cs229]\n", argv[i]);
				continue;
			}
			inputfilenames[numfiles] = malloc(strlen(argv[i]) +1);	
			inputfilenames[numfiles] = argv[i];
			numfiles++;
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

	
	/* Create my structs for each file, identifying what extension it is.*/
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
			if(err < 0) fprintf(stderr, "ERROR, %s is an invalid file of type %s\n", file_info[i].filename, file_info[i].extension);
		}
		else if(strcmp(extension, "cs229") == 0)
		{
			char err = fillCS229Info(&file_info[i], files[i]);
			if(err < 0) fprintf(stderr, "ERROR, %s is an invalid file of type %s\n", file_info[i].filename, file_info[i].extension);
		}
	}


	

	
	/* Check for differences and scale bitdepths, use higher one.. Channels.. fill extra with zero, use higher. */
	int bitdepth = 0, numchannels = 0; long numsamples = 0; double seconds = 0.0;
	for(i = 1; i < numfiles; i++) if(file_info[i].bitdepth > bitdepth) bitdepth = file_info[i].bitdepth;
	

	/* Find highest # of channels for the new file */
	for(i = 1; i < numfiles; i++) if(file_info[i].numchannels > numchannels) numchannels = file_info[i].numchannels;
	

	/* Number of bytes in the file will be smaller than this is we cut channels, but we're still mallocing all */
	for(i = 1; i < numfiles; i++) if(file_info[i].numsamples > numsamples) numsamples = file_info[i].numsamples;
	

	/* Determine the time (seconds) for the new file */
	for(i = 1; i < numfiles; i++) if(file_info[i].seconds > seconds)seconds = file_info[i].seconds;
	

	/* Sample rate safety check */
	for(i = 1; i < numfiles; i++) 
	{
		if(file_info[1].samplerate != file_info[i].samplerate){ fprintf(stderr, "\nERROR, %s is not the same sample rate as the first file.  Terminating.\n\n", file_info[i].filename); 
		exit(1);}
	}
	file_info[0].samplerate = file_info[1].samplerate; /* IT IS ASSUMED SAMPLERATES MATCH! */
	

	/* Construct output file */
	file_info[0].filename = outfilename;
	char *extension = strrchr(outfilename, '.');
	if(extension == NULL) file_info[0].extension = "";
	else file_info[0].extension = extension + 1;
	file_info[0].bitdepth = bitdepth;
	file_info[0].numchannels = numchannels;
	file_info[0].numsamples = numsamples;
	file_info[0].seconds = seconds; 
	

	/*Print user information about the new file */
	//printfileinfo(&file_info[0]);
	
	/* Now with numsamples, malloc required memory for the actual samples */
	if((file_info[0].samples = malloc(numsamples * numchannels * sizeof(int) + 10)) == NULL) fprintf(stderr, "MALLOC FAILED! In Sndmix.c\n");
	


	//for(i = 0; i < numfiles; i++) printf("%d\n", multipliers[i]);

	long test = 0;
	/* Scale Data and/or truncate before.  Must also check after. */
	for(i = 1; i < numfiles; i++)
	{
		for(j = 0; j < file_info[i].numsamples * file_info[i].numchannels; j++)
		{
			if(file_info[i].bitdepth == 32)
			{
				test = (long)(file_info[i].samples[j] * multipliers[i]);
				test = test >> 32;

				if(test == 0) file_info[i].samples[j] = file_info[i].samples[j] * multipliers[i]; /* No overflow */
				else if(test < 0 && test != -1) file_info[i].samples[j] = -2147483647;
				else if(test > 0) file_info[i].samples[j] = 2147483647;
				
			}
			else
			{
				file_info[i].samples[j] = file_info[i].samples[j] * multipliers[i];
				if(file_info[i].samples[j] > 127 && file_info[i].bitdepth == 8) file_info[i].samples[j] = 127;
				else if(file_info[i].samples[j] < -127 && file_info[i].bitdepth == 8) file_info[i].samples[j] = -127;
				else if(file_info[i].samples[j] > 32767 && file_info[i].bitdepth == 16) file_info[i].samples[j] = 32767;
				else if(file_info[i].samples[j] < -32767 && file_info[i].bitdepth == 16) file_info[i].samples[j] = -32767;
				
			}
		}
	}


	/*Need to fille output file with zeros before adding */
	for(i = 0; i < numsamples * numchannels; i++) file_info[0].samples[i] = 0;

	/* Going to cycle through files and scale the samples.  Truncate samples that are out of bounds. */
	/* m is the final sample array, channelcount is the # of chans for a subfile, i is the file counter */

	int channelcount = 0;
	long placeholders[numfiles]; for(i = 0; i < numfiles; i++) placeholders[i] = 0;

	for(m = 0; m < numsamples * numchannels; 1) /* Basically a while loop */
	{
		for(i = 1; i < numfiles; i++)
		{
			if(channelcount < file_info[i].numchannels)
			{
				test = (((long)file_info[0].samples[m]) + ((long)file_info[i].samples[placeholders[i]]));
				test = (test >> 32);
					
				if(test < 0 && bitdepth == 32 && test != -1)
				{
					file_info[0].samples[m] = -2147483647;
				} 
				else if (test > 0 && bitdepth == 32)
				{
					file_info[0].samples[m] = 2147483647;
				
				}
				else
				{
					file_info[0].samples[m] = file_info[0].samples[m] + file_info[i].samples[placeholders[i]];	
				}
				placeholders[i]++;
			} 
			else {}	
		}
		m++;
		channelcount++;
		if(channelcount >= numchannels) channelcount = 0;
	}

	/* if(file_info[0].samples[m] >= (2 << (bitdepth -2))) file_info[0].samples[m] = (2 << (bitdepth -2) -1);
	 else if(file_info[0].samples[m] <= -(2 << (bitdepth -2))) file_info[0].samples[m] = -(2 << (bitdepth -2) +1);*/

	/* Check to ensure proper truncation of samples again after summing */
	for(i = 0; i < numsamples * numchannels; i++)
	{
		if(file_info[0].bitdepth == 32)
		{
			test = (long) file_info[0].samples[i];
			test = test >> 32;

			if(test == 0) ; /* No overflow */
			else if(test < 0 && test != -1) file_info[0].samples[i] = -2147483647;
			else if(test > 0) file_info[0].samples[i] = 2147483647;
			continue;
		}

		if(file_info[0].samples[i] > 127 && file_info[0].bitdepth == 8) file_info[0].samples[i] = 127;
		else if(file_info[0].samples[i] < -127 && file_info[0].bitdepth == 8) file_info[0].samples[i] = -127;
		else if(file_info[0].samples[i] > 32767 && file_info[0].bitdepth == 16) file_info[0].samples[i] = 32767;
		else if(file_info[0].samples[i] < -32767 && file_info[0].bitdepth == 16) file_info[0].samples[i] = -32767;
		
	}
	

	/* Finished mixing combining, now rebuild the output file*/
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
		
		}

	}


	/* Free allocated memory/files */
	for(i = 1; i < numfiles; i++)
	{
		fclose(files[i]);
	}

	fprintf(stderr, "\nSuccess!  Mixed all valid sound files with the specified multipliers into new file: %s\n\n", file_info[0].filename);

	return 0;
}

void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n\n");
	printf("\tIMPORTANT: To mix files in sndmix, sound files MUST have the same samplerate.\n");
	printf("\tALSO IMPORTANT: ALL FILES PASSED AS ARGUMENTS MUST HAVE PROPER EXTENSIONS (i.e. .wav or .cs229)\n");
	printf("\n*****************************\n");
	printf("\tSupported Switches:");
	printf("-h (help), -w (output wav file instead of cs229), \n\t-o filename (output to specified filename, otherwise stdout)");
	printf("\n\tSupported Arguments: mult1, Inputfile1, mult2, Inputfile2... multn, Inputfilen  \n\tWill mix file1 -> file2...->filen with the specified multpliers\n\tSwitches must come before arguments!\n");
	printf("\tMult must be a value specified between -10 and 10, and Inputfile must be a valid file path.\n");
	printf("\tOnce all files are scaled with their given value (-10 to 10) they will be summed into a new sound file.\n\n");
	printf("\tIMPORTANT NOTE: If you chose a value of 0 to scale a file, it is basically muted since all samples are multiplied by zero.\n\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n\n");
	printf("\tThis program will exit after writing the new sound file containing all argument files mixed into one.\n\n");
	printf("For more information please see the included README file.\n\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}