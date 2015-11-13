// *****************************************************************************
/// \file       main.cpp
///
/// \brief      Escrido: a multi-language documentation generator
///             that can be used parallel to doxygen/Doc/Ajascrido.
///
/// \details    A multi-language documentation generator for
///             generating pretty documentations and manual.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2015 Gunnar Schulze
///
// *****************************************************************************

// -----------------------------------------------------------------------------

// DOCUMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \mainpage   Escrido: a multi-language documentation generator.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \version    0.0.0
/// \copyright  2015 Gunnar Schulze
// *****************************************************************************

// -----------------------------------------------------------------------------

// INCLUSIONS

// -----------------------------------------------------------------------------

#include <vector>
#include <string>
#include <iostream>         // cin, cout, cerr, endl
#include <fstream>          // ifstream

#include "filesys.h"

#include "escrido-doc.h"

// Declare yyparse: required for yacc under Ubuntu 12.x (at least)
extern int yyparse();

// -----------------------------------------------------------------------------

// GLOBAL CONSTANTS

// -----------------------------------------------------------------------------

namespace applicationInfo
{
  const char szName[]        = "Escrido";
  const char szDescription[] = "Multi-language documentation generator.";
  const char szVersion[]     = "0.0.0";
  const char szFirstDate[]   = "October 2015";
  const char szDate[]        = "October 2015";
  const char szAuthor[]      = "Gunnar Schulze";
}

// -----------------------------------------------------------------------------

// GLOBAL VARIABLES

// -----------------------------------------------------------------------------

namespace escrido
{
  bool fVersion = false;                      ///< Flag if the program outputs version information.
  bool fHelp    = false;                      ///< Flag if the program outputs help information.
  std::string sConfigFile;                    ///< Configuration file name (if specified).
  std::vector <std::string> saIncludePaths;   ///< Include file names.
  std::vector <std::string> saNamespaces;     ///< List of namespaces the output shall be restricted to.
  std::string sTemplateDir = "./template/";   ///< Template directory name.
  bool fWDOutput = true;                      ///< Flag whether web document output shall be created.
  std::string sWDOutputDir = "./html/";       ///< Output directory name for web document files.
  std::string sWDOutputPostfix = ".html";     ///< Output postfix (file ending) of webdocument files.
  bool fLOutput = false;                      ///< Flag whether LaTeX output shall be created.
  std::string sLOutputDir = "./latex/";       ///< Output directory name for LaTeX document files.
  bool fShowInternal = false;                 ///< Flag, whether 'internal' tag blocks shall be shown.
  bool fDebug   = false;                      ///< Output debug information.

  CDocumentation oDocumentation;              ///< The code documentation content.

  // Parsing buffers:
  CContentUnit oParseContUnit;                ///< Content unit that is written to while parsing.
}

// -----------------------------------------------------------------------------

// FUNCTIONS DECLARATIONS

// -----------------------------------------------------------------------------

namespace escrido
{
  void AppendBlankSepStrings( char* szAppend_i, std::vector <std::string>& saStingList_o );
  void UnderlinedOut( const std::string& sOutput_i );
}

extern void InitScanner( const std::string* pSrc_i );  // Lex initialization.

// -----------------------------------------------------------------------------

// INCLUSIONS

// -----------------------------------------------------------------------------

#include "interpargs.h"         // Command line argument parser.
#include "config_file_parser.h" // Parser for the configuration file.
#include "yescrido.h"           // flex/bison JavaScript document parser.

// -----------------------------------------------------------------------------

// MAIN

