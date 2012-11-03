/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#define VERSION "2.0"

/*
=============================================================================

RTSMAKER

=============================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <direct.h>


#include "cmdlib.h"
#include "scriplib.h"

#define MAXSOUNDS 10
//================
//
// wad file types
//
//================

typedef struct
{
	long            filepos;
	long            size;
	char            name[8];
} lumpinfo_t;


typedef struct
{
	char    identification[4];
	long    numlumps;
	long    infotableofs;
} wadinfo_t;

//==========================================================================

char            destpath[MAXPATH];
wadinfo_t       newwad;                                 // the file being written out
lumpinfo_t      *newlumps;
int                     newhandle;
lumpinfo_t      *lump_p;                                // wadlumps[lumpon]
int                     lumpon;                                 // current lump

char                    loadwadpath[MAXPATH];
char                    lumppath[MAXPATH];
int                     datahandle;                             // can be an individual file or a WAD
wadinfo_t       loadwad;                                // a WAD file to look for lumps in
lumpinfo_t      *wadlumps;
lumpinfo_t      *wadlump_p;                             // info on lump to copy
boolean         inwadfile;

boolean outputnamed;


char    sourcepath[MAXPATH];
char    defaultdestpath[MAXPATH];


int             lumpscopied;                            // number of copy operations done
char    scriptfilename[MAXPATH];

//============================================================================


/*
================
=
= OpenLump
=
= If in a wad file, lseeks to the start of the lump
=
= If a separate file, sets inputhandle to the open file
=
================
*/

void OpenLump (void)
{
	int                     i;
	char            lumpname[9];

#if 0
	if (inwadfile)
	{
		lumpname[8] = 0;
		strncpy (lumpname, lump_p->name, 8);

	//
	// set wadlump_p to the lumpinfo in the open wad file
	//
		wadlump_p = wadlumps;
		for (i=0 ; i<loadwad.numlumps ; i++, wadlump_p++)
			if (strncmp( lump_p->name , wadlump_p->name, 8) == 0)
				break;

		if (i == loadwad.numlumps)
			Error ("lump %s is not in wad file %s\n",lumpname,lumppath);

		strcpy (lumppath, loadwadpath);
		strcat (lumppath, " : ");
		strcat (lumppath, lumpname);
	}
	else
#endif
	{
	//
	// open the file on disk
	//
		datahandle = SafeOpenRead (lumppath);
	}

}


/*
=================
=
= CopyLump
=
=================
*/

#define MAXCOPYSIZE (16384)
void CopyLump (void)
{
	long    size;
   long    tempsize;
	byte            *buffer;

#if 0
	if (inwadfile)
	{
		lseek (datahandle, LittleLong (wadlump_p->filepos) ,SEEK_SET);
		size = LittleLong (wadlump_p->size);
	}
	else
#endif
		size = filelength (datahandle);

	printf ("%4i = %s (%lu bytes)\n",lumpon,lumppath,size);

	lump_p->filepos = LittleLong (tell(newhandle));
	lump_p->size = LittleLong (size);

   buffer = SafeMalloc (MAXCOPYSIZE);
   while (size>0)
      {
      tempsize=MAXCOPYSIZE;
      if (tempsize>size)
         tempsize=size;

      SafeRead (datahandle, buffer, tempsize);
      SafeWrite (newhandle, buffer, tempsize);
      size -= tempsize;
      }
   SafeFree (buffer);

#if 0
	if (!inwadfile)
		close (datahandle);
#endif

	lumpon++;
	lump_p++;
	lumpscopied++;
}

/*
=============================================================================

					   SCRIPT COMMANDS

=============================================================================
*/

/*
=================
=
= UnpackRTS
=
=================
*/
void UnpackRTS (char * filename)
   {
   long i;
   long handle;
   lumpinfo_t * lptr;
   long size;
   long tempsize;
   byte tempbyte;
   byte * buffer;
   char tempname[80];
//
// open it and read in header / lump directory
//
   datahandle = SafeOpenRead (filename);
	SafeRead (datahandle,&loadwad,sizeof(loadwad));
	loadwad.numlumps = LittleLong (loadwad.numlumps);
	loadwad.infotableofs = LittleLong (loadwad.infotableofs);

	if (strncmp(loadwad.identification,"IWAD",4))
		Error ("Wad file %s doesn't have IWAD id\n",lumppath);

	lseek (datahandle , loadwad.infotableofs , SEEK_SET);

   wadlumps = SafeMalloc ( loadwad.numlumps*sizeof(lumpinfo_t) );
	SafeRead (datahandle, wadlumps , loadwad.numlumps*sizeof(lumpinfo_t));
   for (i=0;i<loadwad.numlumps;i++)
      {
      lptr = &wadlumps[i];
      if (!lptr->size)
         {
         continue;
         }
      lseek (datahandle, LittleLong (lptr->filepos) ,SEEK_SET);
      size = LittleLong (lptr->size);
      memset(tempname,0,sizeof(tempname));
      strncpy(tempname,lptr->name,8);
      SafeRead (datahandle, &tempbyte, 1);
      lseek (datahandle, LittleLong (lptr->filepos) ,SEEK_SET);
      if (tempbyte == 'C')
         {
         strcat(tempname,".voc");
         }
      else
         {
         strcat(tempname,".wav");
         }
      printf("   writing %s\n",tempname);
      handle = SafeOpenWrite (tempname);
      buffer = SafeMalloc (MAXCOPYSIZE);
      while (size>0)
         {
         tempsize=MAXCOPYSIZE;
         if (tempsize>size)
            tempsize=size;

         SafeRead (datahandle, buffer, tempsize);
         SafeWrite (handle, buffer, tempsize);
         size -= tempsize;
         }
      SafeFree (buffer);
      close(handle);
      }
   close(datahandle);
   }


