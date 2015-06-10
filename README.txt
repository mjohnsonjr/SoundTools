README FILE:  Created by Michael Johnson

Programs: Sndinfo, Sndcat, Sndchan, Sndmix, Sndgen, Sndplay

WARNING: Standard output seems to have a very small buffer that can overflow quickly with a lot of files (3+,like concatenating etc.). 
		 I would HIGHLY advise if doing a very large amount of files with any of these programs to use the "-o filename" switch.  It will work if that is used.

IMPORTANT: ALL COMMAND LINE ARGUMENTS THAT ARE FILES MUST HAVE THEIR EXTENSIONS (e.g. .cs229 or .wav).
			If you don't direct standard output to a file, prepare for your screen to attack you.  User
			messages are pushed to stderr.  If you really want to redirect those, go for it.  
			(To redirect stdout, use > or 1>.  For stderr, use 2> in the unix terminal).

Sndinfo:
	
	This program reads a sound file passed as an argument, and prints some information about the sound file
	including bit depth, number of channels, length in seconds, and more.  This program will print out 
	the information about the sound file in the order that they are passed in as inputs. Reading from standard
	input is supported (please see note below).

	Switches: -h (opens the help screen)

	Args: Inputfile1 Inputfile 2 ... Inputfile N 

	Note:  If you read from standard input.  You may either pipe in a file with paths to sound files, if you
	don't you can type them into the console one at a time.
	
	Example use:  ./sndinfo test.wav test2.wav test2chan.wav test.cs229 junk.cs229
	(Will read and print info about each file listed, in order).



Sndcat:

	IMPORTANT:  You can only concatenate files if they have the same samplerate. If they do not the user will be informed and
	the program will terminate.

	This program reads sound files passed as arguments, and writes them to a single sound file that is the 
	concatenation of them all.  If no files are passed as args then we read one file from standard input.  If there is no
	-o switch argument, we will write to standard output (use 1> or > on command line to forward standard output to a file
	otherwise your screen will be a mess).

	Switches: -h (show help screen), -o filename (output to specific filename, otherwise stdout), -w (output .wav instead of .cs229)
	Args: Inputfile1 Inputfile2 ... InputfileN

	This program only supports reading one file from standard input.  So, it really only converts from .cs229
	to .wav or vice-versa from standard input.  If you want to concat files, use the command line arguments.

	Note: So we can use Standard out solely for file data if we don't specify a -o switch, all user messages are output via 
	stderr to the console.  If you want to redirect standard error use 2> on the unix command line.


Sndchan:
	
	IMPORTANT: Files used in Sndmix MUST HAVE the same samplerate.  If they do not, the program will terminate and
	inform the user.

	This program reads sound files passed as arguments, and writes them to a single sound file that is the 
	concatenation of all the channels of the sound files.  If no files are passed as args then we read one from standard input, 
	if no -o switch is present, we will write to standard output.

	Note: Reading from stdin only reads one file, so it is only useful to strip channels off of a single file in this
	instance.  (i.e. Passing a 4 channel file in stdin and outputting it with only its second channel).

	Switches: -h (show help screen), -o filename (output to specified file), -w (output to .wav instead of .cs229),
	-c n (This will output a sound file with ONLY that channel, all other channels lost.)
	
	NOTE: IF -c switch is used, then the output file is created, but all channels EXCEPT n will be zeroed out.
			The spec is not clear on this, this is the way I interpreted it.  The output sound file only contains
			that channel specified in the -c switch.  But the file still reports the amount of channels that would
			exist without the switch.  So, if you play this on a multichannel setup, only the specific speaker will
			play the sound.  It will not play the one channel through all the speakers.  So if you have only 2 channels and your
			file only has sound from channel 3, you will not hear anything.  Please use an adequate speaker setup for this.

			Example: File has 2 channels as a result, with -c 1 switch.  The file will have 2 channels, but the 2nd one is 
			zeroed out.  So, only sound will come out of the left speaker (channel 1) and the right speaker will
			be silent.  This is how I interpreted it and it is intentional.

	Args: Inputfile1 Inputfile 2... Inputfile N. (Warning: too many files will make an output with a 
	ridiculous amount of channels... :D)

	WARNING: If your output file only has 1 channel, most players will play that channel out of all channels. 
	Also, if you use iTunes, it doesn't seem to want to play files with more than 2 channels.  Use another player
	like VLC or Quicktime.
	
	Since we use stdout if no -o switch is used, we print all user messages to stderr which goes to the console.  Stdout
	will go to a file of your choice using > on the command line, or the console otherwise.  Output will only go to stdout
	when the -o is NOT used.  



