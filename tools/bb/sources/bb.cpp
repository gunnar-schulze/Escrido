// *****************************************************************
// *                     Blue-Footed Booby (bb)                    *
// *            command line argument scanner generator            *
// *                                                               *
// *                               by                              *
// *                         Gunnar Schulze                        *
// *                                                               *
// *****************************************************************
// *                  (c) 2013 by Gunnar Schulze                   *
// *****************************************************************
// * ToDo:                                                         *
// *                                                               *
// * - Benoetigte Laenge der Fehlerausgabe wirklich berechnen und  *
// *   nicht einfach statisch auf 128 Zeichen setzten;             *
// *****************************************************************

// *****************************************************************************
// Copyright (c) 2013 Gunnar Schulze
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// *****************************************************************************

// -----------------------------------------------------------------

// INDEPENDENT INCLUSIONS

//------------------------------------------------------------------

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>      // std::stringstream

// -----------------------------------------------------------------

// GLOBAL SETTINGS

//------------------------------------------------------------------

using namespace std;

// -----------------------------------------------------------------

// GLOBAL CONSTANTS

//------------------------------------------------------------------

const int nLineLen = 79;    // Line length set to 79 character to "look good" under windows.

// -----------------------------------------------------------------

// MAKROS

//------------------------------------------------------------------

// Typen von Argumentwerten:
#define BFB_AVT_INTEGER                                            1
#define BFB_AVT_STRING                                             2
#define BFB_AVT_FLOAT                                              3
#define BFB_AVT_ONOFF                                              4

// -----------------------------------------------------------------

// STRUKTUREN UND KLASSEN

//------------------------------------------------------------------

struct SArg
{
  string sName;                     // Bezeichner des Arguments;
  vector <signed int> faVal;        // Liste der Argumente;
  string sDesc;                     // Beschreibung;
  string sCmd;                      // Kommandoaufruf;
};

// -----------------------------------------------------------------

struct SArgScheme
{
  vector <SArg> oaArgList;          // Liste der moeglichen Argumente;
  unsigned int nFixedLastArgN;      // Anzahl fixer Argumenten am Ende der Argumentenlist;
  unsigned int nFixedFirstArgN;     // Anzahl fixer Argumenten am Ende der Argumentenlist;

  SArgScheme();
  void Clear();
};

// .................................................................

SArgScheme::SArgScheme() :
  nFixedLastArgN( 0 ),
  nFixedFirstArgN( 0 )
{}

// .................................................................

void SArgScheme::Clear()
{
  oaArgList.clear();
  nFixedLastArgN = 0;
  nFixedFirstArgN = 0;
}

// -----------------------------------------------------------------

// GLOBALE KONSTANTEN

//------------------------------------------------------------------

namespace applicationInfo
{
  const char szName[]      = "Blue-Footed Booby";
  const char szDesc[]      = "Command line argument scanner generator";
  const char szVersion[]   = "2.3.3";
  const char szFirstDate[] = "Mai 2012";
  const char szDate[]      = "January 2016";
  const char szAuthor[]    = "Gunnar Schulze";
}

// -----------------------------------------------------------------

// FUNKTIONSUEBERSICHT

//------------------------------------------------------------------

bool ReadGrammarFile( const char* szFileName_i, vector <SArgScheme>& oaSchemeList_o );
unsigned int GetMinArgN( vector <SArgScheme>& oaSchemeList_i );
bool WriteCommand( char* szCmd_i, SArg& oArg_i, const char* szPrefix_i, unsigned int nOffs_i, ofstream& oOutfile_o );
void OutputVarTypeString( signed int faVal_i, ostream& oOStream_o );
void BuildSchemeIdent( unsigned int nN_i, char* szIdent_o );
std::string& EscapeString( std::string& sString_io );
std::string SplitFirstDescrLine( std::string& sDescr_io, unsigned int nLineLen_i );
size_t Hash( const char* szString_i );

// -----------------------------------------------------------------

// IMPLEMENTATION MAIN

// -----------------------------------------------------------------

