/*  sf: hdump.c                                                         */
/*  10/12/90                                                            */
/*  Hex dump test program for dos ( Windows exercise )                  */
/*  H.G. Barney                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <io.h>
#include "scancode.h"

#define FALSE	0
#define	TRUE	1
#define EXTENDED_CHAR 0
#define TEXTLENGTH	16

// block sizes for CTRL_PGUP etc keys
#define ONE_KBYTES		1024L
#define TWO_KBYTES		2048L
#define FOUR_KBYTES		4096L
#define EIGHT_KBYTES		8192L
#define	SIXTEEN_KBYTES		16384L
#define	THIRTYTWO_KBYTES	32768L
#define	SIXTYFOUR_KBYTES	65536L

// macros 
#define	HIBITS(x)	(char)( (x & 0xF0) >> 4 )
#define LOBITS(x)	(char)( x & 0x0F )
#define	cls()		printf("%c[2J",27)
#define beep()		printf("%c",7)

//macro to calc the number of bytes per on a full page
#define	nFULL_PAGE  ( (long)PageLength * TEXTLENGTH )

char	HexChar[] = "0123456789ABCDEF";

void	main( int, char *[] );
char	*GetDumpLine(char *, int);
void	RemoveControlChars( char *, int);
int	PageForward(   FILE *, long);
int	PageBackward(  FILE *, long, long);
int	GotoEOF(FILE *, long,  long);
int	BlockForward(FILE *, long, long, long);
int	BlockBackward(FILE *, long, long, long);


void main( int argc, char *argv[] )
{
	FILE	*fp;
	int 	EndProgram = FALSE;
	long	PageLength = 24L;
	int	command;
	long	FileSize;	
	long	BlockSize;
	
	// abort if filename not supplied
	if (argc != 2)
	{
		printf("\nHDUMP Hex Dump Program         (C) H.G. Bareny 1990");
		printf("\nUsage : hdump filename\n");
		printf("\n\nCommand Keys:");
		printf("\nEscape\t:\tExit");
		printf("\nPgDn\t:\tNext Page");
		printf("\nPgUp\t:\tPrev Page");
		printf("\nHome\t:\tTop of File");
		printf("\nEnd\t:\tEnd of File");
		
		exit(1);
	}

	if ( (fp = fopen(argv[1],"rb")) == NULL)
	{
		printf("\nUnable to open %s\n",argv[1]);
		exit(2);
	}

	// get file size in bytes	
	FileSize = filelength( fileno(fp) );

	// clear screen
	cls();
	
	// show first screens worth
	PageForward(fp, PageLength);
	
	while( !EndProgram )
	{
		command = getch();

		// if user requests exit via escape key
		if ( command == ESCAPE)
		{
			EndProgram = TRUE;
			continue;
		}
		else if ( command != EXTENDED_CHAR )
			continue;
		else
			command = getch();

	
		switch( (int)command)
		{
			case PAGE_DOWN: // move down 1 page
				PageForward(fp, PageLength);
				break;

			case PAGE_UP: // move back up 1 page 
				BlockSize = nFULL_PAGE;
				BlockBackward(fp, FileSize, BlockSize, PageLength);
				break;

			case HOME:  // goto top of file
				if ( ftell(fp) > nFULL_PAGE )
				{
					// in middle or at end of file 
					rewind( fp );
					PageForward(fp, PageLength);
				}
				else
				{	// on first page
					beep();
				}
				break;
				
			case END:		// goto end of file
				GotoEOF(fp, FileSize, PageLength);
				break;
			
			case CTRL_PAGE_UP:	// move up file by small block size
				BlockSize = ONE_KBYTES;
				BlockBackward(fp, FileSize, BlockSize, PageLength);
				break;

			case CTRL_PAGE_DOWN:	// move down file by small block size
				BlockSize = ONE_KBYTES;
				BlockForward(fp, FileSize, BlockSize, PageLength);
				break;
			
			case CTRL_HOME: 	// move up file by large block size
				BlockSize = SIXTEEN_KBYTES;
				BlockBackward(fp, FileSize, BlockSize, PageLength);
				break;

			case CTRL_END:		// move down file by large block size
				BlockSize = SIXTEEN_KBYTES;
				BlockForward(fp, FileSize, BlockSize, PageLength);
				break;
			
			default:
				break;
		}
	}
	fclose(fp);
}


int	BlockForward(FILE *fp, long FileSize, long BlockSize, long PageLength)
{
	int	retval;
	
	// if there is not more than BlockSize left then goto end
	if ( (FileSize - ftell(fp)) < BlockSize)
	{
		GotoEOF(fp, FileSize, PageLength);
		return(0);
	}
	else
		retval = fseek(fp, BlockSize - nFULL_PAGE , SEEK_CUR);
	
	if ( retval != 0)
	{
		printf("\nFile Seek Error\n");
		exit(0);
	}
	
	PageForward(fp, PageLength);
	return(0);
}




int	BlockBackward(FILE *fp, long FileSize, long BlockSize, long PageLength)
{
	int	retval;
	long	nLastBlockSegment;	// the last incomplete 16K segment of the file

	// if file is small enough for 1 page do nothing
	if (FileSize < nFULL_PAGE )
	{
		beep();
		return(0);
	}

	// if end of file then position at the top of the last incomplete block
	if ( feof(fp) )	// at end of file
	{
		if ( BlockSize == nFULL_PAGE )
		{
			// go back by the length of last page plus a full page
			nLastBlockSegment = FileSize % nFULL_PAGE;
			retval = fseek(fp, -nLastBlockSegment - nFULL_PAGE, SEEK_END);
		}
		else
		{
			nLastBlockSegment = FileSize %  BlockSize;
			
			if ( nLastBlockSegment  < nFULL_PAGE )
			{
				// last segment is smaller than a page
				retval = fseek(fp, -nLastBlockSegment - BlockSize, SEEK_END);
			}
			else if ( (BlockSize + nFULL_PAGE) > ftell(fp) )
			{
				// not enough file above so go to top of file
				retval = fseek(fp, 0L, SEEK_SET);
			}
			else
			{	// move up by the size of the last incomplete block
				retval = fseek(fp, -nLastBlockSegment, SEEK_END);
			}
		}
	}
	else		// in middle of file
	{
		if ( ftell(fp) <= nFULL_PAGE )
		{
			// on first page already - do nothing
			beep();
			return(0);
		}
		else if ( (BlockSize + nFULL_PAGE) > ftell(fp) )
		{
			// not enough file above so set to top of file
			retval = fseek(fp, 0L, SEEK_SET);
		}
		else
		{	// enough space above so move a full block back
			retval = fseek(fp, -BlockSize - nFULL_PAGE, SEEK_CUR);
		}
	}
	
	if ( retval != 0)
	{
		printf("\nFile Seek Error\n");
		exit(0);
	}
	
	PageForward(fp, PageLength);
	return(0);
}




int	PageForward(FILE *fp, long PageLength)
{
	long	FilePos;
	char	TextLine[TEXTLENGTH + 1];
	char	*DumpLine;
	int	i, NumRead;
	
	// if end of file has been reached do not display beyond it
	if ( feof(fp) )
	{
		beep();
		return(-1);
	}
	
	TextLine[TEXTLENGTH] = '\0';
	cls();
	
	for( i = 0; i < (int)PageLength; i++)
	{
		strset(TextLine,'\0');
		NumRead = fread(TextLine, sizeof( char ), TEXTLENGTH, fp);
		DumpLine = GetDumpLine( TextLine, NumRead );
		RemoveControlChars( TextLine, NumRead );
		FilePos =  ftell(fp);
		FilePos =  FilePos - (long)NumRead;
		
		printf("\n%04X:%04X  %s   %s",
			(int)( ( FilePos & 0xFFFF0000 ) >> 16),
			(int)( FilePos & 0x0000FFFF ),
			DumpLine, TextLine );

		if (NumRead < TEXTLENGTH)
			break;
	}

	return(0);
}	




int	GotoEOF(FILE *fp, long FileSize, long PageLength)
{
	int	retval;
	long	nLastPageBytes;
	
	// if file is small enough for 1 page or at end of file - do nothing
	if ( FileSize < nFULL_PAGE || feof(fp) )
	{
		beep();
		return(0);
	}

	// position at start of last page worth
	nLastPageBytes = FileSize %  nFULL_PAGE;
	retval = fseek(fp, -nLastPageBytes, SEEK_END);
	
	if ( retval != 0)
	{
		printf("\nFile Seek Error\n");
		exit(0);
	}

	PageForward(fp, PageLength);
	return(0);
}
	


char	*GetDumpLine(char *TextLine, int NumRead)
{
	static	char	DumpLine[48];
	int 	i;

	strset(DumpLine,'\0');	

	// convert text line to Hex output string	
	for(i = 0; i < NumRead; i++)
	{
		DumpLine[i * 3 ]	= HexChar[ HIBITS(TextLine[i]) ];
		DumpLine[i * 3 + 1]	= HexChar[ LOBITS(TextLine[i]) ];
		DumpLine[i * 3 + 2] 	= ' ';
	}

	// put spaces on last line of hex dump if last line not TEXTLENGTH chars
	for( i = NumRead; i < TEXTLENGTH; i++)
	{
		DumpLine[i * 3 ]	= ' ';
		DumpLine[i * 3 + 1]	= ' ';
		DumpLine[i * 3 + 2] 	= ' ';
	}
	
	// remove last space from line
	DumpLine[47] = '\0';
	
	// put '-' in middle of Hex output
	DumpLine[23] = '-';

	return(DumpLine);	
}



void RemoveControlChars( char *TextString, int NumRead)
{
	char	 *Tptr;
	int	 i;
	
	Tptr = TextString;

	// replace all control chars with dots
	for(i = 0; i < NumRead; i++)
	{
		if ( (unsigned)*Tptr < 32 )
			*Tptr = '.';
			
		Tptr++;
	}
}
