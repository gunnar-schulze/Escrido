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
  const std::string StripOption( std::string& sLine_i );
  const std::string StripValue( std::string& sLine_i );
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

    // => All other lines must be of format "OPTION = value value value"

    // Strip OPTION
    const std::string sOption = StripOption( sLine );

    // Compare against known option names
    if( sOption == "TEMPLATE_DIR" )
    {
      std::string sDir = StripValue( sLine );
      if( !sDir.empty() )
        escrido::sTemplateDir = sDir;
    }
    else
    if( sOption == "INCLUDE" )
    {
      escrido::saIncludePaths.clear();
      std::string sPath = StripValue( sLine );
      while( !sPath.empty() )
      {
        escrido::saIncludePaths.push_back( sPath );
        sPath = StripValue( sLine );
      }
    }
    else
    if( sOption == "NAMESPACE" )
    {
      escrido::saNamespaces.clear();
      std::string sNamespace = StripValue( sLine );
      while( !sNamespace.empty() )
      {
        escrido::saNamespaces.push_back( sNamespace );
        sNamespace = StripValue( sLine );
      }
    }
    else
    if( sOption == "EXCLUDE_GROUPS" )
    {
      escrido::saExludeGroups.clear();
      std::string sGroup = StripValue( sLine );
      while( !sGroup.empty() )
      {
        escrido::saExludeGroups.push_back( sGroup );
        sGroup = StripValue( sLine );
      }
    }
    else
    if( sOption == "RELABEL" )
    {
      std::string sTerm = StripValue( sLine );
      std::string sReplace = StripValue( sLine );
      escrido::asRelabel.emplace_back( sTerm, sReplace );
    }
    else
    if( sOption == "GENERATE_WEBDOC" )
    {
      escrido::fWDOutput = ( StripValue( sLine ) == "YES" );
    }
    else
    if( sOption == "WEBDOC_OUT_DIR" )
    {
      std::string sDir = StripValue( sLine );
      if( !sDir.empty() )
        escrido::sWDOutputDir = sDir;
    }
    else
    if( sOption == "WEBDOC_FILE_ENDING" )
    {
      std::string sEnding = StripValue( sLine );
      if( !sEnding.empty() )
        escrido::sWDOutputPostfix = sEnding;
    }
    else
    if( sOption == "GENERATE_SEARCH_INDEX" )
    {
      escrido::fSearchIndex = ( StripValue( sLine ) == "YES" );
    }
    else
    if( sOption == "SEARCH_INDEX_ENDCODING" )
    {
      if( StripValue( sLine ) == "JS" )
        escrido::fSearchIdxEncode = escrido::search_index_encoding::JS;
      else
        escrido::fSearchIdxEncode = escrido::search_index_encoding::JSON;
    }
    else
    if( sOption == "SEARCH_INDEX_FILE" )
    {
      std::string sFileName = StripValue( sLine );
      if( !sFileName.empty() )
        escrido::sSeachIndexFile = sFileName;
    }
    else
    if( sOption == "GENERATE_LATEX" )
    {
      escrido::fLOutput = ( StripValue( sLine ) == "YES" );

    }
    else
    if( sOption == "LATEX_OUT_DIR" )
    {
      std::string sDir = StripValue( sLine );
      if( !sDir.empty() )
        escrido::sLOutputDir = sDir;
    }
    else
    {
      // Unknown OPTION
      return nResult;
    }
  }

  // Close file after reading.
  oFile.close();

  return std::string::npos;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Removes the first non-whitespace word (expected to be the option
///             name in a newly read line) and the folloing equation mark and
///             removes these.
///
/// \return     The option name.
// -----------------------------------------------------------------------------

const std::string config_file_parser::StripOption( std::string& sLine_i )
{
  std::string sResult;

  // Length of the line string.
  const size_t nLen = sLine_i.length();

  // Find start position of the option name: first non-whitespace character
  size_t nStart;
  for( nStart = 0; nStart < nLen; ++nStart )
    if( sLine_i[nStart] != ' ' &&
        sLine_i[nStart] != '\t' )
      break;
  if( nStart == nLen )
  {
    sLine_i.clear();
    return sResult;
  }

  // Find one-plue-the-end position of the option name: whitespace or equation mark
  size_t nEnd;
  for( nEnd = nStart; nEnd < nLen; ++nEnd )
    if( sLine_i[nEnd] == ' ' ||
        sLine_i[nEnd] == '\t' ||
        sLine_i[nEnd] == '=' )
      break;

  // Copy option name
  sResult = sLine_i.substr( nStart, nEnd - nStart );

  // Search further until the equation mark is found.
  for( nEnd; nEnd < nLen; ++nEnd )
    if( sLine_i[nEnd] == '=' )
    {
      ++nEnd;
      break;
    }

  // Remove content up to (and including) the equation mark
  sLine_i.erase( 0, nEnd );

  return sResult;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Strips off the next value token.
///
/// \details    If the value is enclosed by quotation marks, these characters
///             are removed.
///
/// \return     The next value token.
// *****************************************************************************

const std::string config_file_parser::StripValue( std::string& sLine_i )
{
  std::string sResult;

  // Length of the line string.
  const size_t nLineLen = sLine_i.length();

  // Find start position of the value: first non-whitespace character
  size_t nStart;
  for( nStart = 0; nStart < nLineLen; ++nStart )
    if( sLine_i[nStart] != ' ' &&
        sLine_i[nStart] != '\t' )
      break;
  if( nStart == nLineLen )
  {
    sLine_i.clear();
    return sResult;
  }

  // Find one-plue-the-end position of the value
  size_t nEnd;
  size_t nValueLen;
  {
    // Check whether the value is encapsulated in quotation marks
    const char cMark = sLine_i[nStart];
    if( cMark == '"' || cMark == '\'' )
    {
      // => The value is encapsulated within quotation marks

      // Shift start position by one.
      ++nStart;
      if( nStart == nLineLen )
      {
        sLine_i.clear();
        return sResult;
      }

      for( nEnd = nStart; nEnd < nLineLen; ++nEnd )
        if( sLine_i[nEnd] == cMark )
          break;

      nValueLen = nEnd - nStart;

      if( nEnd != nLineLen )
        ++nEnd;
    }
    else
    {
      // => The value is not encapsulated within quotation marks

      for( nEnd = nStart; nEnd < nLineLen; ++nEnd )
        if( sLine_i[nEnd] == ' ' ||
            sLine_i[nEnd] == '\t'  )
          break;

      nValueLen = nEnd - nStart;
    }
  }

  // Copy value
  sResult = sLine_i.substr( nStart, nValueLen );

  // Remove value
  sLine_i.erase( 0, nEnd );

  return sResult;
}

#endif /* CONFIG_FILE_PARSER_DOC_READ_ONCE */