// -----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
  using namespace escrido;

  // Interprete the command line arguments.
  char szArgScanError[argscan::max_len_error_msg];
  if( !argscan::InterpArgs( argc, argv, szArgScanError ) )
  {
    std::cerr << "Error: " << szArgScanError << std::endl;
    return 1;
  }

  // Output version information;
  {
    // Version information output as in GNU standard:
    std::cout << applicationInfo::szName << " " << applicationInfo::szVersion << std::endl
              << "Copyright (C) " << applicationInfo::szDate << " " << applicationInfo::szAuthor << std::endl
              << std::endl;
    if( fVersion )
      return 0;
  }

  // Output help;
  if( fHelp )
  {
    argscan::PrintCmdLineHelp( "escrido" );
    return 0;
  }

  // Read in configuration file, if demanded.
  if( !sConfigFile.empty() )
  {
    size_t nErrLine = config_file_parser::ParseConfigFile( sConfigFile );
    if( nErrLine != std::string::npos )
    {
      if( nErrLine == 0 )
        std::cerr << "Error on reading config file \"" << sConfigFile << "\"" << std::endl;
      else
        std::cerr << "Error in line " << nErrLine << " of config file \"" << sConfigFile << "\"" << std::endl;
      return 1;
    }
  }

  // Read files and parse them.
  for( size_t i = 0; i < saIncludePaths.size(); i++ )
  {
    // Get list of files defined by the include paths list.
    std::vector<filesys::SFileInfo> oaFileInfo;
    {
      char* szIncludePath = new char[saIncludePaths[i].length() + 1];
      strcpy( szIncludePath, saIncludePaths[i].c_str() );

      if( filesys::GetCanonicalPath( szIncludePath, szIncludePath ) )
        GetFilesInfo( szIncludePath, filesys::case_type::OS_CONVENTION, oaFileInfo );

      delete[] szIncludePath;
    }

    // Walk every file of the result list.
    for( size_t f = 0; f < oaFileInfo.size(); f++ )
      if( oaFileInfo[f].fItemType == filesys::item_type::FILE )
      {
        // Output currently processed file:
        UnderlinedOut( oaFileInfo[f].sPath );

        // Read file into a string.
        std::string sFileData;
        {
          // Open input file for binary reading ("raw mode");
          std::ifstream oInFile( oaFileInfo[f].sPath.c_str(), std::ifstream::in | std::ifstream::binary );
          if( !oInFile.is_open() )
          {
            std::cerr << "Error: file \"" << oaFileInfo[f].sPath << "\" cannot be opened" << std::endl;
            continue;
          }

          // Use iterator-template way of read the file completely (good 'best practice" method);
          // (Attention: the extra brackets arround the first constructor are essential; DO NOT REMOVE;)
          sFileData.assign( (std::istreambuf_iterator<char>( oInFile )),
                            std::istreambuf_iterator<char>() );

          // Close file again.
          oInFile.close();
        }

        // Initialize scanner with the input string.
        InitScanner( &sFileData );

        // Perform parsing.
        yyparse();
      }
  }

  // Program output.
  std::cout << std::endl;

  // Debug output.
  if( fDebug )
    escrido::oDocumentation.DebugOutput();

  // Remove all content unit that do not comply to the namespace white list.
  if( !saNamespaces.empty() )
    escrido::oDocumentation.RemoveNamespaces( saNamespaces );

  // Output web document.
  if( fWDOutput )
  {
    std::cout << "Writing web document into \"" << sWDOutputDir << "\"..." << std::endl;
    escrido::oDocumentation.WriteWebDoc( sTemplateDir, sWDOutputDir, sWDOutputPostfix, fShowInternal );
  }

  // Output LaTeX document.
  if( fLOutput )
  {
    std::cout << "Writing of LaTeX document not implemented yet. Sorry." << std::endl;
  }

  return 0;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Appends the blank space separted path declarations to the input
///             paths array.
///
/// \param[in]  szAppend_i
///             C string that is broken into seperate substrings that are
///             appended to the output list.
/// \param[out] saStingList_o
///             Output list of C++ strings.
// *****************************************************************************

void escrido::AppendBlankSepStrings( char* szAppend_i, std::vector <std::string>& saStingList_o )
{
  // Split into token seperated by blank spaces.
  char* pPos = strtok( szAppend_i, " \t" );
  while( pPos != NULL )
  {
    saStingList_o.push_back( pPos );
    pPos = strtok( NULL, " \t" );
  }
}

// -----------------------------------------------------------------------------

void escrido::UnderlinedOut( const std::string& sOutput_i )
{
  std::cout << sOutput_i << std::endl;
  const size_t nLen = sOutput_i.length();
  for( size_t l = 0; l < nLen; l++ )
    std::cout << "-";
  std::cout << std::endl;
}

