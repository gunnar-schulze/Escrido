#ifndef FILESYS_READ_ONCE
#define FILESYS_READ_ONCE

// *****************************************************************************
/// \file       filesys.h
///
/// \brief      OS-independent file system access (including wildcards).
///
/// \author     Gunnar Schulze
/// \version    2015-10-21
/// \date       2012
/// \copyright  2015 Gunnar Schulze
// *****************************************************************************

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

// -----------------------------------------------------------------------------

// INCLUSIONS

// -----------------------------------------------------------------------------

#include <cstring>
#include <string>
#include <vector>
#include <new>               // std::bad_alloc

// -----------------------------------------------------------------------------

// CLASSES OVERVIEW

// -----------------------------------------------------------------------------

// *********************
// *                   *
// *     SFileInfo     *
// *                   *
// *********************

/// Plattform independent file operations namespace.
namespace filesys
{
  struct SFileInfo;
}

// -----------------------------------------------------------------------------

// MACROS

// -----------------------------------------------------------------------------

namespace filesys
{
  // Operating systems depending format type of paths.
  enum os : unsigned char
  {
    THIS,      ///< This build's operating system (see OPERATING SYSTEM SETTING).
    UNIX,      ///< Unix, Mac OS X.
    WINDOWS    ///< DOS/Windows.
  };

  /// Path name capitalization cases.
  enum case_type : unsigned char
  {
    OS_CONVENTION,
    CASE_SENSITIVE,
    CASE_INSENSITIVE
  };

  /// Types of paths.
  enum path_type : unsigned char
  {
    ABS,
    REL
  };

  /// Types of items.
  enum item_type : unsigned char
  {
    DIRECTORY,
    FILE,
    OTHER
  };

  /// Comparison match type.
  enum match_type : unsigned char
  {
    EXACT,
    CASE_VAR
  };
}

// -----------------------------------------------------------------------------

// OPERATING SYSTEM SETTING

// -----------------------------------------------------------------------------

// Precompiler switch for setting the operating system.
// Comment out all inapplicable values.
#define FILESYS_UNIX
//#define FILESYS_WINDOWS

// -----------------------------------------------------------------------------

// FUNCTIONS OVERVIEW

// -----------------------------------------------------------------------------

namespace filesys
{
  // Path names:
  bool GetCanonicalPath( const char* szInPath_i, char* szCanonPath_o, os fOSType_i = os::THIS );
  bool GetCanonicalPath( const char* szInPath_i, std::string& sCanonPath_o, os fOSType_i = os::THIS );
  bool IsCanonicalPath( const char* szPath_i );

  // Accessing files and directories on the drive:
  bool GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o );

  // Helper functions:
  char* strtoupper( char* szString_i );
  int   strtouppercmp( const char* szString1_i, const char* szString2_i );
  int   strglobcmp( const char* szString1_i, const char* szString2_i );
  int   strtoupperglobcmp( const char* szString1_i, const char* szString2_i );
}

// -----------------------------------------------------------------------------

// STRUCT SFileInfo

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Information structure of a file or directory on the file system.
// *****************************************************************************

struct filesys::SFileInfo
{
  std::string sPath;      ///< Standartized path name.
  os          fOSType;    ///< Operating system depending format type of the path.
  path_type   fPathType;  ///< Type of the path (absolute or relative).
  item_type   fItemType;  ///< Type of the item (file or directory).
  match_type  fMatchType; ///< Flag defining how the path string matches to the search pattern.
  bool        fMatch;     ///< Used internally to flag paths that keep matching.
};

// -----------------------------------------------------------------------------

// FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Creates a <em>canonical path</em> string from an input path.
///
/// \details    This library uses <em>canonical paths</em> to represent absolute
///             or relative paths from different sources. These can be used in
///             other functions that share a common interface for different
///             operating systems.
///
///             A canonical path complies with the following rules:
///             - \b slashes ('/') as directory seperators
///             - \b no consecutive multiple slashes ("//")
///             - \b one leading slash '/' for an absolute path
///             - \b no trailing slash except the path represents the bare root
///                  directory ("/").
///             - \b no "single-dots" ("./") as reference to the same
///                  directory, neither at the beginning, inside the path or at
///                  its end
///             - \b may include one or more "double-dots" ("../") as reference
///               to the parent directory but only if the path is relative and
///               only at its beginning. All double-dots inside the path or at
///               its end need to be resolved.
///             - \b may contain all UTF-8 encoded unicode characters as file or
///               directory names except the slash character and the wildcards.
///             - \b may include wildcards '?' or '*'
///
///             Windows type absolute paths of the type "C:" are resolved into
///             the form "/c".
///
/// \param[in]  szInPath_i
///             Input path C string. This can be an absolute or relative path
///             in the convention of the operating system defined by fOSType_i.
/// \param[out] szCanonPath_o
///             C string that returns the standardized path name, if the
///             function succeeds. This must point to a memory block with a size
///             equal or larger than that of szInPath_i. The pointer may be
///             identical to szInPath_i.
/// \param[in]  fOSType_i
///             Defines the operating system convention of szInPath_i. Allowed
///             values are:
///             - os::THIS    : this compile's operating system conventions
///             - os::UNIX    : Unix or Mac OS X path conventions
///             - os::WINDOWS : DOS/Windows path conventions
///
/// \return     'true' if the input string was a valid path (with respect to the
///             the specific operation system) that got converted, 'false'
///             otherwise.
// *****************************************************************************

