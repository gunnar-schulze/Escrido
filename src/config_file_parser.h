#ifndef CONFIG_FILE_PARSER_DOC_READ_ONCE
#define CONFIG_FILE_PARSER_DOC_READ_ONCE

// -----------------------------------------------------------------------------
/// \file       config_file_parser.h
///
/// \brief      Configuration file parser.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2015 trinckle 3D GmbH
// -----------------------------------------------------------------------------

#include <new>               // std::bad_alloc
#include <fstream>           // std::ifstream
#include <string>            // std::string

// -----------------------------------------------------------------------------

// FUNCTION OVERVIEW

// -----------------------------------------------------------------------------

namespace config_file_parser
{
  size_t ParseConfigFile( const std::string& sConfigFile_i );
  const std::string ConvertTokString( const char* szConvTok_i );
  void AppendTokString( const char* szAppendTok_i, std::vector <std::string>& saStingList_o );
}

// -----------------------------------------------------------------------------

// FUNCTION IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Parses the configuration file that is given as argument.
///
/// \return     std::string::npos on success, zero if a general file error
///             occurred or the number of the line (starting with 1) in
///             which an error occurred first.
// *****************************************************************************

inline size_t config_file_parser::ParseConfigFile( const std::string& sConfigFile_i )
{
  size_t nResult = 0;

  // Open config file for reading.
  std::ifstream oFile( sConfigFile_i );
  if( !oFile.good() )
    return nResult;

  while( oFile.good() )
  {
    // Extract a line from the file.
    std::string sLine;
    std::getline( oFile, sLine );
    nResult++;

    // Check against lines of pure whitespaces. Skip these.
    size_t nPos = sLine.find_first_not_of( " \t\r\n" );
    if( nPos == std::string::npos )
      continue;

    // Check against an opening hash tag: a comment. Skip these.
    if( sLine[nPos] == '#' )
      continue;

    // => All other lines must be of format "IDENTIFIER = value value value"

    // Create a C string for tokenization.
    size_t nSize = sizeof( char ) * ( sLine.size() + 1 );
    char* szLine = (char*) malloc( nSize );
    if( szLine == NULL )
      throw std::bad_alloc();
    memcpy( szLine, sLine.data(), nSize );

    // Tokenize the line up to the third token (i.e. the first value).
    // (Use of strtok() is ok since we do not run this code in parallel.)
    bool fError = false;
    char* szTok;
    char* szFirstTok = strtok( szLine, " \t" );
    if( szFirstTok == NULL )
      fError = true;
    else
    {
      szTok = strtok( NULL, " \t" );
      if( szTok == NULL )
        fError = true;
      else
        if( szTok[0] != '=' )
          fError = true;
        else
          szTok = strtok( NULL, " \t" );
    }

    // Assign the parameter values.
    if( !fError && szTok != NULL )
    {
      if( strcmp( szFirstTok, "TEMPLATE_DIR" ) == 0 )
      {
        escrido::sTemplateDir = ConvertTokString( szTok );
      }
      else
      if( strcmp( szFirstTok, "INCLUDE" ) == 0 )
      {
        escrido::saIncludePaths.clear();
        while( szTok != NULL )
        {
          AppendTokString( szTok, escrido::saIncludePaths );
          szTok = strtok( NULL, " \t" );
        }
      }
      else
      if( strcmp( szFirstTok, "NAMESPACE" ) == 0 )
      {
        escrido::saNamespaces.clear();
        while( szTok != NULL )
        {
          escrido::saNamespaces.push_back( szTok );
          szTok = strtok( NULL, " \t" );
        }
      }
      else
      if( strcmp( szFirstTok, "EXCLUDE_GROUPS" ) == 0 )
      {
        escrido::saExludeGroups.clear();
        while( szTok != NULL )
        {
          escrido::saExludeGroups.push_back( szTok );
          szTok = strtok( NULL, " \t" );
        }
      }
      else
      if( strcmp( szFirstTok, "GENERATE_WEBDOC" ) == 0 )
      {
        if( strcmp( szTok, "YES" ) == 0 )
          escrido::fWDOutput = true;
        else
          escrido::fWDOutput = false;
      }
      else
      if( strcmp( szFirstTok, "WEBDOC_OUT_DIR" ) == 0 )
      {
        escrido::sWDOutputDir = ConvertTokString( szTok );
      }
      else
      if( strcmp( szFirstTok, "WEBDOC_FILE_ENDING" ) == 0 )
      {
        escrido::sWDOutputPostfix = szTok;
      }
      else
      if( strcmp( szFirstTok, "GENERATE_SEARCH_INDEX" ) == 0 )
      {
        if( strcmp( szTok, "YES" ) == 0 )
          escrido::fSearchIndex = true;
        else
          escrido::fSearchIndex = false;
      }
      else
      if( strcmp( szFirstTok, "SEARCH_INDEX_ENDCODING" ) == 0 )
      {
        if( strcmp( szTok, "JS" ) == 0 )
          escrido::fSearchIdxEncode = escrido::search_index_encoding::JS;
        else
          escrido::fSearchIdxEncode = escrido::search_index_encoding::JSON;
      }
      else
      if( strcmp( szFirstTok, "SEARCH_INDEX_FILE" ) == 0 )
      {
        escrido::sSeachIndexFile = ConvertTokString( szTok );
      }
      else
      if( strcmp( szFirstTok, "GENERATE_LATEX" ) == 0 )
      {
        if( strcmp( szTok, "YES" ) == 0 )
          escrido::fLOutput = true;
        else
          escrido::fLOutput = false;
      }
      else
      if( strcmp( szFirstTok, "LATEX_OUT_DIR" ) == 0 )
      {
        escrido::sLOutputDir = ConvertTokString( szTok );
      }

    }

    // Free line memory.
    free( szLine );

    if( fError )
      return nResult;
  }

  // Close file after reading.
  oFile.close();

  return std::string::npos;
}

// -----------------------------------------------------------------------------

const std::string config_file_parser::ConvertTokString( const char* szConvTok_i )
{
  const char* pStart = szConvTok_i;
  size_t nLen = strlen( szConvTok_i );

  // Strip single or double quotation marks if apparent at beginning and end.
  if( nLen >= 2 )
  {
    if( *pStart == '"' )
    {
      if( *(pStart + nLen - 1 ) == '"' )
      {
        pStart++;
        nLen -= 2;
      }
    }
    else
    if( *pStart == '\'' )
    {
      if( *(pStart + nLen - 1 ) == '\'' )
      {
        pStart++;
        nLen -= 2;
      }
    }
  }

  return std::string( pStart, 0, nLen );
}

// -----------------------------------------------------------------------------

void config_file_parser::AppendTokString( const char* szAppendTok_i, std::vector <std::string>& saStingList_o )
{
  const char* pStart = szAppendTok_i;
  size_t nLen = strlen( szAppendTok_i );

  // Strip single or double quotation marks if apparent at beginning and end.
  if( nLen >= 2 )
  {
    if( *pStart == '"' )
    {
      if( *(pStart + nLen - 1 ) == '"' )
      {
        pStart++;
        nLen -= 2;
      }
    }
    else
    if( *pStart == '\'' )
    {
      if( *(pStart + nLen - 1 ) == '\'' )
      {
        pStart++;
        nLen -= 2;
      }
    }
  }

  saStingList_o.emplace_back( pStart, 0, nLen );
}

#endif /* CONFIG_FILE_PARSER_DOC_READ_ONCE */
