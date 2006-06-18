
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct globalArgs_t {
	int triggertype;			// 0 = none, 1 = edge, 2 = pattern	
	int recordtype;				// 0=intern,1=online, 2=snapshot	
	char *filename;
	FILE *file;
	int channel;			/* channelnumber */
	long number;			/* number of samples */
	char *samplerate;		/* # of input files */
	int triggervalue;		/* # of input files */
	int append; 			// append data to given file
	int verbose; 			// show all you can 
} globalArgs;

char * progname;

static const char *optString = "f:R:T:c:t:s:n:vaq?h";

/* Display program usage, and exit.
 */

void display_usage( void )
{
fprintf(stderr,
"Usage: %s [options]\n"
"Options:\n"
"  -f <vcd-file>			Specify location of data file.\n\n"
"  -R <record-type>		Specifiy the record type (online,internal or snapshot).\n"
"  -T <trigger-type>		Activate Trigger.\n"
"  -c <channel>		      	Specify channel for edge trigger (1-8).\n"
"  -t <trigger-value>	      	0 or 1 for edgetrigger.\n"
"			      	and the port state in hex at pattern-trigger.\n"
"  -s <samplerate>            	Specify samplerate 5us|10us|100us|1ms|10ms|100ms\n"
"  -n <number>                	Number of values to sample.\n"
"  -a                         	Add value to given file. Do not override given file.\n"
"  -v                         	Verbose output. -v -v for more.\n"
"  -q                         	Quell progress output. -q -q for less.\n"
"  -?                         	Display this usage.\n"
"\nlogic2vcd project: <URL:http://www.ixbat.de/logic2vcd>\n"
,progname);

    exit( EXIT_FAILURE );
}

/* Convert the input files to HTML, governed by globalArgs.
 */
void logic2vcd( void )
{
	// check trigger and switch on



	// check record typ 
	    // online - start, catch into memory, stop 
	    // intern - start, catch values
	    // snapshot - get one value
	
	// write to file
	  // create file header
	
	// write data to file


	printf(" Record: %i\n",globalArgs.recordtype);
	printf(" Trigger: %i\n",globalArgs.triggertype);
	printf(" Samplerrate: %s\n",globalArgs.samplerate);
	printf(" Channel: %i\n",globalArgs.channel);
	printf(" Numbers: %i\n",globalArgs.number);
	printf(" TValue: %x\n",globalArgs.triggervalue);
	printf(" Append: %i\n",globalArgs.append);
	printf(" File: %s\n",globalArgs.filename);

}

int main( int argc, char *argv[] )
{
	int opt = 0;
	progname = rindex(argv[0],'/');
	if (progname)
	  progname++;
	 else
	  progname = argv[0];

	/* Initialize globalArgs before we get to work. */
	globalArgs.triggertype = 0;	
	globalArgs.recordtype = 0;	
	globalArgs.filename = NULL;
	globalArgs.file = NULL;
	globalArgs.channel = 1;
	globalArgs.number = 1000;	
	globalArgs.samplerate = NULL;
	globalArgs.triggervalue = 0x00;	
	globalArgs.append = 0;
	globalArgs.verbose = 0;

	/* Process the arguments with getopt(), then 
	 * populate globalArgs. 
	 */

	opt = getopt( argc, argv, optString );
	while( opt != -1 ) {
		switch( opt ) {
			case 'f':
				// filename
				globalArgs.filename = optarg;	/* true */
				break;
				
			case 'R':
				printf("Recordtype %s\n",optarg);
				//globalArgs.langCode = optarg;
				if(strcmp( optarg, "intern" ) == 0)
				  globalArgs.recordtype = 0;
				else if (strcmp( optarg, "online")== 0)
				  globalArgs.recordtype = 1;
				else if (strcmp( optarg, "snapshot")== 0)
				  globalArgs.recordtype = 2;
				break;
				
			case 'T':
				if(strcmp( optarg, "edge" ) == 0)
				  globalArgs.triggertype = 1;
				else if (strcmp( optarg, "pattern")== 0)
				  globalArgs.triggertype = 2;
				break;
			case 'c':
				printf("channel %s\n",optarg);
				break;
			case 't':
				globalArgs.triggervalue = atoi(optarg);
				break;
			case 's':
				globalArgs.samplerate=optarg;	
				break;
			case 'n':
				globalArgs.number = atoi(optarg);	
				break;
			case 'a':
				globalArgs.append=1;
				break;
			case 'v':
				globalArgs.verbose=1;
				break;
	
			case 'h':	/* fall-through is intentional */
			case '?':
				display_usage();
				break;
				
			default:
				/* You won't actually get here. */
				break;
		}
		
		opt = getopt( argc, argv, optString );
	}
	
//	globalArgs.inputFiles = argv + optind;
//	globalArgs.numInputFiles = argc - optind;

	logic2vcd();
	
	return EXIT_SUCCESS;
}
