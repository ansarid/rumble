/*
*
* libjoyrumble.c
*
* version 0.2.2
*
* by Cleber de Mattos Casali <clebercasali@yahoo.com.br>
*
*
* This program is distributed under the terms of the GNU LGPL license:
*
* http://www.gnu.org/copyleft/lesser.html
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* The purpose of this program is to provide game / emulator programmers
* an extremely simple way to access the joystick "rumble" feature.
*
* Only simple joystick / joypad rumble effect is suported. There are no
* plans to support any advanced force feedback effects.
*
* As of now it's only for Linux. There are plans for a Windows version
* in the future.
*
* This program is loosely inspired by "rumble.c" written by Stephen M.Cameron,
* wich is a modified version of the "fftest.c" program from the "joystick"
* package, written by Johan Deneux.
*
* You are free to use the library in your free or commercial games.
* Please give me some credit if you do.
*
*/

/*
*
* To create the shared library (libjoyrumble.so), compile it with:
*
* gcc -c -fPIC libjoyrumble.c -o libjoyrumble.o
* gcc -shared -Wl,-soname,libjoyrumble.so -o libjoyrumble.so  libjoyrumble.o
*
*
* To create the command-line tool (joyrumble), compile it with:
*
* gcc libjoyrumble.c -o joyrumble
*
* If you're running on x86_64 (64 bits), and want to compile the library for x86 (32 bits), add the "-m32" flag to gcc.
*
*/

/* Uncomment this if you want to add debug messages. */

#define DEBUG 1


/*  Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>

/* test_bit  : Courtesy of Johan Deneux */
#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)


/* 8 joypads should be enough for everyone */
#define MAXJOY 8


/* Shared variables */
static unsigned long features[4];
static struct ff_effect effects[MAXJOY];
static struct input_event stop[MAXJOY];
static struct input_event play[MAXJOY];
static char *eventfile[MAXJOY];
static int event_fd[MAXJOY];
static int hasrumble[MAXJOY];
static int int_strong[MAXJOY];
static int int_weak[MAXJOY];
static int int_duration[MAXJOY];
static int initialized=0;
static int lastjoy=-1;


/* Check if a file exists */
int file_exists(char *filename)
{
	FILE *i;
  i = fopen(filename, "r");

if (i == NULL)
  {
    return 0;
  }

  fclose(i);
  return 1;

}


/* Check if a directory exists */
int dir_exists(char *filename)
{
	DIR *i ;
	i =opendir(filename);

	if (i == NULL)
	{
	return 0;
	}

	closedir(i);
	return 1;
}

/* re-open event file to try to recover from errors */
int joy_reopen(int joy)
{
	#ifdef DEBUG
		//printf("Trying to recover from error: closing event file\n");
	#endif
	
	close(event_fd[joy]);
	
	#ifdef DEBUG
		//printf("Trying to recover from error: re-opening event file\n");
	#endif
	
	event_fd[joy] = open(eventfile[joy], O_RDWR);
	
}