int main( int argc, char* argv[] )
{
  using namespace std;
  using namespace applicationInfo;

  // ** Startausgabe; **
  cout << szName << " " << szVersion << endl
       << szDesc << endl
       << "Copyright (C) " << szFirstDate << " - " << szDate << " " << szAuthor << endl;

  // ** Sonderfall: Lediglich Versionsausgabe; **
  if( argc == 2 )
    if( ( strcmp( argv[1], "--version" ) == 0 ) || ( strcmp( argv[1], "-V" ) == 0 ) )
      return 0;


  // ** Abbruch, falls nicht die korrekte Anzahl an Argumenten uebergeben wurde; **
  if( argc != 3 )
  {
    cout << endl
         << "ERROR: wrong number of arguments" << endl
         << "  To use bfb type something like:" << endl
         << "    #./bb arglist.bb outfile.h" << endl
         << "  where arglist.bb is an existing and valid argument list file," << endl
         << "  and outfile.h is the name of the target file that receives the" << endl
         << "  argument scanner." << endl
         << endl;
    return -1;
  }

  // ** Grammatik-Datei einlesen; **
  vector <SArgScheme> oaArgSchemeList;
  if( !ReadGrammarFile( argv[1], oaArgSchemeList ) )
    return -1;

  // ** Berechne minimal noetige Anzahl an Argumenteintraegen; **
  unsigned int nMinArgN = GetMinArgN( oaArgSchemeList );

  // ** Erzeuge Ausgabedatei; **
  ofstream oOutFile( argv[2] );
  if( !oOutFile.good() )
  {
    cout << endl
         << "ERROR: unable to create output file" << endl
         << "  The output file \"" << argv[2] << "\" cannot be created." << endl
         << endl;
    return -1;
  }
  else
  {
    // ** Den Dateikopf erzeugen; **
    oOutFile << "// *****************************************************************************" << endl
             << "// *                      C Command Line Argument Scanner                      *" << endl
             << "// *****************************************************************************" << endl
             << endl
             << "// This C command line scanner was generated automatically by the" << endl
             << "// Blue-Footed Booby (bb) command line argument scanner generator." << endl
             << "//" << endl
             << "// " << szName << " " << szVersion << endl
             << "// " << "(C) " << szDate << " " << szAuthor << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "// INCLUSIONS" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "#include <cstring>" << endl
             << "#include <cstdlib>" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "// FUNCTION OVERVIEW" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "namespace argscan" << endl
             << "{" << endl
             << "  // Size of error message C string (zero-terminated array of characters):" << endl
             << "  const size_t max_len_error_msg = 128;" << endl
             << endl
             << "  // Functions for public use:" << endl
             << "  bool InterpArgs( int argc, char* argv[] );" << endl
             << "  bool InterpArgs( int argc, char* argv[], char* szErrMsg_o );" << endl
             << "  bool PrintCmdLineHelp( const char* szProgName );" << endl
             << endl
             << "  // Helper functions:" << endl
             << "  bool Error( const char* szTxt, char* szErrMsg_o );" << endl
             << "  size_t Hash( const char* szString_i );" << endl
             << "}" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "// FUNCTION IMPLEMENTATION" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "bool argscan::InterpArgs( int argc, char* argv[] )" << endl
             << "{" << endl
             << "  return InterpArgs( argc, argv, NULL );" << endl
             << "}" << endl
             << endl
             << "// -----------------------------------------------------------------------------" << endl
             << endl
             << "bool argscan::InterpArgs( int argc, char* argv[], char* szErrMsg_o )" << endl
             << "{" << endl
             << "  using namespace argscan;" << endl
             << endl
             << "  size_t* naHashList = (size_t*) std::malloc( sizeof( size_t ) * argc );" << endl
             << "  if( naHashList == NULL && argc > 0 )" << endl
             << "    return Error( \"error on memory allocation for arguments hash\", szErrMsg_o );" << endl
             << "  for( unsigned int n = 1; n < argc; n++ )" << endl
             << "    naHashList[n] = Hash( argv[n] );" << endl
             << "  if( argc < " << ( 1 + nMinArgN ) << " )" << endl
             << "  {" << endl
             << "    if( naHashList != NULL )" << endl
             << "      std::free( naHashList );" << endl
             << "    return Error( \"too few arguments - at least " << nMinArgN << " argument(s) are required\", szErrMsg_o );" << endl
             << "  }" << endl
             << "  signed int nWrongArg = -1;" << endl;

    // ** Ausgabe; **
    cout << endl
         << "-> Output file \"" << argv[2] << "\" created." << endl;
  }

  // ** Alle Argumentschemata durchlaufen; **
  for( unsigned int s = 0; s < oaArgSchemeList.size(); s++ )
  {
    oOutFile << "  if( argc >= " << ( 1 + oaArgSchemeList[s].nFixedFirstArgN + oaArgSchemeList[s].nFixedLastArgN ) << " )" << endl
             << "  {" << endl
             << "    bool fOptArgOk = true;" << endl
             << "    const unsigned int nArgMax = argc - " << oaArgSchemeList[s].nFixedLastArgN << ";" << endl
             << "    for( unsigned int n = " << ( 1 + oaArgSchemeList[s].nFixedFirstArgN ) << "; n < nArgMax; n++ )" << endl
             << "    {" << endl
             << "      switch( naHashList[n] )" << endl
             << "      {" << endl;
    // ** Code fuer jedes optionale Argument einbetten; **
    for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
      if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<LASTARG>" ) * oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) != 0 )
      {
        // ** Die Argumentabfrage in die Ausgabedatei schreiben; **
        oOutFile << "        // \"-" << oaArgSchemeList[s].oaArgList[a].sName << "\"" << endl
                 << "        case " << Hash( ( std::string( "-" ) + oaArgSchemeList[s].oaArgList[a].sName ).c_str() ) << ":" << endl
                 << "        {" << endl;

        // ** Die Bearbeitungsfunktion in der Ausgabedatei weiterschreiben: Pruefen, ob ausreichend Argumenttokens fuer alle Argumentwerte vorhanden ist; **
        if( oaArgSchemeList[s].oaArgList[a].faVal.size() > 0 )
          oOutFile << "          if( n >= argc - "<< ( oaArgSchemeList[s].oaArgList[a].faVal.size() + oaArgSchemeList[s].nFixedLastArgN ) << " )" << endl
                   << "          {" << endl
                   << "            if( naHashList != NULL )" << endl
                   << "              std::free( naHashList );" << endl
                   << "            return Error( \"argument \\\"-" << oaArgSchemeList[s].oaArgList[a].sName << "\\\" requires " << oaArgSchemeList[s].oaArgList[a].faVal.size() << " value(s)\", szErrMsg_o );" << endl
                   << "          }" << endl;

        // ** Den Kommandocode in die Ausgabedatei schreiben; **
        oOutFile << "          ";
        if( !WriteCommand( (char*) oaArgSchemeList[s].oaArgList[a].sCmd.c_str(), oaArgSchemeList[s].oaArgList[a], "n + ", 1, oOutFile ) )
          return -1;

        // ** Den Bearbeitungsfall des Arguments wieder schliessen; **
        if( oaArgSchemeList[s].oaArgList[a].faVal.size() > 0 )
          oOutFile << "          n += " << oaArgSchemeList[s].oaArgList[a].faVal.size() << ";" << endl;
        oOutFile << "          continue;" << endl
                 << "        }" << endl;

        // ** Ausgabe; **
        cout << "-> Argument \"-" << oaArgSchemeList[s].oaArgList[a].sName << "\" added with " << oaArgSchemeList[s].oaArgList[a].faVal.size() << " value(s)." << endl;
      }
    oOutFile << "        default:" << endl
             << "          nWrongArg = n;" << endl
             << "          fOptArgOk = false;" << endl
             << "          break;" << endl
             << "      }" << endl
             << "      break;" << endl
             << "    }" << endl;

    // ** Das Schleifenende erzeugen; **
    oOutFile << "    if( fOptArgOk )" << endl
             << "    {" << endl;

    // ** Code fuer jedes fixe Argument am Anfang und am Ende in der Reihenfolge der Eingabedatei einbetten; **
    {
      unsigned int nCountLast = 1;
      unsigned int nCountFirst = 1;
      for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
      {
        // ** Die festen ersten Argumente ausfuehren; **
        if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) == 0 )
        {
          oOutFile << "      ";
          if( !WriteCommand( (char*) oaArgSchemeList[s].oaArgList[a].sCmd.c_str(), oaArgSchemeList[s].oaArgList[a], "", nCountFirst, oOutFile ) )
            return -1;
          nCountFirst++;

          // ** Ausgabe; **
          cout << "-> One required first argument added." << endl;
        }

        // ** Die festen letzten Argumente ausfuehren; **
        if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<LASTARG>" ) == 0 )
        {
          oOutFile << "      ";
          if( !WriteCommand( (char*) oaArgSchemeList[s].oaArgList[a].sCmd.c_str(), oaArgSchemeList[s].oaArgList[a], "argc - ", nCountLast, oOutFile ) )
            return -1;
          nCountLast++;

          // ** Ausgabe; **
          cout << "-> One required last argument added." << endl;
        }
      }
    }

    // ** Das Schleifenende abschliessen; **
    oOutFile << "      if( naHashList != NULL )" << endl
             << "        std::free( naHashList );" << endl
             << "      return true;" << endl
             << "    }" << endl
             << "  }" << endl;
  }

  // ** Die InterpArgs()-Funktion abschliessen; **
  oOutFile << "  if( nWrongArg != -1 )" << endl
           << "  {" << endl
           << "    if( szErrMsg_o != NULL )" << endl
           << "    {" << endl
           << "      bool fUnknown = true;" << endl
           << "      switch( naHashList[nWrongArg] )" << endl
           << "      {" << endl;
  for( unsigned int s = 0; s < oaArgSchemeList.size(); s++ )
    for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
      if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<LASTARG>" ) * oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) != 0 )
      {
        // Argument mustn't appear duplicate in switch case.
        // Check if argument appears earlier in the list.
        bool fFirst;
        for( unsigned int s2 = 0; s2 <= oaArgSchemeList.size(); s2++ )
        {
          bool fBreak = false;
          for( unsigned int a2 = 0; a2 < oaArgSchemeList[s2].oaArgList.size(); a2++ )
          {
            if( s2 == s && a2 == a )
            {
              fFirst = true;
              fBreak = true;
              break;
            }

            if( oaArgSchemeList[s].oaArgList[a].sName == oaArgSchemeList[s2].oaArgList[a2].sName )
            {
              fFirst = false;
              fBreak = true;
              break;
            }
          }
          if( fBreak )
            break;
        }

        if( fFirst )
          oOutFile << "        case " << Hash( ( std::string( "-" ) + oaArgSchemeList[s].oaArgList[a].sName ).c_str() ) << ":" << endl
                   << "        fUnknown = false;" << endl
                   << "        break;" << endl;
      }
  oOutFile << "      }" << endl
           << "      if( fUnknown )" << endl
           << "      {" << endl
           << "        strcpy( szErrMsg_o, \"argument \\\"\" );" << endl
           << "        strcat( szErrMsg_o, argv[nWrongArg] );" << endl
           << "        strcat( szErrMsg_o, \"\\\" unknown\" );" << endl
           << "      }" << endl
           << "      else" << endl
           << "      {" << endl
           << "        strcpy( szErrMsg_o, \"illegal use of argument \\\"\" );" << endl
           << "        strcat( szErrMsg_o, argv[nWrongArg] );" << endl
           << "        strcat( szErrMsg_o, \"\\\"\" );" << endl
           << "      }" << endl
           << "    }" << endl
           << "    if( naHashList != NULL )" << endl
           << "      std::free( naHashList );" << endl
           << "    return false;" << endl
           << "  }" << endl
           << "  if( naHashList != NULL )" << endl
           << "    std::free( naHashList );" << endl
           << "}" << endl;

  // ** Die Funktion PrintCmdLineHelp() erzeugen; **
  oOutFile << endl
           << "// -----------------------------------------------------------------" << endl
           << endl
           << "bool argscan::PrintCmdLineHelp( const char* szProgName )" << endl
           << "{" << endl
           << "  printf( \"COMMAND LINE USAGE\\n\" );" << endl
           << "  printf( \"\\n\" );" << endl;

  // ** Schreibe die Kommandoaufrufe jedes Schemas; Tracke die Anzahl an Argumenten und die max. Stringlaenge der opt. Argumente mit; **
  unsigned int nArgN = 0;
  vector <unsigned int> naSchemeOptArgN;
  naSchemeOptArgN.resize( oaArgSchemeList.size() );
  vector <unsigned int> naSchemeMaxOArgLen;
  naSchemeMaxOArgLen.resize( oaArgSchemeList.size() );
  for( unsigned int s = 0; s < oaArgSchemeList.size(); s++ )
  {
    // ** Bezeichner des Schemas; **
    char szSchemeIdent[10];
    BuildSchemeIdent( s, szSchemeIdent );

    // ** Zaehle die Anzahl optionalen Argumente und ermittle die **
    // ** laengste Stringlaenge der Namen der optionalen Argumente; **
    naSchemeOptArgN[s] = 0;
    naSchemeMaxOArgLen[s] = 0;
    for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
      if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<LASTARG>" ) * oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) != 0 )
      {
        naSchemeOptArgN[s]++;
        if( oaArgSchemeList[s].oaArgList[a].sName.length() > naSchemeMaxOArgLen[s] )
          naSchemeMaxOArgLen[s] = oaArgSchemeList[s].oaArgList[a].sName.length();
      }

    // Limit counter for maximum optional argument name length to 10 characters
    // since it is later used as automatic indentation that should not be too
    // large.
    if( naSchemeMaxOArgLen[s] > 10 )
      naSchemeMaxOArgLen[s] = 10;

    // ** Schreibe den Kommandoaufruf des Schemas; **
    oOutFile << "  printf( \"    %s";
    for( unsigned int a = 0; a < oaArgSchemeList[s].nFixedFirstArgN; a++ )
    {
      oOutFile << " Arg" << szSchemeIdent << nArgN + 1;
      nArgN++;
    }
    if( naSchemeOptArgN[s] > 0 )
      oOutFile << " [OPTIONS " << szSchemeIdent << "]";
    for( unsigned int a = 0; a < oaArgSchemeList[s].nFixedLastArgN; a++ )
    {
      oOutFile << " Arg" << szSchemeIdent << nArgN + 1;
      nArgN++;
    }
    oOutFile << "\\n\", szProgName );" << endl;
  }
  oOutFile << "  printf( \"\\n\" );" << endl;

  // ** Erklaere die obligatorschen Argumente jedes Schemas; **
  if( nArgN > 0 )
  {
    oOutFile << "  printf( \"ARGUMENTS\\n\" );" << endl
             << "  printf( \"\\n\" );" << endl;
    for( unsigned int s = 0; s < oaArgSchemeList.size(); s++ )
    {
      // ** Bezeichner des Schemas; **
      char szSchemeIdent[10];
      BuildSchemeIdent( s, szSchemeIdent );

      // ** Schreibe die vorderen obligatorischen Argumente; **
      unsigned int nSchemeArgN = 0;
      for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
        if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) == 0 )
        {
          oOutFile << "  printf( \"    Arg" << szSchemeIdent << nSchemeArgN +1 ;
          for( unsigned int v = 0; v < oaArgSchemeList[s].oaArgList[a].faVal.size(); v++ )
          {
            oOutFile << " (";
            OutputVarTypeString( oaArgSchemeList[s].oaArgList[a].faVal[v], oOutFile );
            oOutFile << ")";
          }
          oOutFile << "\\n\" );" << endl;

          // ** Schreibe den Erklaerungstext; **
          {
            // ** Laenge der Zeile und Erzeugung einer Kopie des Erklaertextes; **
            signed int nDescLineLen = nLineLen - 11;
            string sDesc = oaArgSchemeList[s].oaArgList[a].sDesc;

            // ** Alle Zeilen ausgeben; **
            while( sDesc.length() > 0 )
            {
              string sLine = SplitFirstDescrLine( sDesc, nDescLineLen );
              oOutFile << "  printf( \"           " << EscapeString( sLine ) << "\\n\" );" << endl;
            }

            // ** Finale Leerzeile; **
            oOutFile << "  printf( \"\\n\" );" << endl;
          }

          nSchemeArgN++;
        }

      // ** Schreibe die hinteren obligatorischen Argumente; **
      // ** (Durchlauf VON HINTEN, da die letzten Argumente in umgekehrter Reihenfolge sortiert sind;) **
      for( unsigned int a = oaArgSchemeList[s].oaArgList.size(); a > 0; a-- )
        if( oaArgSchemeList[s].oaArgList[a-1].sName.compare( "<LASTARG>" ) == 0 )
        {
          oOutFile << "  printf( \"    Arg" << szSchemeIdent << nSchemeArgN + 1;
          for( unsigned int v = 0; v < oaArgSchemeList[s].oaArgList[a-1].faVal.size(); v++ )
          {
            oOutFile << " (";
            OutputVarTypeString( oaArgSchemeList[s].oaArgList[a-1].faVal[v], oOutFile );
            oOutFile << ")";
          }
          oOutFile << "\\n\" );" << endl;

          // ** Schreibe den Erklaerungstext; **
          {
            // ** Laenge der Zeile und Erzeugung einer Kopie des Erklaertextes; **
            signed int nDescLineLen = nLineLen - 11;
            string sDesc = oaArgSchemeList[s].oaArgList[a-1].sDesc;

            // ** Alle Zeilen ausgeben; **
            while( sDesc.length() > 0 )
            {
              string sLine = SplitFirstDescrLine( sDesc, nDescLineLen );
              oOutFile << "  printf( \"           " << EscapeString( sLine ) << "\\n\" );" << endl;
            }

            // ** Finale Leerzeile; **
            oOutFile << "  printf( \"\\n\" );" << endl;
          }

          nSchemeArgN++;
        }
    }
  }

  // ** Erklaere die optionalen Argumente jedes Schemas; **
  for( unsigned int s = 0; s < oaArgSchemeList.size(); s++ )
    if( naSchemeOptArgN[s] > 0 )
    {
      // ** Bezeichner des Schemas; **
      char szSchemeIdent[10];
      BuildSchemeIdent( s, szSchemeIdent );

      oOutFile << "  printf( \"OPTIONS " << szSchemeIdent << "\\n\" );" << endl
               << "  printf( \"\\n\" );" << endl;

      for( unsigned int a = 0; a < oaArgSchemeList[s].oaArgList.size(); a++ )
        if( oaArgSchemeList[s].oaArgList[a].sName.compare( "<LASTARG>" ) * oaArgSchemeList[s].oaArgList[a].sName.compare( "<FIRSTARG>" ) != 0 )
        {
          // ** => Ein optionales Argument; **

          // ** "Description lumping": Pruefe, ob das nachfolgende optionale Argument **
          // ** DIESELBE Erklaerung hat. In diesem Fall soll die Erklaerung fuer die **
          // ** Argumente gemeinsam erfolgen. **
          bool fLump = false;
          {
            // ** Suche das naechste optionale Argument; **
            for( unsigned int na = a + 1; na < oaArgSchemeList[s].oaArgList.size(); na++ )
              if( oaArgSchemeList[s].oaArgList[na].sName.compare( "<LASTARG>" ) * oaArgSchemeList[s].oaArgList[na].sName.compare( "<FIRSTARG>" ) != 0 )
              {
                // ** Pruefe, ob die Beschreibung des darauffolgenden optionalen Arguments **
                // ** tupfengleich identisch mit der Beschreibung dieses Arguments ist; **
                if( oaArgSchemeList[s].oaArgList[a].sDesc == oaArgSchemeList[s].oaArgList[na].sDesc )
                  fLump = true;
                break;
              }
          }

          // ** String erzeugen der den Bezeichner und alle Argumente des optionalen Parameters enthaelt;
          std::string sFullIdent;
          {
            std::stringstream oFullIdent;
            oFullIdent << oaArgSchemeList[s].oaArgList[a].sName;
            for( unsigned int v = 0; v < oaArgSchemeList[s].oaArgList[a].faVal.size(); v++ )
            {
              oFullIdent << " ";
              OutputVarTypeString( oaArgSchemeList[s].oaArgList[a].faVal[v], oFullIdent );
            }
            sFullIdent = oFullIdent.str();
          }

          // ** Ausgabe des Bezeichners inkl. Argumente, fallunterschieden nach dessen Laenge; **
          if( sFullIdent.length() <= naSchemeMaxOArgLen[s] )
          {
            // ** => Kurzes Argument; Schreibe offen **
            oOutFile << "  printf( \"    -" << std::left << setw( naSchemeMaxOArgLen[s] ) << sFullIdent << "  ";

            // ** Zeilenumbruch beim Description-Lumping; **
            if( fLump )
              oOutFile << "\\n\" );" << endl;
          }
          else
          {
            // ** => Langes Argument; Schreibe mit Zeilenumbruch; **
            oOutFile << "  printf( \"    -" << sFullIdent << "\\n\" );" << endl;

            // ** Wiedereroeffnen der Zeile falls KEIN Description-Lumping; **
            if( !fLump )
              oOutFile << "  printf( \"       " << setw( naSchemeMaxOArgLen[s] ) << "";
          }

          // ** Beim Description-Lumping mit dem naechsten optionalen Argument fortfahren; **
          if( fLump )
            continue;

          // ** Schreibe den Erklaerungstext in Anschluss an den Argumentbezeichner; **
          {
            // ** Berechne Laenge der Zeile und erzeuge eine Kopie des Erklaertextes; **
            const signed int nDescLineLen = nLineLen - naSchemeMaxOArgLen[s] - 7;
            string sDesc = oaArgSchemeList[s].oaArgList[a].sDesc;

            // ** Reduziere Erklaertext um eine Zeile; **
            string sLine = SplitFirstDescrLine( sDesc, nDescLineLen );

            // ** Ausgabe der ersten Zeile; **
            oOutFile << EscapeString( sLine ) << "\\n\" );" << endl;

            // ** Alle weiteren Zeilen ausgeben; **
            while( sDesc.length() > 0 )
            {
              sLine = SplitFirstDescrLine( sDesc, nDescLineLen );
              oOutFile << "  printf( \"       " << setw( naSchemeMaxOArgLen[s] ) << "" << EscapeString( sLine ) << "\\n\" );" << endl;
            }

            // ** Finale Leerzeile; **
            oOutFile << "  printf( \"\\n\" );" << endl;
          }
        }
      }
  oOutFile << "  return true;" << endl
           << "}" << endl;

  // ** Die Helper-Funktionen erzeugen; **
  oOutFile << endl
           << "// -----------------------------------------------------------------------------" << endl
           << endl
           << "// HELPER FUNCTION IMPLEMENTATION" << endl
           << endl
           << "// -----------------------------------------------------------------------------" << endl
           << endl
           << "inline bool argscan::Error( const char* szTxt, char* szErrMsg_o )" << endl
           << "{" << endl
           << "  if( szErrMsg_o != NULL )" << endl
           << "    if( szTxt == NULL )" << endl
           << "      szErrMsg_o[0] = 0;" << endl
           << "    else" << endl
           << "    {" << endl
           << "      strncpy( szErrMsg_o, szTxt, max_len_error_msg );" << endl
           << "      szErrMsg_o[max_len_error_msg-1] = 0;" << endl
           << "    }" << endl
           << "  return false;" << endl
           << "}" << endl
           << endl
           << "// -----------------------------------------------------------------------------" << endl
           << endl
           << "// *****************************************************************************" << endl
           << "// * Hash implementation of the Sleepycat's Datenbank BDB (Berkeley DataBase). *" << endl
           << "// *****************************************************************************" << endl
           << endl
           << "inline size_t argscan::Hash( const char* szString_i )" << endl
           << "{" << endl
           << "  size_t nHash = 0;" << endl
           << "  int c;" << endl
           << "  while (c = *szString_i++)" << endl
           << "    nHash = c + (nHash << 6) + (nHash << 16) - nHash;" << endl
           << "  return nHash;" << endl
           << "}" << endl;

  // ** Dateien schliessen; **
  oOutFile.close();
  cout << "-> Finished writing output file \"" << argv[2] << "\"." << endl;

  // ** Program erfolgreich beenden; **
  cout << endl;
  return 0;
}

