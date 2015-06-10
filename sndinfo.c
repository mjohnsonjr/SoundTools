#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "soundfunctions.h"

/*#define DEBUG*/

/*
	IMPORTANT: ALL COMMAND LINE ARGUMENTS THAT ARE FILES MUST HAVE EXTENSION (e.g. .cs229 or .wav).
	
	This program reads a sound file passed as an argument, and prints some info about the sound file
	including bit depth, number of channels, length in seconds, and more.  This program will print out 
	the information about the sound file in the order that they are passed in as inputs.

	Switches: -h (help screen)

	Args: Inputfile1, Inputfile 2,..., Inputfile N 

	Note:  If you read from standard input.  You may either pipe in a file with paths to sound files, if you
	don't you can type them into the console one at a time.


*/


void help();


int main(int argc, char** argv)
{
	char cs229flag = 0; /*file is cs229 */
	char wavflag = 0; /* file is wav */
	char nofileflag = 0; 

	FILE* files[argc]; /* number of files */
	
	int i;
	
	char* tok; char console[100];
	tok = malloc(50);

	
	if(argc == 1) nofileflag = 1;

	
	for(i =1; i < argc; i++)
	{
		/* Check if read from stdin */
		if(nofileflag == 1) break;

		tok = *(argv + i);/*used to get the first char of the string pointer*/
		
		/*Struct definition that contains all sound information (can be converted to .cs229 or .wav */
		fileinfo file_info;
		/*------------------*/
		
		if(strcmp(argv[i], "-h") == 0)  help(); /*see help.*/
		
		if((files[i] = fopen(argv[i], "r")) == NULL)
			{
				fprintf(stderr, "File %d: \n\nFailed to open \"%s\"...Continuing to the next file.\n(Remember that files must include their extenions. [e.g. .wav or .cs229]).\n\n", i, argv[i]);
				continue;
			}	
			
			
		/* read/print info from this file */
		file_info.filename = argv[i];
		
		/* reading the filename/ext */
		char *extension = strrchr(argv[i], '.');
		if(extension == NULL) file_info.extension = "";
		else file_info.extension = extension +1;
		
		if(strcasecmp(file_info.extension, "cs229") == 0) cs229flag = 1;
		
		else if(strcasecmp(file_info.extension, "wav") == 0) wavflag = 1;
		
		
		/* Read data dependent filetype: */
		if(wavflag == 0 && cs229flag == 0)
		{
			fprintf(stderr, "File %d: \n\nError, filetypes must be either .wav or .cs229 extension, continuing to the next file.\n\n", i);
			fclose(files[i]);
			continue;
		}
		
		/* read wav specific data */
		else if(wavflag == 1)
		{
			int err = fillWavInfo(&file_info, files[i]);
			if(err == -1)
			{ 
				fprintf(stderr, "File %d: \n\nError, this file is an improper or corrupt .wav, continuing to the next file.\n\n", i);
				continue;
			}
		}
		/*read cs229 specific data */
		else if(cs229flag == 1)
		{
			 int err = fillCS229Info(&file_info, files[i]);
			if(err == -1)
			{ 
				fprintf(stderr, "File %d: \n\nError, this file is a corrupt or improper .cs229, continuing to the next file.\n\n", i);
				continue;
			}
		}
		
		printf("File %d: \n\n", i);
		printfileinfo(&file_info);
		/* after reading info, close the file, reset flags. */
		wavflag = cs229flag = 0;
		fclose(files[i]);		
		
	}	
		printf("Finished reading all valid files.\n\n");



/************************************************************************************************************************/
		/* here we read from stdin */




	/* We only get here and execute this if reading from standard input, otherwise this is skipped. */
	if(nofileflag == 1)
	{	
		fileinfo file_info;
		printf("Please continuously type file paths (.wav or .cs229 file) in the console, or pipe data in.\nTo end this push Control + C:\n");
		while(fgets(console, 160, stdin) != NULL)
		{

			console[strlen(console) -1] = 0; /* Cut off new line character */


		/*Open the file, error check */
		if((files[i] = fopen(console, "r")) == NULL)
				{
					fprintf(stderr, "File %d: \n\nFailed to open \"%s\"...Continuing to the next file.\n(Remember that files must include their extensions. [e.g. .wav or .cs229]).\n\n", i, console);
					exit(1);
				}	
		

		/* read/print info from this file */
		file_info.filename = console;
		
		/* reading the filename/ext */
		char *extension = strrchr(console, '.');
		if(extension == NULL) file_info.extension = "";
		else file_info.extension = extension +1;
		
		if(strcasecmp(file_info.extension, "cs229") == 0) cs229flag = 1;
		
		else if(strcasecmp(file_info.extension, "wav") == 0) wavflag = 1;
		
		
		/* Read data dependent filetype: */
		if(wavflag == 0 && cs229flag == 0)
		{
			fprintf(stderr, "File %d: \n\nError, filetypes must be either .wav or .cs229 extension, continuing to the next file.\n\n", i);
			fclose(files[i]);
			continue;
		}
		
		/* read wav specific data */
		else if(wavflag == 1)
		{
			char err = fillWavInfo(&file_info, files[i]);
			if(err == -1)
			{ 
				fprintf(stderr, "File %d: \n\nError, this file is an improper or corrupt .wav, continuing to the next file.\n\n", i);
				continue;
			}
		}
		/*read cs229 specific data */
		else if(cs229flag == 1)
		{
			 char err = fillCS229Info(&file_info, files[i]);
			if(err == -1)
			{ 
				fprintf(stderr, "File %d: \n\nError, this file is a corrupt or improper .cs229, continuing to the next file.\n\n", i);
				continue;
			}
		}
		printfileinfo(&file_info);
		fclose(files[i]);
		wavflag = cs229flag = 0;
	}
		printf("Finished reading from standard input.\n");

	}

	return 0;
}




void help()
{
	printf("\n*****************************\n");
	printf("Help Screen:\n\n");
	printf("\tSupported Switches:");
	printf("-h (help) \n");
	printf("\tSupported Arguments: Inputfile1, Inputfile2...  Will give info of all files passed as arguments.\n");
	printf("\tEnsure all files are in the same directory if using relative path, otherwise use absolute.\n\n");
	printf("\tThis program will exit after printing the info of the given files (num channels, bit depth, etc.)\n\n");
	printf("For more information please see the included README file.\n\nWritten by Michael Johnson.\n\n");
	printf("\n*****************************\n\n");
	
	exit(1);

}