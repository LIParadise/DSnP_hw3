/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cctype>      // toupper;
#include <limits>      // numeric_limits<size_t>::max();
#include "util.h"
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
  bool
CmdParser::openDofile(const string& dof)
{
  // TODO... done 10/18 23:01

  if( _dofileStack.size() < MAX_FILE_DEPTH ){
    // try open file.
    if( _dofile != nullptr ){
      _dofileStack.push( _dofile );
    }
    _dofile = new ifstream(dof.c_str());
    if( _dofile->is_open() ){
      if( _dofile->good() ){
        return true;
      }
    }
    // file openning failure...
    _dofile->close();
    delete _dofile;
    if( !_dofileStack.empty() ){
      _dofile = _dofileStack.top();
      _dofileStack.pop();
    }else{
      _dofile = nullptr;
    }
    return false;
  }else{
    // _dofileStack.size() >= MAX_FILE_DEPTH;
    cerr << "well how am i supposed to close this shit?" << endl;
    return false;
  }
}

// Must make sure _dofile != 0
  void
CmdParser::closeDofile()
{
  assert(_dofile != 0);
  // TODO... done 10/18 21:12
  _dofile->close();
  delete _dofile;
  if( !_dofileStack.empty() ){
    _dofile = _dofileStack.top();
    _dofileStack.pop();
  }else{
    _dofile = nullptr;
  }
}

// Return false if registration fails
  bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
  // Make sure cmd hasn't been registered and won't cause ambiguity
  string str = cmd;
  unsigned s = str.size();
  if (s < nCmp) return false;
  while (true) {
    if (getCmd(str)) return false;
    if (s == nCmp) break;
    str.resize(--s);
  }

  // Change the first nCmp characters to upper case to facilitate
  //    case-insensitive comparison later.
  // The strings stored in _cmdMap are all upper case
  //
  assert(str.size() == nCmp);  // str is now mandCmd
  string& mandCmd = str;
  for (unsigned i = 0; i < nCmp; ++i)
    mandCmd[i] = toupper(mandCmd[i]);
  string optCmd = cmd.substr(nCmp);
  assert(e != 0);
  e->setOptCmd(optCmd);

  // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
  return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
  CmdExecStatus
