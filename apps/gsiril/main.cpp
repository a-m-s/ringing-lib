// main.cpp - Entry point for gsiril
// Copyright (C) 2002, 2003 Richard Smith <richard@ex-parrot.com>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// $Id$

#include <ringing/common.h>
#include <ringing/pointers.h>
#include "console_stream.h"
#include "parser.h"
#include "execution_context.h"
#if RINGING_OLD_INCLUDES
#include <stdexcept.h>
#else
#include <stdexcept>
#endif
#if RINGING_OLD_IOSTREAMS
#include <iostream.h>
#else
#include <iostream>
#endif
#if RINGING_USE_STRINGSTREAM
#if RINGING_OLD_INCLUDES
#include <sstream.h>
#else
#include <sstream>
#endif
#else // RINGING_USE_STRINGSTREAM
#if RINGING_OLD_INCLUDES
#include <strstream.h>
#else
#include <strstream>
#endif
#endif // RINGING_USE_STRINGSTREAM

RINGING_USING_NAMESPACE


void welcome()
{
  cout << "gsiril " RINGING_VERSION "\n"
          "Ringing Class Library version " RINGING_VERSION << "." 
       << endl;
}

static const char init_string[] = 
  "true     = \"# rows ending in @\", \"Touch is true\"\n"
  "notround = \"# rows ending in @\", \"Is this OK?\"\n"
  "false    = \"# rows ending in @\", \"Touch is false in $ rows\"\n"
  "conflict = \"# rows ending in @\", \"Touch not completed due to false row$$\"\n"
  "rounds   = \n"
  "everyrow = \n"
  "start    = \n"
  "end      = \n"
;


int main( int argc, const char *argv[] )
{
  bool interactive = false;

  for ( int i=1; i<argc; ++i )
    {
      string arg( argv[i] );
      if (arg == "-i" || arg == "--interactive" )
	interactive = true;
    }

  try
    {
      execution_context e( cout );

      // Prepopulate symbol table
      {
#if RINGING_USE_STRINGSTREAM
	istringstream in(init_string);
#else
	istrstream in(init_string);
#endif
	shared_pointer<parser> p( make_default_parser(in) );
	while (true)
	  {
	    try 
	      {
		statement s( p->parse() );
		if ( s.eof() ) break;
		s.execute(e);
	      }
	    catch (const exception& ex )
	      {
		cerr << "Error initialising: " << ex.what() << endl;
		return 1;
	      }
	  }
      }

      e.interactive( interactive );
      if (interactive) welcome();

      // Read from standard input
      {
	console_istream in( interactive );
	in.set_prompt( "> " );
	shared_pointer<parser> p( make_default_parser(in) );
	while (true)
	  {
	    try 
	      {
		statement s( p->parse() );
		if ( s.eof() ) break;
		s.execute( e );
	      }
	    catch (const exception& ex )
	      {
		cerr << "Error: " << ex.what() << endl;
		if (!interactive) return 1;
	      }
	  }
      }

    }
  catch ( const exception &ex )
    {
      cerr << "Unexpected error: " << ex.what() << endl;
      exit(1);
    }
  catch ( ... )
    {
      cerr << "An unknown error occured" << endl;
      exit(1);
    }

  return 0;
}

