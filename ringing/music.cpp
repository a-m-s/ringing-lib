// -*- C++ -*- music.cpp - Musical Analysis
// Copyright (C) 2001 Mark Banner <mark@standard8.co.uk>

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

#ifdef __GNUG__
#pragma implementation
#endif

#include <ringing/common.h>
#include <ringing/music.h>

RINGING_START_NAMESPACE

RINGING_USING_STD

// ********************************************************
// function definitions for MUSIC_DETAILS
// ********************************************************

music_details::music_details(const string &e, const int &s) : string(e)
{
  _score = s;
  _count_handstroke = 0;
  _count_backstroke = 0;
}

music_details::music_details(const char *e, const int &s) : string(e)
{
  _score = s;
  _count_handstroke = 0;
  _count_backstroke = 0;
}

music_details::~music_details()
{
}

void music_details::Set(const string &e, const int &s)
{
  *this = e;
  _score = s;
  // Should reset this here as we have changed the expression
  _count_handstroke = 0;
  _count_backstroke = 0;
}

void music_details::Set(const char *e, const int &s)
{
  *this = (string) e;
  _score = s;
  // Should reset this here as we have changed the expression
  _count_handstroke = 0;
  _count_backstroke = 0;
}

string music_details::Get() const
{
  return *this;
}

// Return the count
unsigned int music_details::count(const EStroke &stroke) const
{
  switch (stroke)
    {
    case eHandstroke:
      return _count_handstroke;
      break;
    case eBackstroke:
      return _count_backstroke;
      break;
    case eBoth:
      return _count_handstroke + _count_backstroke;
      break;
    default:
      return 0;
    }
  return 0;
}

// Return the calculated score
int music_details::total(const EStroke &stroke) const
{
  return count(stroke) * _score;
}

int music_details::raw_score() const
{
  return _score;
}

// Clear the Current counts
void music_details::clear()
{
  _count_backstroke = 0;
  _count_handstroke = 0;
}

// Which count to increment
void music_details::increment(const EStroke &stroke)
{
  if (stroke == eBackstroke)
    {
      _count_backstroke++;
    }
  else if (stroke == eHandstroke)
    {
      _count_handstroke++;
    }
}

// ********************************************************
// function definitions for MUSIC_NODE
// ********************************************************

#if RINGING_USE_EXCEPTIONS
music_node::memory_error::memory_error() 
  : overflow_error("Not enough memory to allocate to music_node item") {}
#endif

music_node::music_node()
{
  bells = 0;
}

music_node::music_node(const unsigned int &b)
{
  bells = b;
}

music_node::~music_node()
{
  BellNodeMapIterator i;
  for (i = subnodes.begin(); i != subnodes.end(); i++)
    {
      if (i->second != NULL)
	{
	  delete i->second;
	}
    }
}

void music_node::set_bells(const unsigned int &b)
{
  bells = b;
  // for each subnode
  BellNodeMapIterator i;
  for (i = subnodes.begin(); i != subnodes.end(); i++)
    {
      if (i->second != NULL)
	i->second->set_bells(b);
    }
}

void music_node::add(const music_details &md, const unsigned int &i, const unsigned int &key, const unsigned int &pos)
{
  // Does this item end here?
  if (i >= md.size())
    {
      detailsmatch.push_back(key);
    }
  else if (pos <= bells)
    {
      bell b;
      bool isbell = true;

#if RINGING_USE_EXCEPTIONS
      try
	{
	  b.from_char(md[i]);
	}
      catch (exception &)
	{
	  isbell = false;
	}
#else
      b.from_char(md[i]);
      if (b > bell::MAX_BELLS)
	{
	  isbell = false;
	}
#endif
      if (isbell)
	{
	  // Simple bell, add it and move on.
	  add_to_subtree(b + 1, md, i, key,  pos, false);
	}
      else
	{
	  if (md[i] == '?')
	    {
	      add_to_subtree(0, md, i, key, pos, false);
	    }
	  else if (md[i] == '*')
	    {
	      if (md.size() == i + 1)
		{
		  // no more bells to go, don't bother adding to the subtree.
		  // just add here
		  detailsmatch.push_back(key);
		}
	      else
		{
		  // There are more to go
		  // Any of them '*'s?
		  if (md.find('*', i + 1) >= md.size())
		    {
		      // Deal with the only *5 to go in the add_to_subtree
		      // function.
		      add_to_subtree(0, md, i, key, pos, true);
		    }
		  else
		    {
		      // We have *456*
		      // This functionality to be implemented.
		      // First ignore the star and just move on.
		      add(md, i + 1, key, pos);
		      // Now deal with the star
		      if (md.size() - i - 1 >= pos)
			add_to_subtree(0, md, i, key, pos, true);
		      else
			add_to_subtree(0, md, i, key, pos, false);
		    }
		}
	    }
	}
    }
}