/* Initialization. This should run automatically. */
void __attribute__ ((constructor)) joyrumble_init(void)
{

	#ifdef DEBUG
		//printf("libjoyrumble: INIT\n");
	#endif

	/* event_fd[count] is the handle of the event file */

	int count=0, j=0;

	/* this will hold the path of files while we do some checks */
	char tmp[128];

	for (count = 0; count < MAXJOY; count++)
	{

		sprintf(tmp,"/dev/input/js%d",count);
		/* Check if joystick device exists */
		if (file_exists(tmp)==1){
			lastjoy=count;
			for (j = 0; j <= 99; j++)
			{
			/* Try to discover the corresponding event number */
			sprintf(tmp,"/sys/class/input/js%d/device/event%d",count,j);
			if (dir_exists(tmp)==1){
				/* Store the full path of the event file in the eventfile[] array */
				sprintf(tmp,"/dev/input/event%d",j);
				eventfile[count]=(char *)calloc(sizeof(tmp),sizeof(char));
				sprintf(eventfile[count],"%s",tmp);
				#ifdef DEBUG
					//printf(eventfile[count],"%s");
					//printf("\n");
				#endif
				}
			}

		}

	}

	#ifdef DEBUG
		//printf("lastjoy=%d\n",lastjoy);
	#endif

	for (count = 0; count <= lastjoy ; count++)
	{

	hasrumble[count]=0;

	/* Prepare the effects */
	effects[count].type = FF_RUMBLE;
	effects[count].u.rumble.strong_magnitude = 65535;
	effects[count].u.rumble.weak_magnitude = 65535;
	effects[count].replay.length = 1000;
	effects[count].replay.delay = 0;
	effects[count].id = -1;

	/* Prepare the stop event */
	stop[count].type = EV_FF;
	stop[count].code = effects[count].id;
	stop[count].value = 0;

	/* Prepare the play event */
	play[count].type = EV_FF;
	play[count].code = effects[count].id;
	play[count].value = 1;


	/* Open event file to verify if the device and the drivers support rumble */
	event_fd[count] = open(eventfile[count], O_RDWR);


	if (event_fd[count] < 0){

		/* Can't acess the event file */
		hasrumble[count]=0;
	}
	else{

		if (ioctl(event_fd[count], EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1){
			/* This device can't rumble, or the drivers don't support it */
			hasrumble[count]=0;

		}
		else{

			if (test_bit(FF_RUMBLE, features))
				/* Success! This device can rumble! */
				hasrumble[count]=1;
		}

	#ifdef DEBUG
		//printf(eventfile[count],"%s");

		if (hasrumble[count]){
			//printf(" can rumble.\n");
		}
		else{
			//printf(" can't rumble.\n");
		}
	#endif

	}
	}

	/* Initialization is complete */
	initialized=1;
}



/* joyrumble ( joystick number (1-8), strong motor intensity (0-100), weak motor intensity (0-100), duration in miliseconds ) */
/* This is the only function that needs to be visible externally. */
/* Notice the joystick number starts from 1. */
/* 1=/dev/input/js0 , 2=/dev/input/js1 , and so on. */
extern int joyrumble(int joynumber, int strong, int weak, int duration)
{

	int joy=joynumber-1;

	/* Copy the arguments to the shared variables. */
	int_strong[joy]=strong;
	int_weak[joy]=weak;
	int_duration[joy]=duration;

	#ifdef DEBUG
		//printf("init=%d\n",initialized);
	#endif

	/* Check if the program is initialized */
	if (initialized==0) {
		#ifdef DEBUG
			//printf("go init!\n");
		#endif
		joyrumble_init();
	}


	/* If this device doesn't support rumble, just quit */
	if (hasrumble[joy]==0){
		#ifdef DEBUG
			//printf("NO RUMBLE");
		#endif
		return -1;
	}

	#ifdef DEBUG
		//printf("libjoyrumble: will rumble now\n");
	#endif

	#ifdef DEBUG
		//printf(eventfile[joy],"%s");
		//printf("\n");
		//printf("Joy#:%d\n",joy);
		//printf("Strong:%d\n",int_strong[joy]));
		//printf("Weak:%d\n",int_weak[joy]));
		//printf("Duration:%d\n",int_duration[joy]));
	#endif

	/* Stop the effect if it's playing */
	//stop[joy].code =  effects[joy].id;

	//if (write(event_fd[joy], (const void*) &stop[joy], sizeof(stop[joy])) == -1){
	//#ifdef DEBUG
//		//printf("error stopping effect\n");
	//#endif
	//}


	/* Modify effect data to create a new effect */
	effects[joy].type = FF_RUMBLE;
	effects[joy].u.rumble.strong_magnitude = int_strong[joy]*65535/100;
	effects[joy].u.rumble.weak_magnitude = int_weak[joy]*65535/100;
	effects[joy].replay.length = int_duration[joy];
	effects[joy].replay.delay = 0;
	effects[joy].id = -1;	/* ID must be set to -1 for every new effect */

	/* Effect intesity limits. */
	if (effects[joy].u.rumble.strong_magnitude < 0)
		effects[joy].u.rumble.strong_magnitude=0;

	if (effects[joy].u.rumble.strong_magnitude > 65535)
		effects[joy].u.rumble.strong_magnitude=65535;

	if (effects[joy].u.rumble.weak_magnitude < 0)
		effects[joy].u.rumble.weak_magnitude=0;

	if (effects[joy].u.rumble.weak_magnitude > 65535)
		effects[joy].u.rumble.weak_magnitude=65535;


	/* Send the effect to the driver */
	if (ioctl(event_fd[joy], EVIOCSFF, &effects[joy]) == -1) {
		#ifdef DEBUG
			//printf("error uploading effect\n");
//			fprintf(stderr, "%s: failed to upload effect: %s\n",
//				eventfile[joy], strerror(errno));
		#endif
		joy_reopen(joy);
	}

	/* Play the effect */
	play[joy].code = effects[joy].id;

	if (write(event_fd[joy], (const void*) &play[joy], sizeof(play[joy])) == -1){
		#ifdef DEBUG
			//printf("error playing effect\n");
		#endif
		joy_reopen(joy);
	}


	#ifdef DEBUG
		//printf("Done.\n");
	#endif



}


/* The "joyrumble" command-line tool */
int main(int argc, char *argv[]){

  if (argc < 5)
    {
	  printf("JOYRUMBLE\nUsage: joyrumble [ joystick (1-8) ]  [strong motor magnitude (%%) ]  [weak motor magnitude (%%) ]  [duration (miliseconds) ]\n");
	return 0;
    }


    int arg_joy=atoi(argv[1]);
    short arg_strong=atoi(argv[2]);
    short arg_weak=atoi(argv[3]);
    int arg_time=atoi(argv[4]);

	if (arg_joy < 1)
		arg_joy=1;

	if (arg_joy > MAXJOY)
		arg_joy=MAXJOY;

	if (arg_time < 1)
		arg_time=1;

	if (arg_weak < 1)
		arg_weak=1;

	if (arg_weak > 100)
		arg_weak=100;

	if (arg_strong < 1)
		arg_strong=1;

	if (arg_strong > 100)
		arg_strong=100;

		#ifdef DEBUG
			//printf("will rumble now...\n");
		#endif

	joyrumble ( arg_joy, arg_strong, arg_weak, arg_time);

		#ifdef DEBUG
			//printf("will sleep now...\n");
		#endif

	usleep((arg_time+100)*1000);

		#ifdef DEBUG
			//printf("finished sleeping\n");
		#endif


	return 0;

}


/* Close all files. */
void __attribute__ ((destructor)) joyrumble_end(void)
{

	int count=0;

	for (count = 0; count <= lastjoy ; count++)
	{

		/* Close the event file */
		if (event_fd[count]>0){
			close(event_fd[count]);
			#ifdef DEBUG
				//printf("libjoyrumble: File closed.\n");
			#endif
		}

	}

	#ifdef DEBUG
		//printf("libjoyrumble: All done. Leaving.\n");
	#endif

}