bool filesys::GetCanonicalPath( const char* szInPath_i, char* szCanonPath_o, os fOSType_i )
{
  // Eventually reset OS type.
  if( fOSType_i == os::THIS )
#ifdef FILESYS_UNIX
    fOSType_i = os::UNIX;
#endif
#ifdef FILESYS_WINDOWS
    fOSType_i = os::WINDOWS;
#endif

  // Variables
  const char *pScan, *pNext;
  char* pWrite;

  // Step 1: create standardized paths for different OS.
  switch( fOSType_i )
  {
    case os::UNIX:
    {
      // Build a copy of the path string.
      pScan = szInPath_i;
      pNext;
      pWrite = szCanonPath_o;
      while( *pScan != '\0' )
      {
        // Skip multiple slashes.
        if( *pScan == '/' )
          if( *(pScan+1) == '/' )
          {
            pScan++;
            continue;
          }

        *pWrite++ = *pScan++;
      }
      *pWrite = '\0';

      break;
    }

    case os::WINDOWS:
    {
      // Get string length.
      size_t nStrLen = strlen( szInPath_i );

      // Build a copy of the path string.
      pScan = szInPath_i;
      pNext;
      pWrite = szCanonPath_o;
      {
        // Identify windows type absolute path, e.g. "C:"
        // and converting them into canonical form "/c".
        if( nStrLen >= 2 )
          if( *(pScan+1) == ':' )
            if( ( *pScan >= 'a' && *pScan <= 'z' ) ||
                ( *pScan >= 'A' && *pScan <= 'Z' ) )
            {
              *pWrite++ = '/';
              *pWrite++ = tolower( *pScan );
              pScan += 2;
            }

        // Copy string while converting backslashes into slashes.
        while( *pScan != '\0' )
        {
          // Skip multiple slashes.
          if( *pScan == '/' || *pScan == '\\' )
          {
            pNext = pScan+1;
            if( *pNext == '/' || *pNext == '\\' )
            {
              pScan++;
              continue;
            }

            // Write character as slash only.
            *pWrite++ = '/';
          }
          else
            *pWrite++ = *pScan;

          pScan++;
        }
        *pWrite = '\0';
      }

      break;
    }
  }

  // Step 2: remove all single dots.
  pScan = szCanonPath_o;
  pWrite = szCanonPath_o;
  while( *pScan != '\0' )
  {
    // Skip single dots.
    if( *pScan == '.' )
    {
      pNext = pScan+1;
      if( *pNext == '/' )
      {
        if( pScan == szCanonPath_o )
        {
          pScan += 2;
          continue;
        }
        else
          if( *(pScan-1) == '/' )
          {
            pScan += 2;
            continue;
          }
      }
      else
        if( *pNext == '\0' )
          if( pScan == szCanonPath_o )
            break;
          else
            if( *(pScan-1) == '/' )
              break;
    }

    *pWrite++ = *pScan++;
  }
  *pWrite = '\0';

  // Step 3: Eventually remove trailing slash (except when only the root).
  if( pWrite > szCanonPath_o + 1 )
    if( *(pWrite-1) == '/' )
      *(pWrite-1) = 0;

  // Get the current string length.
  size_t nStrLen = pWrite - szCanonPath_o;

  // Step 4: resolve all double-dots that are not at the beginning.
  if( nStrLen >= 3 )
  {
    // Set pLow to the highest non-decrementable position, i.e.
    // behind the first slash for an absolute path or to either the
    // first character or behind the last slash of an beginning
    // sequence of "../" for a relative path.
    const char* pLow;
    {
      pLow = szCanonPath_o;
      if( szCanonPath_o[0] == '/' )
        pLow = szCanonPath_o + 1;

      const size_t nSeqMaxN = ( nStrLen + 1 ) / 3;
      pScan = szCanonPath_o;
      for( size_t s = 0; s < nSeqMaxN; s++ )
      {
        if( pScan[0] == '.' )
          if( pScan[1] == '.' )
            if( pScan[2] == '/' || pScan[2] == '\0' )
            {
              pScan += 3;
              pLow = pScan;
              continue;
            }
        break;
      }
    }

    // Scan through string to resolve double dots.
    pScan = pLow;
    pWrite = const_cast<char*>( pLow );
    while( *pScan != '\0' )
    {
      // Resolve double dots.
      if( *pScan == '.' )
      {
        pNext = pScan + 1;
        if( *pNext == '/' || *pNext == '\0' )
          if( pScan > pLow + 1 )
            if( *(pScan-1) == '.' )
              if( *(pScan-2) == '/' )
              {
                // => A sequence "/../" or "/..{EOS}" (EOS: end of string) was found.

                // Eventually add "../" and shift pLow.
                if( pWrite == pLow + 1 )
                {
                  pWrite = const_cast<char*>( pLow );
                  *pWrite++ = '.';
                  *pWrite++ = '.';
                  *pWrite++ = '/';
                  pLow = pWrite;
                }
                else
                {
                  // Set pWrite back to position after the one-before-last slash
                  // but not less than pLow.
                  pWrite -= 3;
                  while( pWrite > pLow )
                  {
                    if( *pWrite == '/' )
                    {
                      pWrite++;
                      break;
                    }
                    pWrite--;
                  }
                }

                // Set pScan forward behind "../".
                if( *pNext == '\0' )
                  break;
                else
                  pScan += 2;
              }
      }

      *pWrite++ = *pScan++;
    }
    *pWrite = '\0';
  }

  // Step 5: Eventually remove trailing slash (except when only the root).
  if( pWrite > szCanonPath_o + 1 )
    if( *(pWrite-1) == '/' )
      *(pWrite-1) = 0;

  // Get the current string length.
  nStrLen = pWrite - szCanonPath_o;

  // Step 6: check agains invalid paths "/../" (unix) or "/" (windows).
  switch( fOSType_i )
  {
    case os::UNIX:
    {
      // Path staring "/../" or path "/..{EOS}" (EOS: end of string).
      if( nStrLen >= 3 )
        if( szCanonPath_o[0] == '/' &&
            szCanonPath_o[1] == '.' &&
            szCanonPath_o[2] == '.' &&
            ( szCanonPath_o[3] == '/' || szCanonPath_o[3] == '\0' ) )
          return false;
      break;
    }

    case os::WINDOWS:
    {
      // Path "/" (windows).
      if( nStrLen == 1 )
        if( szCanonPath_o[0] == '/' )
          return false;
      break;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of \ref GetCanonicalPath( const char*, char*, os ) that
///             accepts a string as return variable.
// *****************************************************************************

bool filesys::GetCanonicalPath( const char* szInPath_i, std::string& sCanonPath_o, os fOSType_i )
{
  bool fResult;
  size_t nInLen = strlen( szInPath_i );
  if( nInLen == 0 )
  {
    sCanonPath_o.clear();
    fResult = true;
  }
  else
  {
    char* sBuf = (char*) malloc( sizeof( char ) * ( nInLen + 1 ) );
    if( sBuf == NULL )
      throw std::bad_alloc();
    fResult = GetCanonicalPath( szInPath_i, sBuf, fOSType_i );
    sCanonPath_o.assign( sBuf );
    free( sBuf );
  }
  return fResult;
}

// *****************************************************************************
// *                                                                           *
// *                            UNIX IMPLEMENTATION                            *
// *                                                                           *
// *****************************************************************************

#ifdef FILESYS_UNIX

// -----------------------------------------------------------------------------

// UNIX: INCLUSIONS

// -----------------------------------------------------------------------------

#include <string.h>
#include <ctype.h>
#include <dirent.h>     // POSIX directories
#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------

// UNIX: GLOBAL MACROS AND CONSTANTS

// -----------------------------------------------------------------------------

namespace filesys
{
  const char cSeparator = '/';               ///< Path separator character.
  const char szDevNull[] = "/dev/null";      ///< Null device string.
}

// -----------------------------------------------------------------------------

// UNIX: FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Evaluates a canonical path (that may include wildcards) into a
///             list of existing files or directories that exist on the drive.
///
/// \details    The function checks whether files or directories exists on the
///             file system that match the given (absolute or relative) path.
///
///             To verify a match, the function uses the uppercase/lowercase
///             conventions of the specific file system, i.e. it requires an
///             exact match on a UNIX system and a non-case-sensitive match on
///             a Windows system.
///
/// \param[in]  szCanonPath_i
///             A canonical path as it can be retrieved from
///             \ref GetCanonicalPath(). The path can contain the following
///             wildcards:
///             - ? : zero or one unknown character except dot(.)
///             - * : Match any number of unknown characters (regardless of
///                   the position where it appears, including at the start
///                   and/or multiple times)
///             Note that the string is changed by the function.
/// \param[in]  fCaseType_i
///             Flag what letter case convention is used for matching the
///             file names. Possible values are:
///             - case_type::OS_CONVENTION : the convention of the respective
///               operating system is used
///             - case_type::CASE_SENSITIVE : the comparison is performed
///               in a case sensitive form
///             - case_type::CASE_INSENSITIVE : the comparison is performed
///               in a not case sensitive form
/// \param[out] oaFileInfo_o
///             Returns a list of information about the files or directories
///             matching the canonical path name.
// *****************************************************************************

bool filesys::GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o )
{
  // Eventually reset case sensitivity flag.
  if( fCaseType_i == case_type::OS_CONVENTION )
    fCaseType_i = case_type::CASE_SENSITIVE;

  // Variables of types from <dirent.h>.
  DIR *pDir, *pCheckDir;
  struct dirent oEntry;
  struct dirent* pResult;

  // Pointers to the comparison functions (as used and case sensitive version).
  int (*pStrCmp)(const char*, const char*);
  int (*pStrCmpCaseSensitive)(const char*, const char*);

  // Split cursors.
  char* pBeg = szCanonPath_i;
  char* pEnd;

  // Use the output vector as stack for the different results.
  // Start with an initial value.
  oaFileInfo_o.resize( 1 );
  {
    if( *pBeg == '/' )
    {
      oaFileInfo_o[0].sPath.push_back( '/' );
      pBeg++;
      oaFileInfo_o[0].fPathType = path_type::ABS;
    }
    else
    {
      oaFileInfo_o[0].sPath = "./";
      oaFileInfo_o[0].fPathType = path_type::REL;
    }
    oaFileInfo_o[0].fOSType = os::UNIX;
    oaFileInfo_o[0].fItemType = item_type::DIRECTORY;
    oaFileInfo_o[0].fMatchType = match_type::EXACT;
    oaFileInfo_o[0].fMatch = true;
  }

  // Loop over all directory sections of the path.
  bool fLastSec = false;
  while( !fLastSec )
  {
    // Find next path section and cut it out.
    pEnd = pBeg;
    while( *pEnd != '/' && *pEnd != '\0' )
      pEnd++;
    fLastSec = ( *pEnd == '\0' );
    *pEnd = '\0';

    // Check whether wildcards are involved in this section.
    bool fWildcard = ( strpbrk( pBeg, "?*" ) != NULL );

    // Set pointer to the comparison functions that must be used for this section.
    if( fWildcard )
    {
      switch( fCaseType_i )
      {
        case case_type::CASE_SENSITIVE:
          pStrCmp = &strglobcmp;
          break;

        case case_type::CASE_INSENSITIVE:
          pStrCmp = &strtoupperglobcmp;
          break;
      }

      pStrCmpCaseSensitive = &strglobcmp;
    }
    else
    {
      switch( fCaseType_i )
      {
        case case_type::CASE_SENSITIVE:
          pStrCmp = &strcmp;
          break;

        case case_type::CASE_INSENSITIVE:
          pStrCmp = &strtouppercmp;
          break;
      }

      pStrCmpCaseSensitive = &strcmp;
    }

    // Loop over all elements that are currently in the stack.
    // The stack may get expanded during this process.
    const size_t nStackN = oaFileInfo_o.size();
    for( size_t s = 0; s < nStackN; s++ )
      if( oaFileInfo_o[s].fMatch )
      {
        // Store length of base path string.
        size_t nBasePathLen = oaFileInfo_o[s].sPath.size();

        // Store whether the base path is (up to now) an exact match or
        // whether it matches only case insensitively.
        match_type fBasePathMatchType = oaFileInfo_o[s].fMatchType;

        // Store path type. (Although this cannot change due to the algorithm's
        // construction but it's more consistent to do it here.)
        path_type fBasePathType = oaFileInfo_o[s].fPathType;

        // Open path section with POSIX "opendir()" function.
        bool fFirst = true;
        if( pDir = opendir( oaFileInfo_o[s].sPath.c_str() ) )
        {
          // Loop over all files within the path section.
          // Use multithreading safe reenrent version of POSIX "readdir()".
          while( readdir_r( pDir, &oEntry, &pResult ) == 0 )
          {
            // Break off if all items of the directory were read.
            if( pResult == NULL )
              break;

            // Distinguish between directory-like items and file-like items.
            // See http://man7.org/linux/man-pages/man3/readdir.3.html.
            item_type fItemType;
            switch( oEntry.d_type )
            {
              // Block device.
              case DT_BLK:
                fItemType = item_type::OTHER;
                break;

              // Character device.
              case DT_CHR:
                fItemType = item_type::OTHER;
                break;

              // Directory.
              case DT_DIR:
                fItemType = item_type::DIRECTORY;
                break;

              // Named pipe (FIFO).
              case DT_FIFO:
                fItemType = item_type::OTHER;
                break;

              // Symbolic link (treated as directory).
              case DT_LNK:
                fItemType = item_type::DIRECTORY;
                break;

              // Regular file.
              case DT_REG:
                fItemType = item_type::FILE;
                break;

              // UNIX domain socket.
              case DT_SOCK:
                fItemType = item_type::OTHER;
                break;

              case DT_UNKNOWN:
              {
                // According to http://man7.org/linux/man-pages/man3/readdir.3.html
                // not all file systems do support the d_type argument. The case of
                // an unknown item type should therefore be checked as well.
                {
                  // Do an early name comparison here to avoid extra work of
                  // creating extra the check path strings.
                  if( pStrCmp( pBeg, oEntry.d_name ) == 0 )
                  {
                    // Create a full path string. To do this operation here will
                    // slightly decrease the performance but it is only meant as
                    // fallback solution for rare file systems. Optimize the
                    // runtime performance of the function, if this block is
                    // called often.
                    std::string sCheckPath;
                    sCheckPath.assign( oaFileInfo_o[s].sPath, 0, nBasePathLen );
                    sCheckPath += oEntry.d_name;

                    // Try opening the check path as directory.
                    if( pCheckDir = opendir( sCheckPath.c_str() ) )
                    {
                      fItemType = item_type::DIRECTORY;
                      closedir( pCheckDir );
                    }
                    else
                      fItemType = item_type::FILE;
                  }
                  else
                    // Use item type "OTHER" here for discriminating wrongly named paths.
                    fItemType = item_type::OTHER;
                }
                break;
              }
            }

            // Check whether the item shall be added to the result list.
            // Add files only when this is the last path section.
            if( ( fItemType == item_type::DIRECTORY ) ||
                ( fItemType == item_type::FILE && fLastSec ) )
              if( pStrCmp( pBeg, oEntry.d_name ) == 0 )
                if( fFirst )
                {
                  fFirst = false;

                  // => Modify stack element s.

                  // Append entry name and eventually a slash to the path.
                  oaFileInfo_o[s].sPath += oEntry.d_name;
                  if( fItemType == item_type::DIRECTORY )
                    oaFileInfo_o[s].sPath.push_back( '/' );

                  // Set item type.
                  oaFileInfo_o[s].fItemType = fItemType;

                  // Check whether the path matches only case insensitively.
                  if( fCaseType_i == case_type::CASE_INSENSITIVE )
                    if( fBasePathMatchType == match_type::EXACT )
                      if( pStrCmpCaseSensitive( pBeg, oEntry.d_name ) != 0 )
                        oaFileInfo_o[s].fMatchType = match_type::CASE_VAR;
                }
                else
                {
                  // => Append new element to the stack.
                  oaFileInfo_o.resize( oaFileInfo_o.size() + 1 );

                  // Set path name as combination of the base path plus the entry name.
                  oaFileInfo_o.back().sPath.assign( oaFileInfo_o[s].sPath, 0, nBasePathLen );
                  oaFileInfo_o.back().sPath += oEntry.d_name;
                  if( fItemType == item_type::DIRECTORY )
                    oaFileInfo_o.back().sPath += '/';

                  // Set operating system dependent format type of the path.
                  oaFileInfo_o.back().fOSType = os::UNIX;

                  // Set path type.
                  oaFileInfo_o.back().fPathType = fBasePathType;

                  // Set item type.
                  oaFileInfo_o.back().fItemType = fItemType;

                  // Check whether the path matches exactly or only case insensitively.
                  switch( fCaseType_i )
                  {
                    case case_type::CASE_SENSITIVE:
                      oaFileInfo_o.back().fMatchType = match_type::EXACT;
                      break;

                    case case_type::CASE_INSENSITIVE:
                    {
                      if( fBasePathMatchType == match_type::EXACT )
                      {
                        if( pStrCmpCaseSensitive( pBeg, oEntry.d_name ) == 0 )
                          oaFileInfo_o.back().fMatchType = match_type::EXACT;
                        else
                          oaFileInfo_o.back().fMatchType = match_type::CASE_VAR;
                      }
                      else
                        oaFileInfo_o.back().fMatchType = match_type::CASE_VAR;
                      break;
                    }
                  }

                  // Set match to true.
                  oaFileInfo_o.back().fMatch = true;
                }
          }

          closedir( pDir );
        }

        // Check if the search returned any match and mark the stack
        // element accordingly.
        if( fFirst )
          oaFileInfo_o[s].fMatch = false;
      }

    // Break off or shift pBeg.
    if( fLastSec )
      break;
    else
      pBeg = pEnd + 1;
  }

  // Clear result list of failed elements.
  {
    size_t nPos = 0;
    for( size_t s = 0; s < oaFileInfo_o.size(); s++ )
      if( oaFileInfo_o[s].fMatch )
        oaFileInfo_o[nPos++] = oaFileInfo_o[s];
    oaFileInfo_o.resize( nPos );
  }

  // Return whether any element was found.
  return !oaFileInfo_o.empty();
}

#endif

// -----------------------------------------------------------------------------

// *****************************************************************
// *                                                               *
// *                     WINDOWS-IMPLEMENTATION                    *
// *                                                               *
// *****************************************************************

#ifdef FILESYS_WINDOWS

// -----------------------------------------------------------------------------

// INCLUSIONS FUER WINDOWS

// -----------------------------------------------------------------------------

#include <windows.h>
#include <string.h>

// -----------------------------------------------------------------------------

// WINDOWS-SPEZIFISCHE GLOBALE MAKROS UND KONSTANTEN

// -----------------------------------------------------------------------------

namespace filesys
{
  static const char cPathSep = '\\';          ///< Path separator character.
  static const char szDevNull[] = "NUL";      ///< Null device string.
}

// -----------------------------------------------------------------------------

// IMPLEMENTATION DER FUNKTIONEN FUER WINDOWS

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Evaluates a canonical path (that may include wildcards) into a
///             list of existing files or directories that exist on the drive.
///
/// \details    The function checks whether files or directories exists on the
///             file system that match the given (absolute or relative) path.
///
///             To verify a match, the function uses the uppercase/lowercase
///             conventions of the specific file system, i.e. it requires an
///             exact match on a UNIX system and a non-case-sensitive match on
///             a Windows system.
///
/// \param[in]  szCanonPath_i
///             A canonical path as it can be retrieved from
///             \ref GetCanonicalPath(). The path can contain the following
///             wildcards:
///             - ? : zero or one unknown character except dot(.)
///             - * : Match any number of unknown characters (regardless of
///                   the position where it appears, including at the start
///                   and/or multiple times)
///             Note that the string is changed by the function.
/// \param[in]  fCaseType_i
///             Flag what letter case convention is used for matching the
///             file names. Possible values are:
///             - case_type::OS_CONVENTION : the convention of the respective
///               operating system is used
///             - case_type::CASE_SENSITIVE : the comparison is performed
///               in a case sensitive form
///             - case_type::CASE_INSENSITIVE : the comparison is performed
///               in a not case sensitive form
/// \param[out] oaFileInfo_o
///             Returns a list of information about the files or directories
///             matching the canonical path name.
// *****************************************************************************

bool filesys::GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o )
{
  // Eventually reset case sensitivity flag.
  if( fCaseType_i == case_type::OS_CONVENTION )
    fCaseType_i = case_type::CASE_INSENSITIVE;

  // Variables of types from <windows.h>.
  HANDLE hDir;
  WIN32_FIND_DATA oEntry;

  // Pointers to the comparison functions (as used and case sensitive version).
  int (*pStrCmp)(const char*, const char*);
  int (*pStrCmpCaseSensitive)(const char*, const char*);

  // Split cursors.
  char* pBeg = szCanonPath_i;
  char* pEnd;

  // Use the output vector as stack for the different results.
  // Start with an initial value.
  oaFileInfo_o.resize( 1 );
  {
    size_t nStrLen = strlen( szCanonPath_i );

    // Identify the absolute path form "/c" and convert it into windows
    // format "C:". Catch forms invalid under windows like "/", "/ca..."
    // etc.
    bool fAbsolute = false;
    bool fInvalid = false;
    if( szCanonPath_i[0] == '/' )
    {
      fAbsolute = true;
      if( nStrLen >= 2 )
        if( szCanonPath_i[1] >= 'a' && szCanonPath_i[1] <= 'z' )
        {
          switch( szCanonPath_i[2] )
          {
            case '\0':
            {
              oaFileInfo_o[0].sPath.push_back( toupper( szCanonPath_i[1] ) );
              oaFileInfo_o[0].sPath += ":"; // TODO Check if this is correct.
              oaFileInfo_o[0].fPathType = path_type::ABS;
              pBeg += 2;
              break;
            }

            case '/':
            {
              oaFileInfo_o[0].sPath.push_back( toupper( szCanonPath_i[1] ) );
              oaFileInfo_o[0].sPath += ":\\";  // TODO Check if this is correct.
              oaFileInfo_o[0].fPathType = path_type::ABS;
              pBeg += 3;
              break;
            }

            default:
              fInvalid = true;
              break;
          }
        }
        else
          fInvalid = true;
      else
        fInvalid = true;
    }

    // If the absolute's path opening was invalid: return without any result.
    if( fInvalid )
    {
      oaFileInfo_o.clear();
      return false;
    }

    // Otherwise set a relative path opening.
    if( !fAbsolute )
    {
      oaFileInfo_o[0].sPath = ".\\";
      oaFileInfo_o[0].fPathType = path_type::REL;
    }
    oaFileInfo_o[0].fOSType = os::WINDOWS;
    oaFileInfo_o[0].fItemType = item_type::DIRECTORY;
    oaFileInfo_o[0].fMatchType = match_type::EXACT;
    oaFileInfo_o[0].fMatch = true;
  }

  // Loop over all directory sections of the path.
  bool fLastSec = false;
  while( !fLastSec )
  {
    // Find next path section and cut it out.
    pEnd = pBeg;
    while( *pEnd != '/' && *pEnd != '\0' )
      pEnd++;
    fLastSec = ( *pEnd == '\0' );
    *pEnd = '\0';

    // Check whether wildcards are involved in this section.
    bool fWildcard = ( strpbrk( pBeg, "?*" ) != NULL );

    // Set pointer to the comparison functions that must be used for this section.
    if( fWildcard )
    {
      switch( fCaseType_i )
      {
        case case_type::CASE_SENSITIVE:
          pStrCmp = &strglobcmp;
          break;

        case case_type::CASE_INSENSITIVE:
          pStrCmp = &strtoupperglobcmp;
          break;
      }

      pStrCmpCaseSensitive = &strglobcmp;
    }
    else
    {
      switch( fCaseType_i )
      {
        case case_type::CASE_SENSITIVE:
          pStrCmp = &strcmp;
          break;

        case case_type::CASE_INSENSITIVE:
          pStrCmp = &strtouppercmp;
          break;
      }

      pStrCmpCaseSensitive = &strcmp;
    }

    // Loop over all elements that are currently in the stack.
    // The stack may get expanded during this process.
    const size_t nStackN = oaFileInfo_o.size();
    for( size_t s = 0; s < nStackN; s++ )
      if( oaFileInfo_o[s].fMatch )
      {
        // Store length of base path string.
        size_t nBasePathLen = oaFileInfo_o[s].sPath.size();

        // Store whether the base path is (up to now) an exact match or
        // whether it matches only case insensitively.
        match_type fBasePathMatchType = oaFileInfo_o[s].fMatchType;

        // Store path type. (Although this cannot change due to the algorithm's
        // construction but it's more consistent to do it here.)
        path_type fBasePathType = oaFileInfo_o[s].fPathType;

        // Append "*" to stimulate a search in the whole directory.
        oaFileInfo_o[s].sPath += "*";

        // Open path section with windows "FindFirstFile()" function.
        bool fFirst = true;
        if( ( hDir = FindFirstFile( oaFileInfo_o[s].sPath.c_str(), &oEntry ) ) != INVALID_HANDLE_VALUE )
        {

          // Remove "*" used for stimulating a directory search.
          oaFileInfo_o[s].sPath.pop_back();

          // Loop over all files within the path section.
          do
          {
            // Distinguish between directory-like items and file-like items.
            // See https://msdn.microsoft.com/en-us/library/windows/desktop/gg258117%28v=vs.85%29.aspx
            item_type fItemType;
            switch( oEntry.dwFileAttributes )
            {
              // Directory.
              case FILE_ATTRIBUTE_DIRECTORY:
                fItemType = item_type::DIRECTORY;
                break;

              // Everything else.
              default:
                fItemType = item_type::FILE;
                break;
            }

            // Check whether the item shall be added to the result list.
            // Add files only when this is the last path section.
            if( ( fItemType == item_type::DIRECTORY ) ||
                ( fItemType == item_type::FILE && fLastSec ) )
              if( pStrCmp( pBeg, oEntry.cFileName ) == 0 )
                if( fFirst )
                {
                  fFirst = false;

                  // => Modify stack element s.

                  // Append entry name and eventually a slash to the path.
                  oaFileInfo_o[s].sPath += oEntry.cFileName;
                  if( fItemType == item_type::DIRECTORY )
                    oaFileInfo_o[s].sPath.push_back( '\\' );

                  // Set item type.
                  oaFileInfo_o[s].fItemType = fItemType;

                  // Check whether the path matches only case insensitively.
                  if( fCaseType_i == case_type::CASE_INSENSITIVE )
                    if( fBasePathMatchType == match_type::EXACT )
                      if( pStrCmpCaseSensitive( pBeg, oEntry.cFileName ) != 0 )
                        oaFileInfo_o[s].fMatchType = match_type::CASE_VAR;
                }
                else
                {
                  // => Append new element to the stack.
                  oaFileInfo_o.resize( oaFileInfo_o.size() + 1 );

                  // Set path name as combination of the base path plus the entry name.
                  oaFileInfo_o.back().sPath.assign( oaFileInfo_o[s].sPath, 0, nBasePathLen );
                  oaFileInfo_o.back().sPath += oEntry.cFileName;
                  if( fItemType == item_type::DIRECTORY )
                    oaFileInfo_o.back().sPath += '\\';

                  // Set operating system dependent format type of the path.
                  oaFileInfo_o.back().fOSType = os::WINDOWS;

                  // Set path type.
                  oaFileInfo_o.back().fPathType = fBasePathType;

                  // Set item type.
                  oaFileInfo_o.back().fItemType = fItemType;

                  // Check whether the path matches exactly or only case insensitively.
                  switch( fCaseType_i )
                  {
                    case case_type::CASE_SENSITIVE:
                      oaFileInfo_o.back().fMatchType = match_type::EXACT;
                      break;

                    case case_type::CASE_INSENSITIVE:
                    {
                      if( fBasePathMatchType == match_type::EXACT )
                      {
                        if( pStrCmpCaseSensitive( pBeg, oEntry.cFileName ) == 0 )
                          oaFileInfo_o.back().fMatchType = match_type::EXACT;
                        else
                          oaFileInfo_o.back().fMatchType = match_type::CASE_VAR;
                      }
                      else
                        oaFileInfo_o.back().fMatchType = match_type::CASE_VAR;
                      break;
                    }
                  }

                  // Set match to true.
                  oaFileInfo_o.back().fMatch = true;
                }
          }
          while( FindNextFile( hDir, &oEntry ) );

          FindClose( hDir );
        }

        // Check if the search returned any match and mark the stack
        // element accordingly.
        if( fFirst )
          oaFileInfo_o[s].fMatch = false;
      }

    // Break off or shift pBeg.
    if( fLastSec )
      break;
    else
      pBeg = pEnd + 1;
  }

  // Clear result list of failed elements.
  {
    size_t nPos = 0;
    for( size_t s = 0; s < oaFileInfo_o.size(); s++ )
      if( oaFileInfo_o[s].fMatch )
        oaFileInfo_o[nPos++] = oaFileInfo_o[s];
    oaFileInfo_o.resize( nPos );
  }

  // Return whether any element was found.
  return !oaFileInfo_o.empty();
}

#endif

// -----------------------------------------------------------------------------

// *****************************************************************
// *                                                               *
// *                    MAC-OS-X-IMPLEMENTATION                    *
// *                                                               *
// *****************************************************************

#ifdef FILESYS_MAC_OS_X

#endif

// -----------------------------------------------------------------------------

// *****************************************************************************
// *                                                                           *
// *                       OS INDEPENDENT IMPLEMENTATIONS                      *
// *                                                                           *
// *****************************************************************************

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Renders the C string uppercase.
// *****************************************************************************

inline char* filesys::strtoupper( char* szString_i )
{
  for( char* c = szString_i; *c; ++c )
    *c = toupper( *c );
  return szString_i;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of strcmp() that compares non-case-sensitive.
// *****************************************************************************

int filesys::strtouppercmp( const char* szString1_i, const char* szString2_i )
{
  char* p1 = (char*) szString1_i;
  char* p2 = (char*) szString2_i;
  unsigned char c1, c2;
  do
  {
    c1 = (unsigned char) toupper( *p1++ );
    c2 = (unsigned char) toupper( *p2++ );
    if( c1 == '\0' || c2 == '\0' )
      return c1 - c2;
  }
  while( c1 == c2 );

  return c1 - c2;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of strcmp() that compares under consideration of
///             wildcards (globbing) in the first string.
///
/// \param[in]  szString1_i
///             String that may include wildcards '*' and '?'.
/// \param[in]  szString2_i
///             Comparison string.
///
/// \return     Returns zero if the contents of both strings with respect to
///             globbing are equal and a non-zero value otherwise.
// *****************************************************************************

int filesys::strglobcmp( const char* szString1_i, const char* szString2_i )
{
  char* p1 = (char*) szString1_i;
  char* p2 = (char*) szString2_i;
  unsigned char c1, c2;
  do
  {
    c1 = (unsigned char) *p1;
    c2 = (unsigned char) *p2;
    switch( c1 )
    {
      case '?':
        if( *++p1 == '\0' )
          if( c2 == '\0' )
            return 0;
          else
            return - (unsigned char) *++p2;
        else
          if( c2 == '\0' )
            return (unsigned char) *p1;
          else
            if( strglobcmp( p1, p2 ) == 0 )
              return 0;
            else
              return strglobcmp( p1, ++p2 );
        break;

      case '*':
        while( *++p1 == '*' ); // Skip multiple asteriks.
        if( *p1 == '\0' )
          return 0;
        else
          if( c2 == '\0' )
            return (unsigned char) *p1;
          else
            if( strglobcmp( p1, p2 ) == 0 )
              return 0;
            else
              return strglobcmp( --p1, ++p2 );
        break;

      default:
        if( c1 == '\0' || c2 == '\0' )
          return c1 - c2;
    }

    p1++;
    p2++;
  }
  while( c1 == c2 );

  return c1 - c2;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of strcmp() that compares non-case-sensitive under
///             consideration of wildcards (globbing) in the first string.
///
/// \param[in]  szString1_i
///             String that may include wildcards '*' and '?'.
/// \param[in]  szString2_i
///             Comparison string.
///
/// \return     Returns zero if the contents of both strings with respect to
///             globbing are equal and a non-zero value otherwise.
// *****************************************************************************

int filesys::strtoupperglobcmp( const char* szString1_i, const char* szString2_i )
{
  char* p1 = (char*) szString1_i;
  char* p2 = (char*) szString2_i;
  unsigned char c1, c2;
  do
  {
    c1 = (unsigned char) toupper( *p1 );
    c2 = (unsigned char) toupper( *p2 );
    switch( c1 )
    {
      case '?':
        if( *++p1 == '\0' )
          if( c2 == '\0' )
            return 0;
          else
            return - (unsigned char) toupper( *++p2 );
        else
          if( c2 == '\0' )
            return (unsigned char) toupper( *p1 );
          else
            if( strtoupperglobcmp( p1, p2 ) == 0 )
              return 0;
            else
              return strtoupperglobcmp( p1, ++p2 );
        break;

      case '*':
        while( *++p1 == '*' ); // Skip multiple asteriks.
        if( *p1 == '\0' )
          return 0;
        else
          if( c2 == '\0' )
            return (unsigned char) toupper( *p1 );
          else
            if( strtoupperglobcmp( p1, p2 ) == 0 )
              return 0;
            else
              return strtoupperglobcmp( --p1, ++p2 );
        break;

      default:
        if( c1 == '\0' || c2 == '\0' )
          return c1 - c2;
    }

    p1++;
    p2++;
  }
  while( c1 == c2 );

  return c1 - c2;
}

// -----------------------------------------------------------------------------

// TESTING FUNCTION GetCanonicalPath()

// -----------------------------------------------------------------------------

/* canonize_path.cpp:
#include <iostream>
#include "./src/independent/filesys.h"
int main( int argc, char* argv[] )
{
  if( argc < 3 ){
    std::cerr << "Usage: canonize_path [UNIX/WINDOWS] [path]" << std::endl;
    return -1;
  }
  filesys::os fOSType = ( strcmp( argv[1], "UNIX" ) == 0 ) ? filesys::os::UNIX : filesys::os::WINDOWS;
  std::string sResult;
  bool fResult = filesys::GetCanonicalPath( argv[2], sResult, fOSType );
  std::cout << "input  : \"" << argv[2] << "\"" << std::endl
            << "success: " << std::boolalpha << fResult << std::endl
            << "output : \"" << sResult << "\"" << std::endl
            << std::endl;
  return 0;
}
*/

/* test_unix.sh:
#!/bin/bash
clear
echo "Default action:"
./canonize_path UNIX ""
./canonize_path UNIX "."
./canonize_path UNIX "/"
./canonize_path UNIX "/home/idallen/bin/file.txt"
./canonize_path UNIX "bin/file.txt"
./canonize_path UNIX "../bin/file.txt"
./canonize_path UNIX "./file.txt"
./canonize_path UNIX "file.txt"
./canonize_path UNIX ".file"
./canonize_path UNIX "file."
./canonize_path UNIX "..file"
./canonize_path UNIX "file.."
./canonize_path UNIX "a./b"
echo "Unicode characters:"
./canonize_path UNIX "./ÿ-y-umlaut"
./canonize_path UNIX "./£"
./canonize_path UNIX "./€"
./canonize_path UNIX "./™"
echo "Whitespaces:"
./canonize_path UNIX " "
./canonize_path UNIX "/home/idallen/my folder"
echo "Multiple slashes and trailing slashes:"
./canonize_path UNIX "///"
./canonize_path UNIX "/home//idallen//file.txt"
./canonize_path UNIX "..///bin////file.txt"
./canonize_path UNIX "/home/"
./canonize_path UNIX "/home//"
./canonize_path UNIX "/home///"
echo "Single dots:"
./canonize_path UNIX "./"
./canonize_path UNIX "./home"
./canonize_path UNIX "././home"
./canonize_path UNIX "/home/."
./canonize_path UNIX "/home///."
./canonize_path UNIX "././././/./..//../home/.//./idallen/bin/././././"
echo "Double dots:"
./canonize_path UNIX ".."
./canonize_path UNIX "../"
./canonize_path UNIX "../../../.."
./canonize_path UNIX "../../../../"
./canonize_path UNIX "/file../"
./canonize_path UNIX "/file../a"
./canonize_path UNIX "/home/iallen/../a/.."
./canonize_path UNIX "../home"
./canonize_path UNIX "../home/../../"
./canonize_path UNIX "./../home"
./canonize_path UNIX "././/../home"
./canonize_path UNIX "/home/..//idallen/..//."
./canonize_path UNIX "home/idallen/../../../../ignacio/bin"
./canonize_path UNIX "/home/../idallen/."
./canonize_path UNIX "../home/..//idallen/../..//."
./canonize_path UNIX "./././././../../tmp/idallen/././././"
./canonize_path UNIX "../../home/idallen/../johndoe/prog/../major/../..//ignacio/bin"
./canonize_path UNIX "../home/../../ignacio"
echo "Invalid paths:"
./canonize_path UNIX "/.."
./canonize_path UNIX "/../"
./canonize_path UNIX "/home/..//idallen/../..//."
*/

/* test_windows.sh:
#!/bin/bash
clear
echo "Default action:"
./a.out WINDOWS "c:"
./a.out WINDOWS "C:"
./a.out WINDOWS "C:\\"
./a.out WINDOWS "C:/"
./a.out WINDOWS "C:\\home\\idallen\\bin\\file.txt"
./a.out WINDOWS "bin\\file.txt"
./a.out WINDOWS "..\\bin\\file.txt"
./a.out WINDOWS ".\\file.txt"
./a.out WINDOWS "file.txt"
./a.out WINDOWS ".file"
./a.out WINDOWS "file."
./a.out WINDOWS "..file"
./a.out WINDOWS "file.."
./a.out WINDOWS "a.\\b"
echo "Unicode characters:"
./a.out WINDOWS ".\\ÿ-y-umlaut"
./a.out WINDOWS ".\\£"
./a.out WINDOWS ".\\€"
./a.out WINDOWS ".\\™"
echo "Slashes instead backslashes:"
./a.out WINDOWS "C:/home/idallen/bin/file.txt"
./a.out WINDOWS "bin/file.txt"
echo "Multiple backslashes and trailing backslashes:"
./a.out WINDOWS "C:\\\\\\"
./a.out WINDOWS "C:\\home\\\\idallen\\\\file.txt"
./a.out WINDOWS "..\\\\\\bin\\\\\\\\file.txt"
./a.out WINDOWS "C:\\home\\"
./a.out WINDOWS "C:\\home\\\\"
./a.out WINDOWS "C:\\home\\\\\\"
*/

// **** Ende Header ****
#endif // FILESYS_READ_ONCE