Sndmix:

	IMPORTANT: Files used in Sndmix MUST HAVE the same samplerate.  If they do not, the program will terminate and
	inform the user.

	This program reads sound files passed as arguments, and writes them to a single sound file that is a mix
	of the sound files.  If no files are passed as args then the program terminates.  This program does not 
	require to read from standard input according to the spec, and doing so is pointless. If no -o switch exists, write
	to standard output.  All user messages are written to standard error.

	Switches: -h (display helpscreen), -o filename (output to this file), -w (output to wav instead of .cs229)

	Args: Mult1 Inputfile1 Mult2 Inputfile 2 ... MultN Inputfile N.

	After multiplying each sound sample by the specified multiplier given on the command line, we sum all of these samples
	together.  If these samples go outside the specified bitdepth of the file, we will truncate it to the maximum. (i.e. if
	we have a file with 8bit depth, and a sample adds up to 400 after scaling, we will truncate it to 127.  That is the highest
	an 8 bit sample can be.)
	
	Example:  File1 has mult 10, samples 1, 2, 3
				File 2 has mult 2, samples 5, 2, 6.
				Fileout will have samples: 20, 24, 42.
				
	Note: Put switches first, arguments second.
	
	Warning:  Sndmix will make sound file sounds incredbibly obnoxious unless they are carefully mixed.


Sndgen:

	This program takes in a wide variety of switches as command line arguments and uses them to generate a simple ADSR sound
	wave based on the switches you choose (see switches below).  This program does not read from standard input. If no -o switch exists, we write to standard output.  All user messages are written to standard error.  You can pipe standard output data to 
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

	This produces a simple sound sample that is t seconds long.  With attack, decay, sustain, and release phases each of their specified
	length.  If the file is not long enough to support those time periods for each segment, they are to be shrank as follows:

		-First we shrink the sustain phase
		-Second shrink the decay phase
		-Third shrink the attack phase
		-If the time t is less than the release time, no sound is played.
	
	Example:  ./sndgen -f 1000 -t 10 -v 0.5 --bits 16 -a 1 -d 3 -s 0.6 -r 2 --sr 22050 --sawtooth -w -o gen.wav

		The above produces a 1000Hz sound file that is 10 seconds long, it has an attack phase of 1 sec, decay phase of 3 sec, a sustain volume that is 0.6 times as strong as the peak attack volume, and a release phase time of 2 seconds.  The output wave is a sawtooth wave that has a samplerate of 22050 samples per second with 16 bit depth, and the output file is called "gen.wav" and we are outputting to .wav format.

	WARNING!:	Switches can be in any order, but you must have all required switches there or the program will consider your lack of a switch to be an error for that specific value. Make sure you use all switches you desire.
				
	Note:  It is advised not to use the maximum volume (-v 1) because it sounds extremely harsh.  Using something right in the middle
	(-v 0.5) is much more pleasant to the ears.  ALSO, using 8 bit depth doesn't sound very good compared to the others. 

Sndplay:

	NOTES:  The spec does not specify or require input from standardinput, so support is omitted here.  Standard output support
	still exists.  You may redirect stdout to a file with "> filename" on the unix command line.  

	Sndplay takes a properly formatted (one is included in the tarball) .abc229 file in via the command line (only one at a time), and creates a sound file that plays
	the listed instruments in the file. Please ensure the abc229 is properly formatted.  If information is missing an error will be 
	thrown.  Make sure all data fields are listed. 

	Note: If an instrument is muted, and it also plays the longest of each instrument.  The audio file will have a lot of silence at 
	the end of it.  If you unmute the instrument that is longest, it will play as normal.  This is because muted instruments are just zeroed out, but the calculated file length is still the same.

	Supported switches: --sr n (sample rate of the file to be created), --bits n (bitdepth of the file to be created),
						-o filename (the filename the output will be directed to), --mute n (mutes the specified instrument number),
						-h (display the help screen), -w (output to wav file instead of cs229).

	Supported Arguments: ONE .abc229 file.					

						Note: Instruments are numbered from 0 - n.  Also, there are no error checks on --mute.  If you mute an instrument that does not exist, nothing happens.

						ALSO: THE SPEC did NOT say we needed to read from stdin, so this program will only support reading .abc229 files
						from the command line arguments.  If multiple .abc229 files are listed an error is thrown.  Please only list one.