// -----------------------------------------------------------------

// IMPLEMENTATION WEITERER FUNKTIONEN

// -----------------------------------------------------------------

bool ReadGrammarFile( const char* szFileName_i, vector <SArgScheme>& oaSchemeList_o )
{
  // Clear buffer.
  oaSchemeList_o.clear();

  // Open input file for reading.
  ifstream oInFile( szFileName_i );
  if( !oInFile.good() )
  {
    cerr << "ERROR by corrupted or missing input file:" << endl
         << "  The input file \"" << szFileName_i << "\" cannot be opened." << endl
         << endl;
    return false;
  }

  // Create an new scheme as buffer storage.
  SArgScheme oNewScheme;

  // Read file content line by line. Count line numbers.
  string sLine;
  size_t nLineCount = 0;
  while( oInFile.good() )
  {
    // Read next line.
    getline( oInFile, sLine );
    nLineCount++;

    // Check 1: skip lines that do not begin with an argument definition
    // (lowercase or uppercase letters, numbers, dash or '<' character) or
    // with a scheme change character (percent character).
    if( sLine.size() == 0 )
      continue;
    if( strchr( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-<%", sLine[0] ) == NULL )
      continue;

    // Check 2: scheme change command ('%%').
    if( sLine.compare( 0, 2, "%%" ) == 0 )
    {
      // => Scheme change.

      // Save scheme in scheme list.
      oaSchemeList_o.push_back( oNewScheme );

      // Clear scheme.
      oNewScheme.Clear();
    }
    else
    {
      // => Argument definition.

      // Create a new argument object.
      SArg oNewArg;

      // Store first line for error output.
      size_t nArgLine = nLineCount;

      // Check position of code sequence termination character '}'. Eventually
      // read additional lines from the file and append sLine.
      while( true )
      {
        // Get last character of current argument line.
        size_t nPosCodeTerm = sLine.find_last_not_of( " \t\r\n" );

        // Break, if the last character is a code sequence termination '}'.
        if( nPosCodeTerm != string::npos )
          if( sLine[nPosCodeTerm] == '}' )
            break;

        // Exit if no further lines can be read.
        if( !oInFile.good() )
        {
          cerr << "ERROR in argument in line " << nArgLine << ":" << endl
               << "  No code sequence termination \'}\' found in this line or in the following." << std::endl;
          return false;
        }

        // Otherwise read next line.
        string sNewLine;
        getline( oInFile, sNewLine );
        nLineCount++;

        // Replace line breaks from end of current line by a blank space and
        // append the new line.
        if( sLine.back() == '\n' )
          sNewLine.pop_back();
        if( sLine.back() == '\r' )
          sNewLine.pop_back();
        sLine.push_back( ' ' );
        sLine += sNewLine;
      }

      // The rest of the code still uses C strings. Therfore create a dynamically allocated
      // C string that gets a copy of the line.
      char* szLine = (char*) malloc( sLine.size() + 1 );
      if( szLine == NULL )
      {
        cerr << "ERROR: memory allocation failed. Exiting program." << std::endl;
        return false;
      }
      strcpy( szLine, sLine.c_str() );

      // Find the code sequence of the argument and split it off. Eventually
      // additional lines need to be read.
      {
        char* szCmd = strrchr( szLine, '}' );
        if( szCmd == NULL )
        {
          cerr << "ERROR: an internal error occurred. Exiting program." << std::endl;
          return false;
        }
        *szCmd = 0;

        // ** Verschachtelung geschweifter Klammern beruecksichtigen; **
        unsigned int nBrckLvl = 0;
        unsigned int nStepN = szCmd - szLine;
        bool bOK = false;
        for( unsigned int s = 0; s < nStepN; s++ )
        {
          szCmd--;
          if( *szCmd == '}' )
            nBrckLvl++;
          if( *szCmd == '{' )
            if( nBrckLvl == 0 )
            {
              *szCmd = 0;
              szCmd++;
              oNewArg.sCmd = szCmd;
              bOK = true;
              break;
            }
            else
              nBrckLvl--;
        }
        if( !bOK )
        {
          cerr << "ERROR in argument in line " << nArgLine << ":" << endl
               << "  No code sequence start delimiter \'{\' found in this line or in the following." << endl
               << endl;
          return false;
        }
      }

      // ** Den Bereich der Argumentbeschreibung der Zeile suchen und abspalten; **
      char* szDesc;
      {
        szDesc = strrchr( szLine, '\'' );
        if( szDesc == NULL )
        {
          cerr << "ERROR in argument in line " << nArgLine << ":" << endl
              << "  No description text end delimiter \"\'\" found in this line or in the following." << endl
              << endl;
          return false;
        }
        *szDesc = 0;
        szDesc = strrchr( szLine, '\'' );
        if( szDesc == NULL )
        {
          cerr << "ERROR in argument in line " << nArgLine << ":" << endl
              << "  No description text start delimiter \"\'\" found in this line or in the following." << endl
              << endl;
          return false;
        }
        *szDesc = 0;
        szDesc++;
        oNewArg.sDesc = szDesc;
      }

      // ** Die Zeile in logische Token zerlegen; Das erste Token ist der Argumentname; **
      char* szArg = strtok( szLine, " \t\r" );
      if( szArg != NULL )
      {
        // ** Spezialargumentzeichen pruefen; **
        if( szArg[0] == '<' )
        {
          if( ( strcmp( szArg, "<LASTARG>" ) * strcmp( szArg, "<FIRSTARG>" ) ) != 0 )
          {
            cerr << "ERROR in special argument in line " << nArgLine << ":" << endl
                << "  Special arguments can only be \"<LASTARG>\" or \"<FIRSTARG>\"." << endl
                << endl;
            return false;
          }

          if( strcmp( szArg, "<LASTARG>" ) == 0 )
            oNewScheme.nFixedLastArgN++;
          if( strcmp( szArg, "<FIRSTARG>" ) == 0 )
            oNewScheme.nFixedFirstArgN++;
        }

        // ** Argumentnamen speichern; **
        oNewArg.sName = szArg;

        // ** Alle weiteren Token sind Definitionen von Argumentwerten; Diese einlesen; **
        while( true )
        {
          char* szTok = strtok( NULL, " \t\r" );
          if( szTok == NULL )
            break;

          // ** Pruefen, ob das Token einen Argumentwert beschreibt und in Argumentliste anhaengen; **
          if( strcmp( szTok, "int" ) == 0 )
            oNewArg.faVal.push_back( BFB_AVT_INTEGER );
          else
            if( strcmp( szTok, "string" ) == 0 )
              oNewArg.faVal.push_back( BFB_AVT_STRING );
            else
              if( strcmp( szTok, "float" ) == 0 )
                oNewArg.faVal.push_back( BFB_AVT_FLOAT );
              else
                if( strcmp( szTok, "onoff" ) == 0 )
                  oNewArg.faVal.push_back( BFB_AVT_ONOFF );
                else
                {
                  cerr << "ERROR in argument in line " << nArgLine << ":" << endl
                      << "  Can't understand argument specifier \"" << szTok << "\"." << endl
                      << endl;
                  return false;
                }
        }

        // ** Prufen, ob Spezialargumente <FIRSTARG> oder <LASTARG> mit genau einem Argumentwert versehen wurden; **
        if( ( strcmp( szArg, "<LASTARG>" ) * strcmp( szArg, "<FIRSTARG>" ) ) == 0 )
          if( oNewArg.faVal.size() != 1 )
          {
            cerr << "ERROR in special argument in line " << nArgLine << ":" << endl
                << "  Special arguments \"<LASTARG>\" or \"<FIRSTARG>\" must have exactly one value." << endl
                << endl;
            return false;
          }
      }
      else
      {
        cerr << "ERROR in argument in line " << nArgLine << ":" << endl
             << "  Argument could not be identified." << endl
             << endl;
        return false;
      }

      // ** Argument in Argumentliste ablegen; **
      oNewScheme.oaArgList.push_back( oNewArg );

      // Free memory.
      free( szLine );
    }
  }

  // ** Schema in Schemenliste ablegen; **
  oaSchemeList_o.push_back( oNewScheme );

  // ** Dateien schliessen; **
  oInFile.close();

  return true;
}

// -----------------------------------------------------------------

unsigned int GetMinArgN( vector <SArgScheme>& oaSchemeList_i )
{
  if( oaSchemeList_i.empty() )
    return 0;
  else
  {
    unsigned int nMinArgN = oaSchemeList_i[0].nFixedFirstArgN + oaSchemeList_i[0].nFixedLastArgN;
    for( unsigned int s = 1; s < oaSchemeList_i.size(); s++ )
    {
      unsigned int nArgN = oaSchemeList_i[s].nFixedFirstArgN + oaSchemeList_i[s].nFixedLastArgN;
      if( nArgN < nMinArgN )
        nMinArgN = nArgN;
    }
    return nMinArgN;
  }
}

// -----------------------------------------------------------------

bool WriteCommand( char* szCmd_i, SArg& oArg_i, const char* szPrefix_i, unsigned int nOffs_i, ofstream& oOutfile_o )
{
  while( *szCmd_i != 0 )
  {
    // ** Argumentwert-Ersetzungszeichen bearbeiten... **
    if( *szCmd_i == '#' )
    {
      szCmd_i++;

      // ** Nummer des Argumentwert-Ersetzungszeichens bestimmen; **
      unsigned int nAVal;
      char szNum[5];
      {
        for( unsigned int n = 0; n < 4; n++ )
          if( szCmd_i[n] == 0 )
          {
            szNum[n] = 0;
            break;
          }
          else
            if( strchr( "0123456789", szCmd_i[n] ) == NULL )
            {
              szNum[n] = 0;
              break;
            }
            else
              szNum[n] = *szCmd_i;
        nAVal = atoi( szNum ) - 1;
      }
      if( nAVal >= oArg_i.faVal.size() )
      {
        cerr << "ERROR: Wrong argument value identifier" << endl
             << "  Argument value identifier \"#" << szNum << "\" not in range in argument \"" << oArg_i.sName << "\"" << endl
             << endl;
        return false;
      }

      // ** Argumentwert je nach Typ interpretiert in Ausgabedatei schreiben; **
      switch( oArg_i.faVal[nAVal] )
      {
        case BFB_AVT_INTEGER:
          oOutfile_o << "atoi( argv[" << szPrefix_i << ( nOffs_i + nAVal ) << "] )";
          break;

        case BFB_AVT_FLOAT:
          oOutfile_o << "atof( argv[" << szPrefix_i << ( nOffs_i + nAVal ) << "] )";
          break;

        case BFB_AVT_ONOFF:
          oOutfile_o << "( strcmp( argv[" << szPrefix_i << ( nOffs_i + nAVal ) << "], \"on\" ) == 0 )";
          break;

        default:
          oOutfile_o << "argv[" << szPrefix_i << ( nOffs_i + nAVal ) << "]";
          break;
      }
    }
    // ** ... oder Code einfach kopieren; **
    else
      oOutfile_o << *szCmd_i;

    szCmd_i++;
  }
  oOutfile_o << endl;

  return true;
}

// -----------------------------------------------------------------

void OutputVarTypeString( signed int faVal_i, ostream& oOStream_o )
{
  switch( faVal_i )
  {
    case BFB_AVT_INTEGER:
      oOStream_o << "integer";
      return;

    case BFB_AVT_STRING:
      oOStream_o << "string";
      return;

    case BFB_AVT_FLOAT:
      oOStream_o << "float";
      return;

    case BFB_AVT_ONOFF:
      oOStream_o << "\\\"on\\\"/\\\"off\\\"";
      return;
  }
}

// -----------------------------------------------------------------

void BuildSchemeIdent( unsigned int nN_i, char* szIdent_o )
{
  unsigned int c = 0;
  while( true )
  {
    szIdent_o[c] = 'A' + nN_i % 26;
    nN_i /= 26;
    c++;
    if( nN_i == 0 )
      break;
  }
  szIdent_o[c] = 0;
}

// -----------------------------------------------------------------

std::string& EscapeString( std::string& sString_io )
{
  // ** Einfache Anfuehrungszeichen mit Backslash maskieren; **
  size_t nPosQM = sString_io.find_last_of( '\'' );
  while( nPosQM != string::npos )
  {
    sString_io.insert( nPosQM, "\\" );
    nPosQM = sString_io.find_last_of( '\'', nPosQM );
  }

  // ** Doppelte Anfuehrungszeichen mit Backslash maskieren; **
  nPosQM = sString_io.find_last_of( '"' );
  while( nPosQM != string::npos )
  {
    sString_io.insert( nPosQM, "\\" );
    nPosQM = sString_io.find_last_of( '"', nPosQM );
  }

  return sString_io;
}

// -----------------------------------------------------------------------------

std::string SplitFirstDescrLine( std::string& sDescr_io, unsigned int nLineLen_i )
{
  // ** Spalte einen Text der Laenge nLineLen_i ab; **
  std::string sLine = sDescr_io.substr( 0, nLineLen_i );

  // ** Pruefe, ob vor Ende der Zeile ein Zeilenumbruch vorkommt; **
  bool fLBreak = false;
  {
    size_t nPos = sLine.find_first_of( "\r\n" );
    if( nPos != string::npos )
    {
      fLBreak = true;
      sLine.resize( nPos );

      // ** Entferne den Zeilenumbruch aus der Beschreibung; **
      if( ( sDescr_io[nPos] == '\r' ) || ( sDescr_io[nPos] == '\n' ) )
        if( sDescr_io[nPos] == '\r' )
        {
          sDescr_io.erase( nPos, 1 );
          if( nPos < sDescr_io.length() )
            if( sDescr_io[nPos] == '\n' )
              sDescr_io.erase( nPos, 1 );
        }
        else
          sDescr_io.erase( nPos, 1 );
    }
  }

  // ** Pruefe, ob die Abspaltung innerhalb eines Wortes **
  // ** (d.h. nicht gefolgt von einem Weisszeichen) erfolgte; **
  bool fLShort = false;
  if( sLine.length() < sDescr_io.length() )
    if( !( ( sDescr_io[sLine.length()] == ' ' ) ||
           ( sDescr_io[sLine.length()] == '\t' ) ||
           ( sDescr_io[sLine.length()] == '\r' ) ||
           ( sDescr_io[sLine.length()] == '\n' ) ) )
    {
      // ** => Versuche, ein Weisszeichen zur Verkuerzung zu finden; **
      size_t nPos = sLine.find_last_of(" \t");
      if( nPos != string::npos )
      {
        fLShort = true;
        sLine.resize( nPos );
      }
    }

  // ** Reduziere den Beschreibungstext um die Zeile und ggf. um vorausgehende; **
  // ** Weisszeichen; **
  sDescr_io.erase( 0, sLine.length() );
  if( !fLBreak )
  {
    while( !sDescr_io.empty() )
      if( ( sDescr_io[0] == ' ' ) ||
          ( sDescr_io[0] == '\t' ) )
        sDescr_io.erase( 0, 1 );
      else
        break;
  }

  // ** => Ab hier kann die Laenge der Ausgabezeile wieder **
  // **    modifiziert werden;

  // ** Expandiere die Ausgabezeile ggf. auf volle Zeilenlaenge **
  // ** (um Block-Text zu erzielen); **
  if( fLShort )
  {

    size_t nPos = sLine.find_first_of( " \t" );
    if( nPos != string::npos )
      while( sLine.length() < nLineLen_i )
      {
        sLine.insert( nPos, " " );
        nPos = sLine.find_first_of( " \t", nPos + 2 );
        if( nPos == string::npos )
          nPos = sLine.find_first_of( " \t" );
      }
  }

  return sLine;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
// * Hash implementation of the Sleepycat's Datenbank BDB (Berkeley DataBase). *
// *****************************************************************************

size_t Hash( const char* szString_i )
{
  size_t nHash = 0;
  int c;

  while (c = *szString_i++)
    nHash = c + (nHash << 6) + (nHash << 16) - nHash;

  return nHash;
}