void music_node::add_to_subtree(const unsigned int &place, const music_details &md, const unsigned int &i, const unsigned int &key, const unsigned int &pos, const bool &process_star)
{
  BellNodeMap::iterator j = subnodes.find(place);
  if (j == subnodes.end())
    {
      music_node *mn = new music_node(bells);
      if (mn != NULL)
	j = (subnodes.insert(make_pair(place, mn))).first;
      else
	{
#if RINGING_USE_EXCEPTIONS
	  throw memory_error();
	  return;
#else
	  cerr << "Not enough memory to allocate to new music_node\n";
	  return;
#endif
	}
    }
  if (process_star)
    {
      // We are to process star data star.
      if ((bells - pos == md.size() - i) &&
	  (md.find('*', i + 1) >= md.size()))
	{
	  // There are now only numbers to go.
	  j->second->add(md, i + 1, key, pos + 1);
	}
      else
	{
	  // We haven't got to the last * position yet, so carry on.
	  j->second->add(md, i, key, pos + 1);
	}
    }
  else
    {
      // Not a star, so move on as normal.
      j->second->add(md, i + 1, key, pos + 1);
    }
}

void music_node::match(const row &r, const unsigned int &pos, vector<music_details> &results, const EStroke &stroke)
{
  DetailsVectorIterator i;
  for (i = detailsmatch.begin(); i < detailsmatch.end(); i++)
    {
      results[*i].increment(stroke);
    }
  // Try all against ? or *
  BellNodeMapIterator j = subnodes.find(0);
  if (j != subnodes.end())
    {
      j->second->match(r, pos + 1, results, stroke);
    }
  // Now try the actual number
  j = subnodes.find(r[pos] + 1);
  if (j != subnodes.end())
    {
      j->second->match(r, pos + 1, results, stroke);
    }
}

// ********************************************************
// function definitions for MUSIC
// ********************************************************

// default constructor.
music::music(const unsigned int &b) : TopNode(b)
{
  // Reset the music
  reset_music();
}

// Specify the music and add it into the search structure
void music::push_back(const music_details &md)
{
  MusicInfo.push_back(md);
  TopNode.add(md, 0, MusicInfo.size() - 1, 0);
}

music::iterator music::begin()
{
  return MusicInfo.begin();
}

music::const_iterator music::begin() const
{
  return MusicInfo.begin();
}

music::iterator music::end()
{
  return MusicInfo.end();
}

music::const_iterator music::end() const
{
  return MusicInfo.end();
}

music::size_type music::size() const
{
  return MusicInfo.size();
}

void music::set_bells(const unsigned int &b)
{
  TopNode.set_bells(b);
}

// reset_music - clears all the music information entries.
void music::reset_music(void)
{
  music::iterator i;
  for (i = begin(); i != end(); i++)
    i->clear();
}

// process_row - works out if a certain row is considered musical,
// and increments or changes the appriopriate variable.
void music::process_row(const row &r, const bool &back)
{
  if (back)
    TopNode.match(r, 0, MusicInfo, eBackstroke);
  else
    TopNode.match(r, 0, MusicInfo, eHandstroke);
}

// Return the total score for all items
int music::Get_Score(const EStroke &stroke)
{
  int total = 0;
  music::const_iterator i;
  for (i = begin(); i != end(); i++)
    total += i->total(stroke);
  return total;
}

// Return the total matches for all items
unsigned int music::Get_Count(const EStroke &stroke)
{
  unsigned int count = 0;
  music::const_iterator i;
  for (i = begin(); i != end(); i++)
    count += i->count(stroke);
  return count;
}

RINGING_END_NAMESPACE
