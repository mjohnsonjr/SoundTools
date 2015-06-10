#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "soundfunctions.h"



/*
	IMPORTANT:  You can only concatenate files if they have the same samplerate and same number of channels.
				You must also give filenames INCLUDING the extension.  (e.g. "filename.wav" cannot be passed as 
				"filename"  It must be passed as filename.wav or this program will not recognize it.)

	This program reads sound files passed as arguments, and writes them to a single sound file that is the 
	concatenation of the two.  If no files are passed as args then read from standard input.  If there is no
	-o switch argument, write to standard output.

	Switches: -h (show helpscreen), -o filename (output to specific filename, otherwise stdout), -w (output .wav instead of .cs229)
	Args: Inputfile1, Inputfile2... InputfileN
	
	Put switches first, arguments second

	This program only supports reading one file from standard input.  So, it really only converts from .cs229
	to .wav or vice-versa from standard input.  If you want to concat files, use the command line arguments.

	So we can use Stdout solely for filedata if we don't specify a -o switch, all user messages are output via 
	stderr to the console.

*/



void help();

int main(int argc, char** argv)
{
/* flags and vars */
char helpflag = 0;
char wavflag =0;
char outflag = 0;	/* write to stdout if zero, otherwise to file specified. */
char fileflag = 0; /* determines whether or not to read from stdin*/
FILE* fileout;


	/*5k for each */
	int numfiles = 1;
	FILE* files[argc]; /* max number of files */
	
	char *outfilename;
	char* inputfilenames[80];
	char command[100];
	inputfilenames[0] = malloc(4);
	outfilename = malloc(180);
	int i, j = 0; char *tok;
	long l, m = 0; /*l counts current samples, m counts total samples */
	tok = malloc(90);

	/* Decode all switches and set flags */
	for(i =1; i < argc; i++)
	{
		tok = *(argv + i);//used to get the first char of the string pointer
		
		if(strcmp(argv[i], "-h") == 0 && helpflag == 0)  helpflag = 1; //set help flag.
		
		else if(strcmp(argv[i], "-w") == 0 && wavflag == 0) wavflag = 1; //wav flag
		
		else if(strcmp(argv[i], "-o") == 0 && outflag == 0)
		{
			 outflag = 1; //output file flag
			 outfilename = argv[i+1];
			 i++;
		}
		else if(tok[0] != '-')
		{	
			fileflag = 1;
			if((files[numfiles]=fopen(argv[i], "r")) == NULL)
			{
				fprintf(stderr, "\nFailed to open \"%s\"...Skipping this file.\n", argv[i]);
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


/* Create my structs for each file, identifying what extension it is.*/
	fileinfo file_info[numfiles];


	/* Do not need safety checks on extensions, already done when opening */
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
				fprintf(stderr, "\nERROR, %s is an invalid file of type %s\n\n", file_info[i].filename, file_info[i].extension);
				exit(1);
			} 
		}
		else if(strcmp(extension, "cs229") == 0)
		{
			char err = fillCS229Info(&file_info[i], files[i]);
			if(err < 0)
			{
				fprintf(stderr, "\nERROR, %s is an invalid file of type %s\n\n", file_info[i].filename, file_info[i].extension);
				exit(1);
			} 
		}
	}

		
	/* All files are stored in structs, remember, file_info[0] is NULL until set, start loops at i = 1 */
	/* Ultimately, file_info[0] will become the output file. */
	/* Otherwise write to stdout.. */
	
	/* Check for differences and scale bitdepths, use higher one.. Channels.. fill extra with zero, use higher. */
	int bitdepth = 0, numchannels = 0; long numsamples = 0; double seconds = 0.0;
	for(i = 1; i < numfiles; i++) if(file_info[i].bitdepth > bitdepth) bitdepth = file_info[i].bitdepth;
	for(i = 1; i < numfiles; i++) if(file_info[i].numchannels > numchannels) numchannels = file_info[i].numchannels;
	for(i = 1; i < numfiles; i++) numsamples = numsamples + file_info[i].numsamples;
	for(i = 1; i < numfiles; i++) seconds = seconds + file_info[i].seconds;
	


	/* Sample rate check */
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
	
	/* Now with numsamples, malloc required memory for the actual samples */
	if((file_info[0].samples = malloc(numsamples * numchannels * sizeof(int) + 10)) == NULL) fprintf(stderr, "MALLOC FAILED! In Sndcat.c\n");
	

	/* This is alg for smashing the files together while preserving their channel numbers.
	Files with less channels than the final file will have the extra channels muted */
	int channelcounter; j = 0;
	for(i = 1; i < numfiles; i++)
	{
		//printfileinfo(&file_info[0]);
			for(l = 0; l < (file_info[i].numsamples * file_info[i].numchannels); l+=file_info[i].numchannels)
			{	
				for(channelcounter = 0; channelcounter < numchannels; channelcounter++)
				{
					if(file_info[i].numchannels > channelcounter) file_info[0].samples[m] = file_info[i].samples[l + channelcounter];
					else file_info[0].samples[m] = 0; 
					m++;
				}
			}
	}
	
	/* Finished concatenating, now write */
	if(wavflag == 1) /*output to wav format.*/
	{
		fileout = createWav(&file_info[0], outfilename);		
		  
		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info[0].filename);
			exit(1);	
		}

		/* Flush to stdout, destroy the temporary file we created. */
		if(outflag == 0)
		{
			fclose(fileout);
			FILE* tempfile = fopen(outfilename, "r");
			if(fileout == NULL)
			{
				fprintf(stderr, "\n%s On read-back, .wav file failed to open.\n\n", file_info[0].filename);
				exit(1);	
			}
			fseek(fileout, 1, SEEK_END);
			long size = ftell(fileout);
			//fprintf(stderr, "%d\n", size);
			rewind(fileout);

			BYTE buffer[size];

			fread(&buffer, size, 1, tempfile);
			fwrite(&buffer, size, 1, stdout);			
			remove(outfilename);
			fclose(tempfile);
			
		}
			
	}
	else /* Output to .cs229 format */
	{	
		fileout = createCS229(&file_info[0], outfilename);
		
		if(fileout == NULL)
		{
			fprintf(stderr, "\n%s is not a valid output file path or name.  File was unable to be created.  Terminating.\n\n", file_info[0].filename);
			exit(1);	
		}

		/* Flush to stdout, destroy the temporary file we created. */
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


	fprintf(stderr, "\nSuccess!  Concatenated all of the specified files into new file: %s\n\n", file_info[0].filename);

	return 0;

}

void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n\n");
	printf("\tIMPORTANT: To concatenate sound files they MUST have the same samplerate and number of channels.\n");
	printf("\tALSO IMPORTANT: ALL FILES PASSED AS ARGUMENTS MUST HAVE PROPER EXTENSIONS (i.e. .wav or .cs229)\n");
	printf("\n*****************************\n");
	printf("\tSupported Switches:");
	printf("-h (help), -w (wav file instead of cs229), -o filename (output to specified filename)\n");
	printf("\tSupported Arguments: Inputfile1, Inputfile2...Inputfilen  Will concat file1 -> file2...->filen\n\tSwitches must come before arguments!\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n\n");
	printf("\tThis program will exit after writing the concatenation of the given files to the new file.\n\n");
	printf("For more information please see the included README file.\n\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}