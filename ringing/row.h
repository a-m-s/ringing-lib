// -*- C++ -*- row.h - Classes for rows and changes
// Copyright (C) 2001 Martin Bright <M.Bright@dpmms.cam.ac.uk>

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


#ifndef RINGING_ROW_H
#define RINGING_ROW_H

#ifdef __GNUG__
#pragma interface
#endif

#include <ringing/common.h>
#if RINGING_OLD_INCLUDES
#include <iostream.h>
#include <vector.h>
#include <iterator.h>
#include <list.h>
#include <algorithm.h>
#include <stdexcept.h>
#else
#include <iostream>
#include <vector>
#include <iterator>
#include <list>
#include <algorithm>
#include <stdexcept>
#endif
#if RINGING_OLD_C_INCLUDES
#include <ctype.h>
#else
#include <cctype>
#endif
#include <string>

#if _MSC_VER
// Something deep within the STL in Visual Studio decides to 
// turn this warning back on.  
#pragma warning(disable: 4231)
#endif

RINGING_START_NAMESPACE

RINGING_USING_STD

// Declare a couple of random functions for hcf and lcm
RINGING_API int hcf(int a, int b);
inline RINGING_API int lcm(int a, int b) 
{
  // Check for a couple of quick cases
  if(a == 1) return b;
  if(b == 1) return a;
  return a * b / hcf(a,b);
}

class RINGING_API bell {
private:
  unsigned char x;
  static char symbols[];	// Symbols for the individual bells
public:
  bell() : x(0) {}
  bell(int i) : x(i) {}
  void from_char(char c) {
    c = toupper(c);
    for(x = 0; x < 33 && symbols[x] != c; x++);
    if(x == 33) 
#if RINGING_USE_EXCEPTIONS
      throw invalid();
#else
      x = 0;
#endif
  }
  operator int() const { return x; }
  bell& operator=(int i) { x = i; return *this; }
  char to_char() const { return (x < 33) ? symbols[x] : '*'; }

  struct invalid : public invalid_argument {
    invalid();
  };
};

inline RINGING_API ostream& operator<<(ostream& o, const bell b) 
  { return o << b.to_char(); }

class row;

#if RINGING_AS_DLL
RINGING_EXPLICIT_STL_TEMPLATE vector<bell>;
#endif

// change : This stores one change
class RINGING_API change {
private:
  int n;			// Number of bells
  vector<bell> swaps;		// List of pairs to swap

public:
  change() : n(0) {}	        //
  explicit change(int num) : n(num) {}   // Construct an empty change
  change(int num, const char *pn);
  change(int num, const string& s);
  // Use default copy constructor and assignment

  change& set(int num, const char *pn) // Assign from place notation
    { change(num, pn).swap(*this); return *this; }
  change& set(int num, const string& pn)
    { change(num, pn).swap(*this); return *this; }
  bool operator==(const change& c) const
  { return (n == c.n) && (n == 0 || swaps == c.swaps); }
  bool operator!=(const change& c) const
    { return !(*this == c); }
  change reverse(void) const;		 // Return the reverse
  void swap(change& other) {  // Swap this with another change 
    int t = n; n = other.n; other.n = t;
    swaps.swap(other.swaps);
  }

  friend RINGING_API row& operator*=(row& r, const change& c);
  friend RINGING_API bell& operator*=(bell& i, const change& c);

  char *print(char *pn) const;	// Print place notation to a string
  string print() const;
  int bells(void) const { return n; } // Return number of bells
  int sign(void) const;		// Return whether it's odd or even
  int findswap(bell which) const; // Check whether a particular swap is done
  int findplace(bell which) const; // Check whether a particular place is made
  int swappair(bell which);		// Swap or unswap a pair
  int internal(void) const;	// Does it contain internal places?
  int count_places(void) const; // Count the number of places made

  // So that we can put changes into containers
  bool operator<(const change& c) const {
    return (n < c.n) || (n == c.n && swaps < c.swaps);
  }
  bool operator>(const change& c) const {
    return (n > c.n) || (n == c.n && swaps > c.swaps);
  }

  struct invalid : public invalid_argument {
    invalid();
  };

};

inline RINGING_API ostream& operator<<(ostream& o, const change& c) {
  return o << c.print();
}

// Apply a change to a position
RINGING_API bell& operator*=(bell& i, const change& c); 
inline RINGING_API bell operator*(bell i, const change& c)
{
  bell j = i; j *= c; return j;
}

