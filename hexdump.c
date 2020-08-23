/*
FILE:       hexdump.c
DATE:       13-Nov-96
AUTHOR:     H G Barney
PURSPOSE:    Basic Hex Dump Program

1-Aug-2012 - updated to remove strlwr() function for linux compatibility
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXTLENGTH	16
#define DUMPLENGTH  50
#define	HIBITS(x)	(char)( ((x) & 0xF0) >> 4 )
#define LOBITS(x)	(char)( (x) & 0x0F )

void main( int, char *[] );
void usage(char * );
void getDumpText( void );
void hideControlChars( void );

int	giBytesRead;
char gszText[ TEXTLENGTH + 1 ];
char gszDump[ DUMPLENGTH ];

void main( int argc, char *argv[] )
{
	FILE*	fp		= NULL;
	long	lFilePos	= 0L;

	if ( argc == 2 )
	{
		fp = fopen( argv[1], "rb" );

		if ( fp == NULL )
		{
			fprintf( stderr, "%s: could not open %s\n", argv[0], argv[1] );
			usage(argv[0]);
			exit(1);
		}
	}
	else if ( argc == 1 )
	{
		fp = stdin;
	}
	else
	{
		fprintf( stderr, "%s: invalid args\n", argv[0] );
		usage(argv[0]);
		exit(1);
	}

	while( !feof(fp) )
	{
	    memset( gszText, '\0', TEXTLENGTH + 1);
		giBytesRead = fread( gszText, sizeof( char ), TEXTLENGTH, fp );
		getDumpText();
		hideControlChars();

		fprintf( stdout, "%04X:%04X  %s   %s\n",
			(int)( ( lFilePos & 0xFFFF0000 ) >> 16),
			(int)( lFilePos & 0x0000FFFF ),
			gszDump, gszText );

		lFilePos += 16;

		if ( giBytesRead < TEXTLENGTH )
			break;
	}

	if ( fp != stdin )
		fclose( fp );

	exit(0);
}

void usage(char *name)
{
	fprintf( stdout, "Usage:  %s [<filename>]\n", name );
}

void getDumpText()
{
	static	char	szHex[] = "0123456789ABCDEF";
	int	i;

	//strset( gszDump, '\0' );
	memset( gszDump, '\0', DUMPLENGTH );

	/* convert text line to Hex output string */
	for( i = 0; i < giBytesRead; i++ )
	{
		gszDump[i * 3 ]		= szHex[ HIBITS( gszText[i] ) ];
		gszDump[i * 3 + 1]	= szHex[ LOBITS( gszText[i] ) ];
		gszDump[i * 3 + 2] 	= ' ';
	}

	/*
	put spaces on last line of hex dump if last line not TEXTLENGTH chars
	*/
	for( i = giBytesRead; i < TEXTLENGTH; i++)
	{
		gszDump[i * 3 ]		= ' ';
		gszDump[i * 3 + 1]	= ' ';
		gszDump[i * 3 + 2] 	= ' ';
	}

	/* remove last space from line */
	gszDump[47] = '\0';

	/* put '-' in middle of Hex output */
	gszDump[23] = '-';
}

/* replace all control chars with dots */
void hideControlChars()
{
	char	 *p = gszText;
	int	 i;

	for(i = 0; i < giBytesRead; i++)
	{
		if ( (unsigned)*p < 32 )
			*p = '.';

		p++;
	}
}