#if 0
/*
=================
=
= CmdCloseWad
=
=================
*/

void CmdCloseWad (void)
{
	if (!inwadfile)
		Error ("$CLOSEWAD issued without an open wad file\n");

	close (datahandle);
   SafeFree (wadlumps);

	inwadfile = false;
}


/*
=================
=
= CmdOpenWad
=
=================
*/

void CmdOpenWad (void)
{
	if (inwadfile)
		CmdCloseWad ();

//
// get and qualify wad file name
//
	GetToken (false);
	strcpy (loadwadpath, token);
	DefaultExtension (loadwadpath, ".wad");
	DefaultPath (loadwadpath, sourcepath);

//
// open it and read in header / lump directory
//
	datahandle = SafeOpenRead (loadwadpath);
	SafeRead (datahandle,&loadwad,sizeof(loadwad));
	loadwad.numlumps = LittleLong (loadwad.numlumps);
	loadwad.infotableofs = LittleLong (loadwad.infotableofs);

	if (strncmp(loadwad.identification,"IWAD",4))
		Error ("Wad file %s doesn't have IWAD id\n",lumppath);

	lseek (datahandle , loadwad.infotableofs , SEEK_SET);

   wadlumps = SafeMalloc ( loadwad.numlumps*sizeof(lumpinfo_t) );
	SafeRead (datahandle, wadlumps , loadwad.numlumps*sizeof(lumpinfo_t));

	inwadfile = true;

}


/*
=================
=
= CmdShootWad
=
= Copies every lump in a wad file into the output
=
=================
*/

void CmdShootWad (void)
{
	int             i;
   int             start,end;

	CmdOpenWad ();

   start = lseek (newhandle , 0, SEEK_CUR);
   for (i=0 ; i<loadwad.numlumps ; i++)
	{
		strncpy (lump_p->name , wadlumps[i].name , 8);
		OpenLump ();
		CopyLump ();
	}
   end = lseek (newhandle , 0, SEEK_CUR);

   CmdCloseWad ();
}


/*
===================
=
= CmdLabel
=
===================
*/

void CmdLabel (void)
{
	GetToken (false);

	lump_p->filepos = 0;
	lump_p->size = 0;

	strupr (token);
	strncpy(  lump_p->name, token, 8);

	lumpon++;
	lump_p++;
}
#endif

/*
===================
=
= AddLabel
=
===================
*/

void AddLabel ( char * string )
{
	lump_p->filepos = 0;
	lump_p->size = 0;

	strupr (token);
	strncpy(  lump_p->name, string, 8);

	lumpon++;
	lump_p++;
}

/*
=====================
=
= CmdRTSName
=
=====================
*/

void CmdRTSName (char *name)
{

//
// get the output filename
//
	strcpy (destpath,name);
	DefaultPath (destpath, defaultdestpath);
	DefaultExtension (destpath, ".RTS");

	printf ("Output rts file: %s\n",destpath);

	newhandle = SafeOpenWrite (destpath);

	newwad.numlumps = 0;
	strncpy (newwad.identification, "IWAD", 4);

//
// allocate space for the lump directory
//
   newlumps = SafeMalloc (0x2000);
	lump_p = newlumps;
	lumpon = 0;

//
// position the file pointer to begin writing data
//
	// leave space in the data file for the header
	lseek (newhandle,sizeof(newwad),SEEK_SET);

}



//==========================================================================
#if 0
/*
================
=
= CheckCommands
=
================
*/

boolean CheckCommands (void)
{
	if (token[0] != '$')
		return false;

//
// link commands
//
	if (!strcmpi(token,"$WADNAME") )
	{
		GetToken (false);
		CmdWadName (token);
		return true;
	}

	if (!strcmpi(token,"$OPENWAD") )
	{
		CmdOpenWad ();
		return true;
	}

	if (!strcmpi(token,"$SHOOTWAD") )
	{
		CmdShootWad ();
		return true;
	}

	if (!strcmpi(token,"$CLOSEWAD") )
	{
		CmdCloseWad ();
		return true;
	}

	if (!strcmpi(token,"$LABEL") )
	{
		CmdLabel ();
		return true;
	}

	Error ("Unrocognized command %s\n",token);
	return false;
}