// Take the place notation between start and finish, and send it as
// a sequence of changes to out.
template<class OutputIterator, class ForwardIterator>
void interpret_pn(int num, ForwardIterator i, ForwardIterator finish, 
		  OutputIterator out)
{
  ForwardIterator j;
  bool symblock;
  change cross = change(num,"X");
  while(i != finish) {
    list<change> block;
    // Skip rubbish
    while(i != finish && !(isalnum(*i) || *i == '&' || *i == '-')) ++i;
    // See whether it's a symmetrical block or not
    if(i == finish) return;
    symblock = (*i == '&'); if(symblock) ++i;
    while(i != finish && *i == '.') ++i;
    while(i != finish && (isalnum(*i) || *i == '-')) {
      // Get a change
      if(*i == 'X' || *i == 'x' || *i == '-') {
	++i;
	block.push_back(cross);
      }
      else {
	j = i;
	while(j != finish && isalnum(*j) && *j != 'X' && *j != 'x') ++j;
	if(j != i) block.push_back(change(num, string(i, j)));
	i = j;
      }
      while(i != finish && *i == '.') ++i;
      // Skip any other rubbish
      while(i != finish && !(isalnum(*i) || *i == '&' || *i == '-' || *i == '.' || *i == ',')) ++i;
    }
    // Now output the block
    copy(block.begin(), block.end(), out);
    if(symblock) { 
      block.pop_back(); 
      copy(block.rbegin(), block.rend(), out);
    }
  }
}

// row : This stores one row 
class RINGING_API row {
private:
  vector<bell> data;	        // The actual row

public:
  row() {}
  explicit row(int num);	// Construct rounds on n bells
  row(const char *s);			// Construct a row from a string
  // Use default copy constructor and copy assignment

  row& operator=(const char *s);	// Assign a string
  int operator==(const row& r) const; // Compare
  int operator!=(const row& r) const
    { return !(*this == r); }
  bell operator[](int i) const	// Return one particular bell (not an lvalue).
    { return data[i]; }
  row operator*(const row& r) const; // Transpose one row by another
  row& operator*=(const row& r);
  row operator/(const row& r) const; // Inverse of transposition
  row& operator/=(const row& r);
  friend RINGING_API row& operator*=(row& r, const change& c); // Apply a change to a row
  row operator*(const change& c) const;	
  row inverse(void) const;	// Find the inverse

  char *print(char *s) const;	// Print the row into a string
  string print() const;
  int bells(void) const { return data.size(); } // How many bells?
  row& rounds(void);		// Set it to rounds

  static row rounds(const int n) { return row(n); }	// Return rounds on n bells

  static row queens(const int n); // Return queens on n bells
  static row kings(const int n); // Return kings on n bells
  static row titums(const int n); // Return titums on n bells
  static row reverse_rounds(const int n); // Return reverse rounds on n bells

  static row pblh(int n, int h=1); // Return first plain bob lead head on 
                                // n bells with h hunt bells
  int isrounds(void) const;	// Is it rounds?
  int ispblh(void) const;	// Which plain bob lead head is it?
  int sign(void) const;         // Return whether it's odd or even
  char *cycles(char *result) const; // Express it as a product of disjoint cycles
  int order(void) const;	    // Return the order
  friend RINGING_API ostream& operator<<(ostream&, const row&);
  void swap(row &other) { data.swap(other.data); }
  
  struct invalid : public invalid_argument {
    invalid();
  };

  // So that we can put rows in containers
  bool operator<(const row& r) const { return data < r.data; }
  bool operator>(const row& r) const { return data > r.data; }
};

inline ostream& operator<<(ostream& o, const row& r) {
  return o << r.print();
}

// An operator which has to be here
RINGING_API row& operator*=(row& r, const change& c);

struct RINGING_API permuter
{
  typedef row result_type;
  explicit permuter(unsigned n) : r(row::rounds(n)) {}

  const row &operator()(const change &c) { return r *= c; }
  const row &operator()(const row &c) { return r *= c; }

private:
  row r;
};

struct RINGING_API row_permuter
{
  typedef row result_type;
  explicit row_permuter(row &r) : r(r) {}  

  const row &operator()(const change &c) { return r *= c; }
  const row &operator()(const row &c) { return r *= c; }

private:
  row &r;
};

inline RINGING_API permuter permute(unsigned n) { return permuter(n); }
inline RINGING_API row_permuter permute(row &r) { return row_permuter(r); }

#if RINGING_AS_DLL
RINGING_EXPLICIT_STL_TEMPLATE vector<row>;
RINGING_EXPLICIT_STL_TEMPLATE vector<change>;
#endif

// row_block : Stores some rows, along with a pointer to some
// changes from which they can be calculated.
class RINGING_API row_block : public vector<row> {
private:
  const vector<change>& ch;	  // The changes which these rows are based on

public:
  row_block(const vector<change> &c);	         // Starting from rounds
  row_block(const vector<change> &c, const row &r); // Starting from the given row

  row& set_start(const row& r)	// Set the first row
    { (*this)[0] = r; return (*this)[0]; }
  row_block& recalculate(int start = 0); // Recalculate rows from changes
  const vector<change>& get_changes(void) const // Return the changes which we are using
    { return ch; }
};
      
RINGING_END_NAMESPACE

// specialise std::swap
RINGING_DELEGATE_STD_SWAP( row )
RINGING_DELEGATE_STD_SWAP( change )

#endif
