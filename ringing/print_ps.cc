// print_ps.cc - PostScript printing stuff

#ifdef __GNUG__
#pragma implementation
#endif

#include "print_ps.h"

const char *printpage_ps::def_string =
"/BD {bind def} bind def
/C {dup stringwidth pop 2 div neg offset neg rmoveto show} BD
/TR {dup stringwidth pop neg 4 -1 roll add 3 -1 roll moveto show} BD
/TC {dup stringwidth pop 2 div neg 4 -1 roll add 3 -1 roll moveto show} BD
/TL {3 1 roll moveto show} BD
/F {exch findfont exch scalefont setfont} BD
/O {
  newpath 
  xspace mul xstart add ypos yspace add 
  3 -1 roll 2 mul 0 360 arc closepath fill
} BD
/D {0 offset neg rmoveto show} BD
/DR {dup stringwidth pop 2 div neg offset neg rmoveto show} BD
/E {currentpoint gsave translate 0.8 1 scale 0 0 M C grestore} BD
/PB {
  dup 20 div setlinewidth
  exch 1 add xspace mul xstart add ypos yspace add 
  newpath 2 copy 5 -1 roll 2 mul 3 div 0 360 arc stroke
  moveto
} BD 
/MR {xspace mul xstart add ypos yspace add moveto} BD
/M {moveto} BD
/L {lineto} BD
/R {rlineto} BD
/S {stroke} BD
/N {newpath} BD
/SL {setlinewidth} BD
/buf 1 string def
/W { 
   /xpos xstart def
   {
      buf dup 0 4 -1 roll put
      xpos dup xspace add /xpos exch def ypos M C
   } forall
   /ypos ypos yspace sub def
} BD
/RO {
  0 SL N
  xstart xspace 2 div sub ypos yspace 2 div add M
  xspace mul 0 R S
} BD
/X {/xstart exch def} BD
/Y {/ypos exch def} BD
/DL1 { 1 exch { dup 49 ge exch dup 57 le 3 -1 roll 
and { 48 sub exch pop }
{ 2 index exch get exch { dup dup type /stringtype eq
{ 2 index exch DL1 pop } { exec } ifelse } repeat pop 1 } ifelse }
forall pop } BD 
/DL { DLD exch DL1 pop } BD
/DLD 4 dict def
/defline { exch 0 get DLD 3 1 roll exch put } BD
(u) { xspace yspace neg R } defline
(d) { xspace neg yspace neg R } defline
(p) { 0 yspace neg R } defline
/J { yspace mul neg exch xspace mul xstart add 
currentpoint pop neg add exch rmoveto } BD
";

printpage_ps::printpage_ps(ostream& o) : os(o), eps(false)
{
os << "%!PS-Adobe 2.0\n"
<< "%%Creator: Method Class Library\n"
<< "%%EndComments\n\n"
<< def_string << "%%Page: 1\n";
}

printpage_ps::printpage_ps(ostream& o, int x0, int y0, int x1, int y1)
  : os(o), eps(true)
{
  os << "%!PS-Adobe 2.0 EPSF\n"
     << "%%BoundingBox: " << x0 << ' ' << y0 
     << ' ' << x1 << ' ' << y1
     << "\n%%Creator: Method Class Library\n"
     << "%%Pages: 0\n"
     << "%%EndComments\n\n"
     << "save\n" << def_string;
}

void printpage_ps::set_text_style(const text_style& s)
{
  os << '/' << s.font << ' ' << s.size << " F\n";
}

// NOTE: At the moment this is quite likely to break when printing
// unusual types of characters.  Putting more left brackets than right
// brackets in the string will _really_ confuse it.  This should be
// put right by somebody who has a PostScript manual.
void printpage_ps::text(const string t, const dimension& x, 
			const dimension& y, text_style::alignment al, 
			const text_style& s)
{
  set_text_style(s);
  os << x.in_points() << ' ' << y.in_points() << " (";
  string::const_iterator i;
  for(i = t.begin(); i != t.end(); i++)
    if(isprint(*i)) os << *i;
  os << ") ";
  switch(al) {
    case text_style::left: os << "TL\n"; break;
    case text_style::right: os << "TR\n"; break;
    case text_style::centre: os << "TC\n"; break;
  }
}

// This function prints a row: it actually writes the code to print
// the numbers, and adds the relevant bit to the end of each of the lines
void printpage_ps::printrow_ps::print(const row& r)
{
  // Start a new column if we need to
  if(!in_column) start_column();

  // Print the row
  if(opt.flags & printrow::options::numbers) {
    os << '(';
    for(int j = 0; j < r.bells(); j++) 
      if((opt.flags & printrow::options::miss_numbers) && has_line(r[j]))
	os << ' ';
      else
	os << row::b_to_c(r[j]);
    os << ") W\n";
  }
  lastrow = r;

  // Add the various bits of lines to the end of the line
  list<drawline>::iterator i;
  for(i = drawlines.begin(); i != drawlines.end(); i++)
    (*i).add(r);
}