#endif

/*
=============================================================================

						MAIN LOOP

=============================================================================
*/


/*
===================
=
= ProcessScript
=
===================
*/

void ProcessScript (void)
{
   int i;

	lumpscopied = 0;
	outputnamed = false;

	//
	// get name of rts file
	//
   GetToken (true);

   CmdRTSName (token);

   // start of remote sound pack

   AddLabel ("REMOSTRT");

   for (i=0;i<MAXSOUNDS;i++)
      {
		GetToken (true);

		if (endofscript)
			break;                       // broke prematurely

		strcpy (lumppath,token);
		DefaultPath (lumppath,sourcepath);
		DefaultExtension (lumppath,".voc");
		ExtractFileBase (lumppath,lump_p->name);

		OpenLump ();                    // Find the lump data, either in a seperate file or in the comp file
		CopyLump ();                    // copy the lump to the output wad
      }
   if (i!=MAXSOUNDS)
      {
      unlink(destpath);
      Error("Only %d of %d vocs were specified in the RTS script file %s\n", i, MAXSOUNDS, &scriptfilename[0]);
      }

   // end of remote sound pack
   AddLabel ("REMOSTOP");

}


/*
===================
=
= WriteDirectory
=
===================
*/

void WriteDirectory (void)
{
//
// write lumpinfo table
//
	newwad.numlumps = LittleLong (lumpon);
	newwad.infotableofs = LittleLong (tell(newhandle));
	write (newhandle,newlumps, lumpon*sizeof(lumpinfo_t));

//
// write wadinfo
//
	lseek (newhandle,0,SEEK_SET);
	write (newhandle,&newwad,sizeof(newwad));

	close (newhandle);
}


/*
===================
=
= main
=
===================
*/

int main (int argc, char **argv)
{
   int i;
	printf ("\nRTSMAKER "VERSION" Copyright (c) 1996 3D Realms Entertainment.\n\n");

//
// check for help
//
	if (CheckParm ("?") || (argc==1))
	{
		printf (
"Usage: RTSMAKER -u(npack) [nameofscriptfile]\n\n"
"RTS files are Remote-Ridicule (tm) sound files used in the multi-\n"
"player modes of Duke Nukem 3D and Rise of the Triad.\n\n"
"Duke Nukem 3D\n"
"To use a different RTS file in Duke Nukem 3D, select 'Change RTS File'\n"
"from the Modem, Network, or Serial Game menus in SETUP.EXE.\n\n"
"Rise of the Triad\n"
"To use a different RTS file in Rise of the Triad, select USE MODIFIED\n"
"STUFF and CHOOSE REMOTE PLAYER SOUNDS from SETUP.EXE.\n\n"
"SCRIPT FILE FORMAT:\n"
"   <Name of RTS file>\n"
"   <VOC or WAV file 1>\n"
"            .\n"
"            .\n"
"            .\n"
//"   <VOC or WAV file 2>\n"
//"   <VOC or WAV file 3>\n"
//"   <VOC or WAV file 4>\n"
//"   <VOC or WAV file 5>\n"
//"   <VOC or WAV file 6>\n"
//"   <VOC or WAV file 7>\n"
//"   <VOC or WAV file 8>\n"
//"   <VOC or WAV file 9>\n"
"   <VOC or WAV file 10>\n"
"   ';' can be used for comments\n"
//"'sample.txt' is provided as an example."
      );
		exit (1);
	}

   i=CheckParm ("U");
   if (i)
      {
      printf ("Unpacking-> %s\n",argv[i+1]);
      UnpackRTS (argv[i+1]);
      exit(0);
      }

//
// get source directory for data files
//
   getcwd (sourcepath,MAXPATH);

 	if (sourcepath[strlen(sourcepath)-1] != '\\')
		strcat (sourcepath,"\\");

	printf ("Source directory     : %s\n",sourcepath);

//
// get destination directory for link file
//
   getcwd (defaultdestpath,MAXPATH);
	if (defaultdestpath[strlen(defaultdestpath)-1] != '\\')
		strcat (defaultdestpath,"\\");

	printf ("Destination directory: %s\n",defaultdestpath);

//
// get script file
//
   strcpy (scriptfilename,argv[1]);
	printf ("Script file          : %s\n",scriptfilename);
	LoadScriptFile (scriptfilename);

//
// start doing stuff
//
	ProcessScript ();
	WriteDirectory ();
	printf ("\nAll VOC and WAV files grabbed into RTS file %s\n",destpath);

	return 0;
}
