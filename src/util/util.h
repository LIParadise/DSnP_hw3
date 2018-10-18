/****************************************************************************
  FileName     [ util.h ]
  PackageName  [ util ]
  Synopsis     [ Define the prototypes of the exported utility functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include <istream>
#include <vector>

using namespace std;

// In myString.cpp
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool myStr2Int(const string& str, int& num);
extern bool isValidVarName(const string& str);

// In myGetChar.cpp
extern char myGetChar(istream&);
extern char myGetChar();

// In util.cpp
extern int listDir(vector<string>&, const string&, const string&);

/*
--listDir:
  -dir: 
  the directory we want to do "ls", either absolute path or "." or ".."
  shall do the trick.
  -files:
  all the files listed would be stored here.
  -prefix:
  act as filter.
  only files that have its name starting with prefix would be in the vector.

--myStrNCmp:
  s1 is the formal command name given in document,
  s2 being user input,
  n being mandatory part of s1;

  if s2 is overall good, return true.

--myStrGetTok:
  try to get the first token and store it in "tok" from "str[pos]";
  if there's no token to take, return value would string::npos;
  if there's more characters in str, including delimiter, it's a valid size_t.
  if there's nothing left, return value would also be string::npos.

--myStr2Int:
  Convert string "str" to integer "num". 
  Return false if str does not appear to be a number.

--isValidVarName:
  this version does not match that described in doc.
  DON'T use it to determine if key-value pair of json object is valid!!
*/

#endif // UTIL_H