CmdParser::execOneCmd()
{
  bool newCmd = false;
  if (_dofile != 0)
    newCmd = readCmd(*_dofile);
  else
    newCmd = readCmd(cin);

  // execute the command
  if (newCmd) {
    string option;
    CmdExec* e = parseCmd(option);
    if (e != 0)
      return e->exec(option);
  }
  return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
  // TODO... done 10/18 21:17
  for( auto it = _cmdMap.begin(); it!= _cmdMap.end(); it++ ){
    it->second->help();
  }
  cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
  assert(_tempCmdStored == false);
  if (_history.empty()) {
    cout << "Empty command history!!" << endl;
    return;
  }
  int s = _history.size();
  if ((nPrint < 0) || (nPrint > s))
    nPrint = s;
  for (int i = s - nPrint; i < s; ++i)
    cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
  CmdExec*
CmdParser::parseCmd(string& option)
{
  assert(_tempCmdStored == false);
  assert(!_history.empty());
  string str = _history.back();


  // TODO... done 10/19 00:48;
  assert(str[0] != 0 && str[0] != ' ');

  string first_word = "";
  size_t start_pos_of_trailing = 0;
  start_pos_of_trailing = myStrGetTok( str, first_word );
  CmdExec* ptr = getCmd( first_word );
  if( start_pos_of_trailing == string::npos ){
    option = "";
  }else{
    option = str.substr( start_pos_of_trailing, string::npos );
  }

  if( ptr == 0 ){
    cerr << "Illegal command!! \"(" << first_word << ")\"" << endl;
    return NULL;
  }else{
    return ptr;
  }
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is THE PARTIAL STRING BEFORE CURRENT CURSOR POSITION. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    Considering the following cases in which prefix is empty:
//    --- 6.1.1 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and they do not have a common prefix,
//    cmd> help $sdfgh
//    [After] print all the file names
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.1.2 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and all of them have a common prefix,
//    cmd> help $orld
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help mydb-$orld
//    --- 6.1.3 ---
//    [Before] if prefix is empty, and only one file in the current directory
//    cmd> help $ydb
//    [After] print out the single file name followed by a ' '
//    cmd> help mydb $
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

  void
CmdParser::listCmd(const string& str)
{
  // TODO... done 10/21 0608
  // we'll do this function based on cmdMgr->_tabPressCount
  // along with # of tokens in current command line.

  string temp  = "";
  size_t index = 0;
  size_t tmp   = 0;

  // case I. there's no token.
  if( myStrGetTok( str, temp ) == string::npos && temp == "" ){
    cout << endl;
    printHelps();
    _myTabPressCount = 0;
    reprintCmd();
    return;
  }
  // End of case I.

  // case II. there's only one token.
  temp = "";
  if( myStrGetTok( str, temp ) == string::npos && temp != "" ){
    temp_cmd_pairs.clear();
    string standard = "";
    
    for( auto it = _cmdMap.begin(); it != _cmdMap.end(); ++it ){
      standard = "";
      standard = it->first + it->second->getOptCmd();
      if( cmp_str_ign_case( standard, temp, temp.size() ) ){
        // match cmd;
        temp_cmd_pairs.push_back( (*it) );
      }
    }

    if( temp_cmd_pairs.empty() ){
      // token could not match any of registered command,
      // beep and do nothing.
      mybeep();
      _myTabPressCount = 0;
      return;
    }else if( temp_cmd_pairs.size() == 1 ){
      // token is good, and only one match.
      standard = "";
      standard = temp_cmd_pairs.at(0).first + 
        temp_cmd_pairs.at(0).second->getOptCmd();
      assert( temp.size() <= standard.size() );
      for( size_t i = temp.size() ; i < standard.size() ; ++i ){
        insertChar( standard[i] );
      }
      insertChar( ' ' );
      _myTabPressCount = 0;
      return;
    }else{
      // temp_cmd_pairs.size() > 1;
      ios_base::fmtflags origFlags = cout.flags();
      standard = "";
      size_t i = 0;
      for ( ; i < temp_cmd_pairs.size(); ++i) {
        if( i%5 == 0 ){
          cout << endl;
        }
        standard = temp_cmd_pairs[i].first+temp_cmd_pairs[i].second->getOptCmd();
        cout << setw(12) << left << standard;
      }
      cout.flags(origFlags);
      reprintCmd();
      _myTabPressCount = 0;
      // since there's only one token and there's no delimiter before cursor,
      // we shall remain _myTabPressCount = 0 for future 'usage()' calling.
    }
    return;
  }// end of case II.

  // case III, there's at least one token and a tailing space.
  // maybe first token correspond to an exact command, maybe not.
  // maybe first token matches full name of that command, maybe not.
  temp = "";
  index = myStrGetTok( str, temp );
  if( index != string::npos ){
    temp_cmd_pairs.clear();
    string neighbor_tok = "";
    string standard = "";
    string _prefix = "";
    // make neighbor_tok the last token right before cursor.
    tmp = str.size()-1;
    for( ; str[tmp] != ' ' && tmp != 0; tmp -- ){}
    neighbor_tok = str.substr( tmp+1, string::npos );
    // neighbor_tok is the last token before cursor.
    vector<string> filenames;
    
    for( auto it = _cmdMap.begin(); it != _cmdMap.end(); ++it ){
      standard = "";
      standard = it->first + it->second->getOptCmd();
      if( cmp_str_ign_case( standard, temp, it->first.size() ) ){
        // match cmd;
        temp_cmd_pairs.push_back( (*it) );
      }
    }

    if( temp_cmd_pairs.empty() ){
      // leading token doesn't mean any command;
      mybeep();
      _myTabPressCount = 0;
      return;
    }else if( temp_cmd_pairs.size() == 1 ){
      // there's only one command like this;
      if( _myTabPressCount == 0 ){
        cout << endl;
        temp_cmd_pairs[0].second->usage(cout);
        _myTabPressCount = 1;
        reprintCmd();
        return;
      }else{
        // _myTabPressCount >= 1;
        assert( _myTabPressCount == 1  &&
            "_myTabPressCount error in case III" );
        if( neighbor_tok == "" ){
          // command line like "cmd> user_input $" and pressed tab again,
          // where '$' means cursor.
          // there shall be usage message of this command printed before.
          // try ls or show common prefix.
          filenames.clear();
          listDir( filenames, "", "." );
          if( filenames.empty() ){
            mybeep();
            _myTabPressCount = 1;
            return;
          }else if( filenames.size() > 1 ){
            _prefix = myFindPrefix( filenames );
            if( !_prefix.empty() ){
              for( auto& it : _prefix ){
                insertChar( it );
              }
              mybeep();
              _myTabPressCount = 1;
              return;
            }else{
              // no prefix, ls.
              ios_base::fmtflags origFlags = cout.flags();
              for( size_t i = 0; i < filenames.size(); ++i ){
                if( i%5 == 0 ){
                  cout << endl;
                }
                cout << setw(16) << left << filenames[i];
              }
              cout.flags(origFlags);
              _myTabPressCount = 1;
              reprintCmd();
              return;
            }

          }else if( filenames.size () == 1 ){
            for( auto& it : filenames[0] ){
              insertChar( it );
            }
            insertChar ( ' ' );
            _myTabPressCount = 1;
          } // end of if ( filenames.empty() ) elif {} elif {};
          // end of case "neighbor_tok.empty()";
        }else if( neighbor_tok != "" ){
          filenames.clear();
          listDir( filenames, neighbor_tok, "." );
          if( filenames.empty() ){
            mybeep();
            _myTabPressCount = 1;
            return;
          }else if( filenames.size() > 1 ){
            _prefix = myFindPrefix( filenames );
            if( !_prefix.empty() ){
              if( _prefix.size() > neighbor_tok.size() ){
                for( size_t i = neighbor_tok.size(); i < _prefix.size(); ++i ){
                  insertChar( _prefix[i] );
                }
                mybeep();
                _myTabPressCount = 1;
                return;
              }else{
                assert( _prefix.size() == neighbor_tok.size() );
                ios_base::fmtflags origFlags = cout.flags();
                for( size_t i = 0; i < filenames.size(); ++i ){
                  if( i%5 == 0 ){
                    cout << endl;
                  }
                  cout << setw(16) << left << filenames[i];
                }
                cout.flags(origFlags);
                _myTabPressCount = 1;
                reprintCmd();
                return;
              }
            }else{
              // no prefix, mybeep() and do nothing.
              // but this shall not happen, given that we
              // used 'neighbor_tok' as prefix for 
              // listDir(vector<string>&, const string&, const string& = ".")
              throw ( "un-explain-able error: \"" + neighbor_tok + "\"" + 
                  "inside listCmd()" );
              return;
            }
          }else if( filenames.size () == 1 ){
            for( size_t i = neighbor_tok.size(); i < filenames[0].size(); ++i){
              insertChar( filenames[0][i] );
            }
            insertChar ( ' ' );
            _myTabPressCount = 1;
            return;
          } // end of if ( filenames.empty() ) elif {} elif {};
        }// end of checking neighbor_tok;
      }// end of temp_cmd_pairs.size() == 1, _myTabPressCount > 1;
    }else{
      // temp_cmd_pairs.size() > 1;
      mybeep();
      _myTabPressCount = 0;
    }
  }// end of case III., with _myTabPressCount > 1;
    

}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
  CmdExec*
CmdParser::getCmd(string cmd)
{
  CmdExec* e = 0;

  string first_word = "";
  string standard = "";
  size_t start_of_non_mand = myStrGetTok( cmd, first_word );

  for( auto it = _cmdMap.begin();
      it != _cmdMap.end(); ++ it ){

    standard = it->first + it->second->getOptCmd() ;
    if( ! myStrNCmp( standard, cmd, it->first.size() ) ){
      // match.
      e = it->second;
      break;
    }

  }// end of for loop;

  // TODO... done 10/19 05:01
  return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
  string err;
  myStrGetTok(option, err);
  if (err.size()) {
    errorOption(CMD_OPT_EXTRA, err);
    return false;
  }
  return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
  size_t n = myStrGetTok(option, token);
  if (!optional) {
    if (token.size() == 0) {
      errorOption(CMD_OPT_MISSING, "");
      return false;
    }
  }
  if (n != string::npos) {
    errorOption(CMD_OPT_EXTRA, option.substr(n));
    return false;
  }
  return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
  string token;
  size_t n = myStrGetTok(option, token);
  while (token.size()) {
    tokens.push_back(token);
    n = myStrGetTok(option, token, n);
  }
  if (nOpts != 0) {
    if (tokens.size() < nOpts) {
      errorOption(CMD_OPT_MISSING, "");
      return false;
    }
    if (tokens.size() > nOpts) {
      errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
      return false;
    }
  }
  return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
  switch (err) {
    case CMD_OPT_MISSING:
      cerr << "Error: Missing option";
      if (opt.size()) cerr << " after (" << opt << ")";
      cerr << "!!" << endl;
      break;
    case CMD_OPT_EXTRA:
      cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
    case CMD_OPT_ILLEGAL:
      cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
    case CMD_OPT_FOPEN_FAIL:
      cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
    default:
      cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
  }
  return CMD_EXEC_ERROR;
}

bool
CmdParser::cmp_str_ign_case( const string& standard, const string& user_input,
    const size_t size ) const {
  char ch1, ch2;
  for( size_t i = 0; i < size; i ++ ){
    ch1 = standard[i];
    ch2 = user_input[i];
    ch1 = toupper( ch1 );
    ch2 = toupper( ch2 );
    if( ch1 != ch2 ){
      return false;
    }
  }
  return true;
}

string
CmdParser::myFindPrefix( const vector<string>& ls ) const{
  if( ls.size() == 1 ){
    return ls[0];
  }else if( ls.empty() ){
    return "";
  }
  // we only need to find prefix for ls have size > 1;

  size_t len = numeric_limits<size_t>::max();
  for( size_t i = 0; i < ls.size(); ++i ){
    if( ls[i].size() < len ){
      len = ls[i].size();
    }
  }

  size_t match_len = 0;
  size_t temp = 0;
  size_t offset = 0;
  string former = ls[0].substr( 0, len/2 );
  string latter = ls[0].substr( len/2, string::npos );
  string _prefix = "";
  bool ending = false;

  while( true ){

    if( former.size() == 0 ){
      assert( latter.size() == 1 && 
          "something went wrong in binary prefix searching final steps" );
      ending = true;
      former = latter;
    }
    match_len = former.size();
    for( size_t i = 1; i < ls.size(); i++ ){
      temp = myStrPrefCmp( ls[i], former, offset );
      if( temp < match_len ){
        match_len = temp;
      }
    }
    if( match_len != former.size() ){
      former.resize( match_len );
      return ( _prefix + former );
    }else if( former.size() == 1 ){
      assert( ending == true &&
          "something went wrong in binary prefix searching final steps" );
      return ( _prefix + former );
    }else{
      // whole former match all that in vector<string> ls;
      // parse latter.
      assert( ending == false &&
          "something went wrong in binary prefix searching" );
      _prefix = _prefix + former;
      offset = len/2;
      former = latter.substr( 0, latter.size()/2 );
      latter = latter.substr( latter.size()/2, string::npos );
    }
  }
}


// compare two string case-SENSITIVELY and return the first position of which
// they don't agree with the other.
size_t
CmdParser::myStrPrefCmp( const string& standard, const string& prf, 
    const size_t pos = 0) const{
  if( standard.size() < pos ){
    assert( 0 && "inproper use of myStrPrefCmp" );
  }
  size_t size = ( (standard.size()-pos) < prf.size() )?
    standard.size()-pos: prf.size();
  for( size_t i = 0; i < size-pos; i++ ){
    if( standard[i+pos] != prf[i] ){
      return i;
    }
  }
  if( pos + prf.size() > standard.size() ){
    return standard.size() - pos;
  }else{
    return prf.size();
  }
}
