/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream&
operator << (ostream& os, const DBJsonElem& j)
{
  os << "\"" << j._key << "\" : " << j._value;
  return os;
}

istream& operator >> (istream& is, DBJson& j)
{
  // TODO: to read in data from Json file and store them in a DB 
  // - You can assume the input file is with correct JSON file format
  // - NO NEED to handle error file format
  assert(j._obj.empty());

  string temp_str = "";
  is.seekg( 0, is.beg );

  while( getline( is, temp_str ) ){ // string::getline
    if( temp_str.find_first_of( "{" ) != string::npos )
      break;
  }

  temp_str = "";
  string key = "";
  string value = "";
  int    value_data = 0;
  bool   valid_line = true;
  size_t start_quot_mark = 0;
  size_t end_quot_mark   = 0;
  size_t colon_mark      = 0;
  size_t start_value     = 0;
  size_t end_value       = 0;

  while( getline( is, temp_str ) ){ // parsing, return fail or throw exception if bad;

    valid_line = true;
    // if (!valid_line), then this->_obj would not be modified.

    if( temp_str.find_first_of( "}" ) != string::npos ){
      break;
    }

    start_quot_mark = temp_str.find_first_of( "\"" );
    end_quot_mark   = temp_str.find_first_of( "\"", start_quot_mark+1 );

    if( start_quot_mark != string::npos && end_quot_mark != string::npos ){
      key = temp_str.substr( start_quot_mark+1, end_quot_mark-start_quot_mark-1 );
    }else{
      valid_line = false;
    }

    colon_mark = temp_str.find_first_of ( ":",  end_quot_mark+1 );

    if( colon_mark != string::npos ){
      start_value = temp_str.find_first_of( "-0123456789", colon_mark+1 );
      // maybe there's only zero as the value...
      end_value = temp_str.find_first_not_of( "0123456789", start_value+1 );
      if( start_value != string::npos && end_value != string::npos ){
        // shall be normal line, with others pending, s.t. we met ',';
        value = temp_str.substr( start_value, end_value - start_value );
      }else if( start_value != string::npos && end_value == string::npos ){
        end_value = temp_str.size();
        value = temp_str.substr( start_value, end_value - start_value );
      }else{
        valid_line = false;
      }
    }else{
      valid_line = false;
    }

    if( valid_line ){
      if(myStr2Int( value, value_data )) {
        j.add( DBJsonElem( key, value_data ) );
      }
    }
  }
  return is;
}

ostream& operator << (ostream& os, const DBJson& j)
{
  // TODO ...possibly done?
  os << '{' << '\n';
  for( size_t i = 0; i < j.size(); i++ ){
    os << j[i] << '\n';
  }
  os << '}';
  return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void
DBJson::reset()
{
  // TODO ...done.
  _obj.clear();
}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
  // TODO ...done
  for( auto& it : this-> _obj ){
    if( elm.key() == it.key() ){
      return false;
    }
  }
  _obj.push_back( elm );
  return true;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
  // TODO ...done
  if( _obj.empty() ){
    return NAN;
  }else{
    return (double)(this->sum())/_obj.size();
  }
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
  // TODO ...done;
  int maxN = INT_MIN;
  if( _obj.size() == 0 ){
    idx = 0;
    return INT_MIN;
  }else{
    for( auto& it : _obj ){
      if( it.value() > maxN )
        maxN = it.value();
    }
  }
  return  maxN;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::min(size_t& idx) const
{
  // TODO ...done
  int minN = INT_MAX;
  if( _obj.size() == 0 ){
    idx = 0;
    return INT_MAX;
  }else{
    for( auto& it : _obj ){
      if( it.value() < minN )
        minN = it.value();
    }
  }
  return  minN;
}

void
DBJson::sort(const DBSortKey& s)
{
  // Sort the data according to the order of columns in 's'
  ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
  // Sort the data according to the order of columns in 's'
  ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
  // TODO ...done
  int s = 0;
  if( _obj.size() == 0 ){
    return 0;
  }else{
    for( auto& it : _obj ){
      s += it.value();
    }
    return s;
  }
}
