// -*- C++ -*- music.cpp - things to analyse music
// Copyright (C) 2002, 2003, 2008, 2009 Richard Smith <richard@ex-parrot.com>

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

#if RINGING_HAS_PRAGMA_INTERFACE
#pragma implementation "methsearch/music"
#endif

#include "music.h"
#if RINGING_OLD_C_INCLUDES
#include <string.h>
#include <assert.h>
#else
#include <cstring>
#include <cassert>
#endif
#if RINGING_OLD_INCLUDES
#include <vector.h>
#include <map.h>
#else
#include <vector>
#include <map>
#endif
#include <ringing/row.h>
#include <ringing/method.h>
#include <ringing/music.h>
#include <ringing/streamutils.h>


RINGING_USING_NAMESPACE
RINGING_USING_STD


class musical_analysis::patterns
{
private:
  patterns() {}
  
public:
 ~patterns() {}
  
  static patterns &instance() 
  { static patterns tmp; return tmp; }

  friend class analyser;
  
  // Data members
  vector<string> p;
};

class musical_analysis::analyser
{
private:
  analyser( int bells );

public:
  // This is public to get it to compile in MSVC.
 ~analyser() {}
    
  static analyser &instance( int bells ) 
  { static analyser tmp( bells ); return tmp; }

  friend class patterns;
    
  void check_n( int n );

  // Data members
  int bells;

  enum length { half_lead, half_lead_2, half_lead_r, half_lead_2r, 
                lead, course };
  map< pair<row,length>, music > musv;
};


musical_analysis::analyser::analyser( int bells )
  : bells(bells)
{
  const vector<string> &p = patterns::instance().p;
  row lh(bells); // Course or lead head -- plain course by default
  length len = course; // Look at whole course by default  

  for ( vector<string>::const_iterator i( p.begin() ), e( p.end() );
	    i != e; ++i )
    {
      int score( 1 );
      string pattern( *i );

      string::size_type eq = pattern.find('=');
      if (eq == string::npos) eq = pattern.size();

      string const opt = pattern.substr(0,eq);

      if ( opt == "course" || opt == "lead" || opt == "halflead" ||
           opt == "2halflead" || opt == "2rhalflead" || opt == "rhalflead" )
        {
          if (opt == "course") len = course;
          else if (opt == "lead") len = lead;
          else if (opt == "halflead") len = half_lead;
          else if (opt == "2halflead") len = half_lead_2;
          else if (opt == "2rhalflead") len = half_lead_2r;
          else if (opt == "rhalflead") len = half_lead_r;

          if (eq == pattern.size()) continue; // No argument given

          string lhstr = pattern.substr(eq+1);
          // Allow treble to be omitted
          if ( lhstr.find( bell(0).to_char() ) == string::npos ) 
            lhstr = bell(0).to_char() + lhstr;

          try {          
            lh = row(lhstr);
          } catch ( exception const& e ) {
            cerr << "Unable to parse music course head: " << e.what() << endl;
            exit(1);
          }

          lh *= row(bells); // Pad to correct number of bels

          continue;
        }

      if ( pattern.size() > 2 && isdigit( pattern[0] ) )
        {
          for ( unsigned int j = 0; j < pattern.size(); ++j )
            if ( pattern[j] == ':' )
              {
                score = atoi( pattern.c_str() );
                pattern = pattern.substr(j+1);
                break;
              }
            else if ( !isdigit( pattern[j] ) )
              break;
        }

      music& mu = musv[ make_pair(lh, len) ];
      mu.set_bells(bells);

      try {
        if ( pattern.size() > 2 
             && pattern[0] == '<' && pattern[ pattern.size()-1 ] == '>' )
          add_named_music( mu, pattern.substr( 1, pattern.size()-2 ), score );

        else
          mu.push_back( music_details( pattern, score ) );
      } 
      catch ( exception const& e ) {
        cerr << "Error parsing music pattern: " << e.what() << endl;
        exit(1);
      }           
    }

  // By default we count the CRUs in the plain course.
  if ( musv.empty() ) 
    {
      music& mu = musv[ make_pair(lh, len) ];
      mu.set_bells(bells);
      add_named_music( mu, "CRUs", 1 ); 
    }
}

void musical_analysis::add_pattern( const string &str )
{
  patterns::instance().p.push_back( str );
}

int musical_analysis::analyse( const method &m )
{
  int score = 0;

  typedef map< pair<row,analyser::length>, music > musv_t;
  musv_t& musv = analyser::instance( m.bells() ).musv;
  for ( musv_t::iterator mi=musv.begin(), me=musv.end(); mi!=me; ++mi)
    {
      vector< row > rows;
      row r = mi->first.first; // Set to the course head

      switch ( mi->first.second ) {
        case analyser::course:
          do 
            transform( m.begin(), m.end(), back_inserter(rows), permute(r) );
          while ( r != mi->first.first );
          break;

        case analyser::lead:
          transform( m.begin(), m.end(), 
                     back_inserter(rows), post_permute(r) );
          break;

        case analyser::half_lead:
          transform( m.begin(), m.begin() + m.size()/2, 
                     back_inserter(rows), post_permute(r) );
          break;

        case analyser::half_lead_2:
          transform( m.begin() + m.size()/2, m.end(),
                     back_inserter(rows), post_permute(r) );
          break;

        case analyser::half_lead_r:
	  transform( m.rbegin() + m.size()/2, m.rend(),
                     back_inserter(rows), post_permute(r) );
          break;

        case analyser::half_lead_2r:
          transform( m.rbegin(), m.rbegin() + m.size()/2,
                     back_inserter(rows), post_permute(r) );
          break;

        default:
          assert(false);
      }

      mi->second.process_rows( rows.begin(), rows.end() );  
      score += mi->second.get_score();
    }

  return score;
}

void musical_analysis::force_init( int bells )
{
  // Going into this function initialises the static there.
  analyser::instance( bells );
}