void printpage_ps::printrow_ps::rule()
{
  if(!in_column) return;
  os << lastrow.bells() << " RO\n";
}

void printpage_ps::printrow_ps::set_position(const dimension& x, const dimension& y)
{
  if(in_column) end_column();
  currx = (int)x.in_points();
  curry = (int)y.in_points();
}

void printpage_ps::printrow_ps::new_column(const dimension& d)
{
  if(in_column) end_column();
  currx += (int)d.in_points();
}

void printpage_ps::printrow_ps::start()
{
  // Select the font we'll be using
  os << '/' << opt.style.font << ' ' << opt.style.size << " F\n";
  // Define xspace and yspace
  os << "/xspace " << opt.xspace.in_points() << " def /yspace "
     << opt.yspace.in_points() << " def\n";
  // Find the height of an X
  os << "gsave N 0 0 M (X) true charpath pathbbox grestore\n"
     << "exch pop sub 2 div neg /offset exch def pop\n";
}

void printpage_ps::printrow_ps::start_column()
{
  map<int, printrow::options::line_style>::iterator i;
  for(i = opt.lines.begin(); i != opt.lines.end(); i++)
    drawlines.push_back(drawline(*this, (*i).first, (*i).second));
  os << currx << " X " << curry << " Y\n";
  in_column = true;
}

void printpage_ps::printrow_ps::end_column()
{
  list<drawline>::iterator i;
  for(i = drawlines.begin(); i != drawlines.end(); i++)
    (*i).output(os, currx, curry);
  drawlines.erase(drawlines.begin(), drawlines.end());
  in_column = false;
}

void printpage_ps::printrow_ps::dot(int i)
{
  if(i == -1) {
    map<int, printrow::options::line_style>::const_iterator j;
    for(j = opt.lines.begin(); j != opt.lines.end(); j++)
      if((*j).first >= 0) dot((*j).first);
  } else {
    int j = 0;
    while(j < lastrow.bells() && lastrow[j] != i) j++;
    if(j < lastrow.bells()) {
      map<int, printrow::options::line_style>::const_iterator k;
      k = opt.lines.find(i);
      if(k != opt.lines.end())
	os << "0 SL " << (*k).second.width.in_points() 
	   << ' ' << i << " O\n";
    }
  }
}

void printpage_ps::printrow_ps::placebell(int i)
{
  int j = 0;
  while(j < lastrow.bells() && lastrow[j] != i) j++;
  if(j < lastrow.bells()) {
    os << lastrow.bells() << ' ' << opt.style.size << " PB (" 
       << j+1 << ((j < 9) ? ") C\n" : ") E\n");
  }
}

void printpage_ps::printrow_ps::text(const string& t, const dimension& x, 
				     text_style::alignment al,
				     bool between, bool right)
{
  if(right) os << lastrow.bells(); else os << '0';
  os << " MR ";
  if(right) os << x.in_points(); else os << -(x.in_points());
  if(between) os << " yspace 2 div neg "; else os << " 0 ";
  os << " R (";
  string::const_iterator i;
  for(i = t.begin(); i != t.end(); i++)
    if(isprint(*i)) os << *i;
  os << ") ";
  switch(al) {
    case text_style::left: os << "D\n"; break;
    case text_style::right: os << "DR\n"; break;
    case text_style::centre: os << "C\n"; break;
  }
}

void printpage_ps::printrow_ps::drawline::add(const row& r)
{
  int j, b;
  b = (bell == -1) ? 0 : bell;
  for(j = 0; j < r.bells() && r[j] != b; j++);
  if(j == r.bells()) j = -1;
  if(curr == -1)
    l.push_back(j);
  else {
    if(j != -1) {
      if(bell != -1 || (j != curr && p.has_line(r[curr]))) {
	if(j == curr) l.push_back(-3);
	else if(j == curr - 1) l.push_back(-4);
	else if(j == curr + 1) l.push_back(-2);
	else l.push_back(j);
      } else l.push_back(j);
    } else
      l.push_back(-1);
  }
  curr = j;
}

void printpage_ps::printrow_ps::drawline::output(ostream& o, int x, int y)
{
  list<int>::iterator i;
  int t, count;
  bool in_line = false;
  i = l.begin(); count = -1;
  o << s.width.in_points() << " SL N " << x << ' ' << y << " M\n";
  while(i != l.end()) {
    t = (*i);
    while(i != l.end() && ((t >= -1 && *i >= -1) || (t == *i))) 
	{ t = *i; i++; count++; }
    if(count > 1 && t <= -2) o << count;
    if(t <= -2 && !in_line) { o << '('; in_line = true; }
    switch(t) {
      case -2: o << 'u'; break;
      case -4: o << 'd'; break;
      case -3: o << 'p'; break;
      default: 
	if(in_line) { o << ") DL "; in_line = false; } 
	o << t << ' ' << count << " J "; 
	break;
    }
    count = 0;
  }
  if(in_line) o << ") DL ";
  o << "S\n";
}
