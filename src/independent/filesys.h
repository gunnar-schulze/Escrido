#ifndef FILESYS_READ_ONCE
#define FILESYS_READ_ONCE

// *****************************************************************************
/// \file       filesys.h
///
/// \brief      Provides an abstraction layer for OS-independent file system
///             access (including wildcards).
///
/// \author     Gunnar Schulze
/// \version    2021-02-18
/// \date       2012
/// \copyright  2016 Gunnar Schulze
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

#include <cstdlib>           // malloc
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

// *********************
// *                   *
// *     iflstream     *
// *                   *
// *********************

// *********************
// *                   *
// *     oflstream     *
// *                   *
// *********************

/// Plattform independent file operations namespace.
namespace filesys
{
  struct SFileInfo;

  // Locked input and output streams on files.
  class iflstream;
  class oflstream;
}

// -----------------------------------------------------------------------------

// MACROS

// -----------------------------------------------------------------------------

namespace filesys
{
  /// Operating systems depending format type of paths.
  enum os : unsigned char
  {
    THIS,      ///< This build's operating system (see OPERATING SYSTEM SETTING).
    UNIX,      ///< Unix, Mac OS X.
    WINDOWS    ///< DOS/Windows.
  };

  /// Path name capitalization cases.
  enum case_type : unsigned char
  {
    OS_CONVENTION,     ///< Use capitalization convention of the compiling OS.
    CASE_SENSITIVE,    ///< Use case sensitive capitalization convention.
    CASE_INSENSITIVE   ///< Use case insensitive capitalization convention.
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

  /// Type of behavior on lock acquisition for iflstream::open() and oflstream::open().
  enum lock_action : unsigned char
  {
    WAIT,
    RETURN
  };
}

// -----------------------------------------------------------------------------

// OPERATING SYSTEM SETTING

// -----------------------------------------------------------------------------

// Precompiler switch for setting the operating system.
// One of these should be set on compiling:
//#define FILESYS_UNIX
//#define FILESYS_WINDOWS

// -----------------------------------------------------------------------------

// FUNCTIONS OVERVIEW

// -----------------------------------------------------------------------------

namespace filesys
{
  // Path names: converting from and to canonical form:
  bool GetCanonicalPath( const char* szInPath_i, char* szCanonPath_o, os fOSType_i = os::THIS );
  bool GetCanonicalPath( const char* szInPath_i, std::string& sCanonPath_o, os fOSType_i = os::THIS );
  bool IsCanonicalPath( const char* szPath_i );
  bool GetOSFormatedPath( const char* szCanonInPath_i, char* szOutPath_o, os fOSType_i = os::THIS );
  bool GetOSFormatedPath( const char* szCanonInPath_i, std::string& sOutPath_o, os fOSType_i = os::THIS );

  // Canonical paths: checks and manipulation:
  bool IsRelativePath( const char* szCanonPath_i );
  void ConcatPaths( const char* szCanonDir_i, const char* szCanonPath_i, char* szCanonOutPath_o );
  void ConcatPaths( const char* szCanonDir_i, const char* szCanonPath_i, std::string& sCanonOutPath_o );

  // Accessing files and directories on the drive:
  bool GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o );
  bool GetFilesInfo( const char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o );

  // Helper functions:
  char* strtoupper( char* szString_i );
  int   strtouppercmp( const char* szString1_i, const char* szString2_i );
  int   strglobcmp( const char* szString1_i, const char* szString2_i );
  int   strtoupperglobcmp( const char* szString1_i, const char* szString2_i );

  void OpenModeConvert( std::ios_base::openmode fOpenMode_i, char* szMode_o );
}

// -----------------------------------------------------------------------------

// STRUCT SFileInfo

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Information structure of a file or directory on the file system.
// *****************************************************************************

struct filesys::SFileInfo
{
  std::string sPath;      ///< Path of the file or directory in OS specific format. (Note that this is <em>not</em> a canonical path.)
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
///             - \b slashes ('/') as directory separators
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

inline bool filesys::GetCanonicalPath( const char* szInPath_i, char* szCanonPath_o, os fOSType_i )
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
  const char *pScan;
  const char *pNext;
  char* pWrite;

  // Step 1: create standardized paths for different OS.
  switch( fOSType_i )
  {
    case os::UNIX:
    {
      // Build a copy of the path string.
      pScan = szInPath_i;
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

inline bool filesys::GetCanonicalPath( const char* szInPath_i, std::string& sCanonPath_o, os fOSType_i )
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

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Creates a path of the <em>operating system specific format</em>
///             from an canonical path.
///
/// \details    The function converts a given <em>canonical path</em> (see
///             \ref GetCanonicalPath() ) into a path that has the format of
///             a specified operating system.
///
/// \param[in]  szCanonInPath_i
///             Input path C string. This can be an absolute or relative
///             canonical path as it can be retrieved from
///             \ref GetCanonicalPath().
/// \param[out] szOutPath_o
///             C string that returns the path name in the convention of the
///             operating system defined by fOSType_i.
///             This must point to a memory block with a size of szCanonInPath_i
///             <b>plus two bytes</b> or larger. The pointer may be
///             identical to szCanonInPath_i.
/// \param[in]  fOSType_i
///             Defines the operating system convention of szOutPath_o. Allowed
///             values are:
///             - os::THIS    : this compile's operating system conventions
///             - os::UNIX    : Unix or Mac OS X path conventions
///             - os::WINDOWS : DOS/Windows path conventions
///
/// \return     'true' if the input string could validly be converted,
///             'false' otherwise.
// *****************************************************************************

inline bool filesys::GetOSFormatedPath( const char* szCanonInPath_i, char* szOutPath_o, os fOSType_i )
{
  // Eventually reset OS type.
  if( fOSType_i == os::THIS )
#ifdef FILESYS_UNIX
    fOSType_i = os::UNIX;
#endif
#ifdef FILESYS_WINDOWS
    fOSType_i = os::WINDOWS;
#endif

  // Step 1: create standardized paths for different OS.
  switch( fOSType_i )
  {
    case os::UNIX:
    {
     // Get string length.
      size_t nStrLen = strlen( szCanonInPath_i );

      // Check whether the input path does not begin with a leading slash (i.e.
      // if it is relative) or with a double dots operator ("../").
      bool fPureRel = true;
      if( nStrLen > 0 )
        if( szCanonInPath_i[0] == '/' )
          fPureRel = false;
        else
          if( nStrLen > 2 )
            if( szCanonInPath_i[0] == '.' && szCanonInPath_i[1] == '.' && szCanonInPath_i[2] == '/' )
              fPureRel = false;

      // Add leading "./" to the string, if it is "purely relative".
      if( fPureRel )
      {
        memcpy( szOutPath_o + 2, szCanonInPath_i, nStrLen + 1 );
        strncpy( szOutPath_o, "./", 2 );
      }
      else
        strcpy( szOutPath_o, szCanonInPath_i );

      return true;
    }

    case os::WINDOWS:
    {
      // Get string length.
      size_t nStrLen = strlen( szCanonInPath_i );

      // Identify windows type absolute paths of the forms "/", "/c" and "/c/" and
      // convert them into the form "C:/".
      const char* pScan = szCanonInPath_i;
      char* pWrite = szOutPath_o;
      if( nStrLen == 0 )
      {
        // => Empty string.
        *pWrite = '\0';
        return true;
      }
      else
      {
        if( *pScan == '/' )
        {
          pScan++;

          if( nStrLen == 1 )
          {
            // => Form "/".
            strcpy( pWrite, "C:\\" );
            return true;
          }

          if( ( *pScan >= 'a' && *pScan <= 'z' ) ||
              ( *pScan >= 'A' && *pScan <= 'Z' ) )
          {
            if( nStrLen == 2 )
            {
              // => Form "/c".
              *pWrite = toupper( *pScan );
              strcpy( pWrite+1, ":\\" );
              return true;
            }
            else
            {
              if( *(pScan+1) == '/' )
              {
                // Forms "/c/" and "/c/...".
                *pWrite = toupper( *pScan );
                strncpy( pWrite+1, ":\\", 2 );

                pScan += 2;
                pWrite += 3;
              }
              else
                return false;
            }
          }
          else
            return false;
        }
      }

        // Copy string while converting slashes into backslashes.
      while( *pScan != '\0' )
      {
        if( *pScan == '/' )
          *pWrite++ = '\\';
        else
          *pWrite++ = *pScan;

        pScan++;
      }
      *pWrite = '\0';

      return true;
    }

    default:
      return false;
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of \ref GetOSFormatedPath( const char*, char*, os ) that
///             accepts a string as return variable.
// *****************************************************************************

inline bool filesys::GetOSFormatedPath( const char* szCanonInPath_i, std::string& sOutPath_o, os fOSType_i )
{
  size_t nInLen = strlen( szCanonInPath_i );
  char* sBuf = (char*) malloc( sizeof( char ) * ( nInLen + 3 ) );
  if( sBuf == NULL )
    throw std::bad_alloc();
  bool fResult = GetOSFormatedPath( szCanonInPath_i, sBuf, fOSType_i );
  sOutPath_o.assign( sBuf );
  free( sBuf );

  return fResult;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns whether a given canonical path is relative.
///
/// \param[in]  szCanonPath_i
///             Input path C string. This can be an absolute or relative
///             canonical path as it can be retrieved from
///             \ref GetCanonicalPath().
///
/// \return     'true' is the input path is relative, 'false' otherwise.
// *****************************************************************************

inline bool filesys::IsRelativePath( const char* szCanonPath_i )
{
  size_t nInLen = strlen( szCanonPath_i );
  if( nInLen == 0 )
    return true;
  else
    if( szCanonPath_i[0] == '/' )
      return false;
    else
      return true;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Concatenates a canonical directory string with a canonical path
///             string.
///
/// \details    This function is useful for concatenating a canonical relative
///             path to a canonical directory in a way as if the path is
///             relative to the directory. In a simple case this is just a
///             string attachment. In more complex cases in which the path
///             contains double dots ('..') some or all parts of the directory
///             string are removed accordingly.
///
/// \param[in]  szCanonDir_i
///             A canonical C-string defining a <em>directory</em>.
/// \param[in]  szCanonPath_i
///             A canonical C-string defining a <em>path</em>. The function's
///             intention is that this path is relative in order to allow
///             combinations with the directory string. However it may also be
///             absolute. Then, the function's result will be a direct copy of
///             szCanonPath_i.
/// \param[out] szCanonOutPath_o
///             Returns a <em>logical combination</em> of the input directory
///             and the input path.
// *****************************************************************************

inline void filesys::ConcatPaths( const char* szCanonDir_i, const char* szCanonPath_i, char* szCanonOutPath_o )
{
  // Get lengths of input directory and path.
  size_t nCanonDirLen = strlen( szCanonDir_i );
  size_t nCanonPathLen = strlen( szCanonPath_i );

  // Check whether input path is relative or absolute.
  if( nCanonPathLen == 0 )
  {
    // => Path is empty, i.e. the relative path to "here".

    // Copy input directory into output.
    strcpy( szCanonOutPath_o, szCanonDir_i );
  }
  else
    if( szCanonPath_i[0] == '/' )
    {
      // => Path is non-empty and absolute.

      // Copy input path into output.
      strcpy( szCanonOutPath_o, szCanonPath_i );
    }
    else
    {
      // => Path is non-empty and relative.

      if( nCanonDirLen == 0 )
      {
        // => Input directory is empty, i.e. the relative path to "here".

        // Copy input path into output.
        strcpy( szCanonOutPath_o, szCanonPath_i );
      }
      else
      {
        // => Input directory is non-empty.

        if( nCanonDirLen == 1 && ( szCanonDir_i[0] == '/' ) )
        {
          // => Input directory is exceptional case "/".

          szCanonOutPath_o[0] = '/';
          szCanonOutPath_o[1] = 0;
          strcat( szCanonOutPath_o, szCanonPath_i );
        }
        else
        {
          // => Input directory is non-empty and not bare root.

          // Copy input directory into output directory. This is then used for
          // further modifications, if those are required.
          strcpy( szCanonOutPath_o, szCanonDir_i );

          // A pointer running over the input path.
          const char* szPath = szCanonPath_i;

          // Resolve all double-dots at the beginning of the input path.
          while( nCanonPathLen >= 2 && nCanonDirLen != 0 )
          {
            // Check whether the input path begins with a directory decrementor (".." or "../*")
            bool fDecrDir = false;
            if( szPath[0] == '.' && szPath[1] == '.' )
              if( nCanonPathLen == 2 )
                fDecrDir = true;
              else
                if( szPath[2] == '/' )
                  fDecrDir = true;

            if( fDecrDir )
            {
              // => The input path begins with a double-dot.

              // Search last directory delimiter in input directory.
              char* pDelim = strrchr( szCanonOutPath_o, '/' );
              if( pDelim == NULL )
              {
                // => Input directory does not contain no directory delimiter,
                //    i.e. it is of the form "", "a" or ".."

                // Check whether the input directory consists of double-dots.
                if( nCanonDirLen == 2 )
                  if( szCanonOutPath_o[0] == '.' && szCanonOutPath_o[1] == '.' )
                  {
                    // => Input directory has the form "..".
                    //    Directory and path can now be concatenated directly.

                    break;
                  }

                // Remove last directory of input directory <=> empty input directory
                // string completely.
                szCanonOutPath_o[0] = 0;
                nCanonDirLen = 0;
              }
              else
              {
                // => Input directory contains at least one directory delimiter,
                //    i.e. it is of the forms "/", "a/b", "../a" or "../.." etc.

                // Calculate length of trailing section after last delimiter.
                size_t nLenAfterDelim = nCanonDirLen - ( pDelim - szCanonOutPath_o ) - 1;

                // Check whether last section of input directory are double-dots.
                if( nLenAfterDelim == 2 )
                  if( pDelim[1] == '.' && pDelim[2] == '.' )
                  {
                    // => Input directory has the form "*/.." and for canonical paths
                    //    this means that it can only have any of the exact forms of
                    //    "../..", "../../.." etc.
                    //    Directory and path can now be concatenated directly.

                    break;
                  }

                // => Input directory has any of the forms "/", "a/b" or "../a" etc.

                // Cut off last delimiter of input directory and anything behind it.
                *pDelim = 0;
                nCanonDirLen -= nLenAfterDelim + 1;
              }

              // Cut off double-dots at the beginning of input path.
              if( nCanonPathLen == 2 )
              {
                // => Input path has the form ".."

                szPath += 2;
                nCanonPathLen = 0;
              }
              else
              {
                // => Input path has the form "../" or "../*"

                szPath += 3;
                nCanonPathLen -= 3;
              }
            }
            else
              break;
          }

          // Append input path to output.
          if( nCanonDirLen * nCanonPathLen > 0 )
            strcat( szCanonOutPath_o, "/" );
          strcat( szCanonOutPath_o, szPath );
        }
      }
    }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Variant of \ref ConcatPaths( const char*, const char*, char* )
///             that accepts a string as return variable.
// *****************************************************************************

inline void filesys::ConcatPaths( const char* szCanonDir_i, const char* szCanonPath_i, std::string& sCanonOutPath_o )
{
  size_t nInLen = strlen( szCanonDir_i ) + strlen( szCanonPath_i );
  char* sBuf = (char*) malloc( sizeof( char ) * ( nInLen + 2 ) );
  if( sBuf == NULL )
    throw std::bad_alloc();
  ConcatPaths( szCanonDir_i, szCanonPath_i, sBuf );
  sCanonOutPath_o.assign( sBuf );
  free( sBuf );
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

// iflstream, oflstream: locking input/output streams on files:
#include <cstdio>               // FILE*, fopen()
#include <fcntl.h>              // flock, fcntl()
#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf

// -----------------------------------------------------------------------------

// UNIX: GLOBAL MACROS AND CONSTANTS

// -----------------------------------------------------------------------------

namespace filesys
{
  const char cSeparator = '/';               ///< Path separator character.
  const char szDevNull[] = "/dev/null";      ///< Null device string.
}

// -----------------------------------------------------------------------------

// UNIX: CLASS iflstream

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      An input stream similar to std::ifstream that applies an
///             advisory read-lock (non-exclusive locking) to the files.
///
/// \details    The file access through this class is managend by advisory file
///             locks. Multiple instances of iflstream can access the file at
///             the same time for reading (non-exclusive locking) while writing
///             operations through \ref filesys::oflstream are prohibited during
///             that time.
///
/// \remark     The lock applied to the file is <em>advisory</em> only, meaning
///             that other objects (as \ref std::ofstream) may ignore it and
///             perform writing operations regardless the lock.
// *****************************************************************************

class filesys::iflstream : public std::istream
{
  private:

    std::FILE* pFile;
    __gnu_cxx::stdio_filebuf<char>* pFileBuf;

  public:

    // Constructors, destructor:
    iflstream();
    explicit iflstream( const char* szFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    explicit iflstream( const std::string& sFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    ~iflstream();

    // Locked file opening and closing.
    void open( const char* szFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    void open( const std::string& sFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    void close();
};

// .............................................................................

// *****************************************************************************
/// \brief      Default constructor.
///
/// \details    Constructs an iflstream object that is not associated with any file.
// *****************************************************************************

inline filesys::iflstream::iflstream() :
  pFileBuf ( NULL )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Initialization constructor.
///
/// \details    Constructs an iflstream object, initially associated with the
///             file identified by its file name argument locked
///             open with the given open mode.
///
///             If the creation succeeds, the file is opend with a read lock
///             (non-exclusive lock). The read lock is removed on a call to
///             \ref iflstream::close() or on destruction of the object.
///
/// \param[in]  szFilePath_i
///             C-string defining a path to the file to be opened.
/// \param[in]  fAction_i
///             Defines the behavior of the function during locking, if the
///             lock cannot be acquired because a concurring lock is set by
///             another application:
///             - lock_action::WAIT:   waits until the lock to the file can be
///                                    set.
///             - lock_action::RETURN: returns immediately with the file not
///                                    beeing opened.
/// \param[in]  fOpenMode_i
///             Open mode flag as used in std::ifstream.
// *****************************************************************************

inline filesys::iflstream::iflstream( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  pFileBuf ( NULL )
{
  // Call open():
  this->open( szFilePath_i, fAction_i, fOpenMode_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Initialization constructor.
///
/// \details    Variant of
///             \ref iflstream::iflstream( const char*, lock_action, std::ios_base::openmode )
///             that accepts the file name as an std::string object.
// *****************************************************************************

inline filesys::iflstream::iflstream( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  iflstream( sFilePath_i.c_str(), fAction_i, fOpenMode_i )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Destructor.
///
/// \details    If a file is open, the read lock is removed and the file is closed.
// *****************************************************************************

inline filesys::iflstream::~iflstream()
{
  this->close();
}

// .............................................................................

// *****************************************************************************
/// \brief      Opens the file identified by the file name argument,
///             associating it with the stream object, so that input/output
///             operations are performed on its content.
///
/// \details    If the creation succeeds, the file is opend with a read lock
///             (non-exclusive lock). The read lock is removed on a call to
///             \ref iflstream::close() or on destruction of the object.
///
///             If the stream is already associated with a file (i.e., it is
///             already open), calling this function fails.
///
/// \param[in]  szFilePath_i
///             C-string defining a path to the file to be opened.
/// \param[in]  fAction_i
///             Defines the behavior of the function during locking, if the
///             lock cannot be acquired because a concurring lock is set by
///             another application:
///             - lock_action::WAIT:   waits until the lock to the file can be set.
///             - lock_action::RETURN: returns immediately with the file not beeing opened.
/// \param[in]  fOpenMode_i
///             Open mode flag as used in std::ifstream.
///
/// \return     None. If the function fails to open a file, the failbit state
///             flag is set for the stream (which may throw ios_base::failure
///             if that state flag was registered using member exceptions).
// *****************************************************************************

inline void filesys::iflstream::open( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  // Check if the stream is already associated with a file and eventually fail
  // (as if std::ifstream::open() would do on beeing alread opened).
  if( pFileBuf != NULL )
  {
    // Set failbit as in std::ifstream::open().
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }

  // Enforce flag value "in" for the open mode.
  fOpenMode_i |= std::ios_base::in;

  // Convert ofstream-open-mode flag into fopen-mode string.
  char sMode[4];
  OpenModeConvert( fOpenMode_i, sMode );

  // Try opening the file.
  // Using a C file stream is required here since the UNIX file locking operation
  // fcntl() only works on file descriptors as returned by fileno().
  pFile = fopen( szFilePath_i, sMode );
  if( pFile == NULL )
  {
    // Set failbit as in std::ifstream::open().
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }

  // Get its file descriptor using the POSIX function fileno().
  int nFileDescr = fileno( pFile );

  // Use UNIX fcntl() to try to aquire an advisory read lock for the full file.
  // See e.g. https://linux.die.net/man/2/fcntl
  {
    // Prepare lock info structure.
    flock oLock;
    oLock.l_type = F_RDLCK;
    oLock.l_whence = SEEK_SET;
    oLock.l_start = 0;
    oLock.l_len = 0;

    // Set the command accordingly to the desired behavior if a lock cannot be
    // acquired: either return immediately or wait.
    int nCmd;
    switch( fAction_i )
    {
      case lock_action::WAIT:
        nCmd = F_SETLKW;
        break;

      case lock_action::RETURN:
        nCmd = F_SETLK;
        break;
    }

    // Try to acquire read lock.
    if( fcntl( nFileDescr, nCmd, &oLock ) == -1 )
    {
      // => The command failed. This could either be because a concurrent
      //    application own the lock (F_SETLK) or because the program has
      //    caught a signal (e.g. break-off signal 15) during waiting (F_SETLKW).

      // Close file, set failbit and return.
      fclose( pFile );
      this->setstate( this->rdstate() | std::ios_base::failbit );
      return;
    }
  }

  // Create a new filebuffer object and open the stream.
  // NOTE: Instead of using a file buffer of type std::filebuf, we need to use
  //       the libstdc++-dependent version __gnu_cxx::stdio_filebuf here since
  //       this is the only one that accepts a file descriptor.
  //       See https://gcc.gnu.org/onlinedocs/gcc-4.6.2/libstdc++/api/a00069.html
  pFileBuf = new __gnu_cxx::stdio_filebuf<char>( nFileDescr, fOpenMode_i );

  // Check if the file buffer is validly open.
  if( pFileBuf->is_open() )
  {
    // => Everything is fine.

    // Set the buffer as stream buffer to the istream.
    this->rdbuf( pFileBuf );
  }
  else
  {
    // => An error has occured during opening the file buffer.

    // Delete file buffer and reset its pointer's value to NULL.
    delete pFileBuf;
    pFileBuf = NULL;

    // Close file, set failbit and return.
    fclose( pFile );
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Opens the file identified by the file name argument,
///             associating it with the stream object, so that input/output
///             operations are performed on its content.

/// \details    Variant of
///             \ref iflstream::open( const char*, lock_action, std::ios_base::openmode )
///             that accepts the file name as an std::string object.
// *****************************************************************************

inline void filesys::iflstream::open( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  this->open( sFilePath_i.c_str(), fAction_i, fOpenMode_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Closes the file currently associated with the object,
///             disassociating it from the stream.
// *****************************************************************************

inline void filesys::iflstream::close()
{
  // Unset the stream buffer of the input stream.
  this->rdbuf( NULL );

  // Delete the std::filebuf object.
  if( pFileBuf != NULL )
  {
    // Use UNIX fcntl() to unlock the full file.
    // See e.g. https://linux.die.net/man/2/fcntl
    {
      // Prepare lock info structure.
      flock oLock;
      oLock.l_type = F_UNLCK;
      oLock.l_whence = SEEK_SET;
      oLock.l_start = 0;
      oLock.l_len = 0;

      // Unlock the file.
      fcntl( fileno( pFileBuf->file() ), F_SETLK, &oLock );
    }

    // Delete the file buffer. Set its pointer back to NULL.
    // Note that deleting the file buffer SHOULD implicitly close the file
    // (like a call to fclose()) but it was found that it does not.
    delete pFileBuf;
    pFileBuf = NULL;

    // Close file explicitly.
    fclose( pFile );
  }
}

// -----------------------------------------------------------------------------

// UNIX: CLASS oflstream

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      An output stream similar to std::ofstream that applies an
///             advisory write-lock (exclusive locking) to the files.
///
/// \details    The file access through this class is managend by advisory file
///             locks. Only one instances of oflstream can access the file at
///             the same time for wrtiting (exclusive locking) while other
///             reading and writing operations through \ref filesys::iflstream
///             and \ref filesys::oflstream are prohibited during that time.
///
/// \remark     The lock applied to the file is <em>advisory</em> only, meaning
///             that other objects (as \ref std::ofstream) may ignore it and
///             perform writing operations regardless the lock.
// *****************************************************************************

class filesys::oflstream : public std::ostream
{
  private:

    std::FILE* pFile;
    __gnu_cxx::stdio_filebuf<char>* pFileBuf;

  public:

    // Constructors, destructor:
    oflstream();
    explicit oflstream( const char* szFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    explicit oflstream( const std::string& sFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    ~oflstream();

    // Locked file opening and closing.
    void open( const char* szFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    void open( const std::string& sFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    void close();
};

// .............................................................................

// *****************************************************************************
/// \brief      Default constructor.
///
/// \details    Constructs an oflstream object that is not associated with any file.
// *****************************************************************************

inline filesys::oflstream::oflstream() :
  pFileBuf ( NULL )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Initialization constructor.
///
/// \details    Constructs an oflstream object, initially associated with the
///             file identified by its file name argument locked
///             open with the given open mode.
///
///             If the creation succeeds, the file is opend with a write lock
///             (exclusive lock). The write lock is removed on a call to
///             \ref oflstream::close() or on destruction of the object.
///
/// \param[in]  szFilePath_i
///             C-string defining a path to the file to be opened.
/// \param[in]  fAction_i
///             Defines the behavior of the function during locking, if the
///             lock cannot be acquired because a concurring lock is set by
///             another application:
///             - lock_action::WAIT:   waits until the lock to the file can be set.
///             - lock_action::RETURN: returns immediately with the file not beeing opened.
/// \param[in]  fOpenMode_i
///             Open mode flag as used in std::ofstream.
// *****************************************************************************

inline filesys::oflstream::oflstream( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  pFileBuf ( NULL )
{
  // Call open():
  this->open( szFilePath_i, fAction_i, fOpenMode_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Initialization constructor.
///
/// \details    Variant of
///             \ref oflstream::oflstream( const char*, lock_action, std::ios_base::openmode )
///             that accepts the file name as an std::string object.
// *****************************************************************************

inline filesys::oflstream::oflstream( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  oflstream( sFilePath_i.c_str(), fAction_i, fOpenMode_i )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Destructor.
///
/// \details    If a file is open, the write lock is removed and the file is closed.
// *****************************************************************************

inline filesys::oflstream::~oflstream()
{
  this->close();
}

// .............................................................................

// *****************************************************************************
/// \brief      Opens the file identified by the file name argument,
///             associating it with the stream object, so that input/output
///             operations are performed on its content.
///
/// \details    If the creation succeeds, the file is opend with a write lock
///             (exclusive lock). The write lock is removed on a call to
///             \ref oflstream::close() or on destruction of the object.
///
///             If the stream is already associated with a file (i.e., it is
///             already open), calling this function fails.
///
/// \param[in]  szFilePath_i
///             C-string defining a path to the file to be opened.
/// \param[in]  fAction_i
///             Defines the behavior of the function during locking, if the
///             lock cannot be acquired because a concurring lock is set by
///             another application:
///             - lock_action::WAIT:   waits until the lock to the file can be set.
///             - lock_action::RETURN: returns immediately with the file not beeing opened.
/// \param[in]  fOpenMode_i
///             Open mode flag as used in std::ofstream.
///
/// \return     None. If the function fails to open a file, the failbit state
///             flag is set for the stream (which may throw ios_base::failure
///             if that state flag was registered using member exceptions).
// *****************************************************************************

inline void filesys::oflstream::open( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  // Check if the stream is already associated with a file and eventually fail
  // (as if std::ofstream::open() would do on beeing alread opened).
  if( pFileBuf != NULL )
  {
    // Set failbit as in std::ofstream::open().
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }

  // Enforce flag value "out" for the open mode.
  fOpenMode_i |= std::ios_base::out;

  // Convert ofstream-open-mode flag into fopen-mode string.
  char sMode[4];
  OpenModeConvert( fOpenMode_i, sMode );

  // Try opening the file.
  // Using a C file stream is required here since the UNIX file locking operation
  // fcntl() only works on file descriptors as returned by fileno().
  pFile = fopen( szFilePath_i, sMode );
  if( pFile == NULL )
  {
    // Set failbit as in std::ofstream::open().
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }

  // Get its file descriptor using the POSIX function fileno().
  int nFileDescr = fileno( pFile );

  // Use UNIX fcntl() to try to aquire an advisory write lock for the full file.
  // See e.g. https://linux.die.net/man/2/fcntl
  {
    // Prepare lock info structure.
    flock oLock;
    oLock.l_type = F_WRLCK;
    oLock.l_whence = SEEK_SET;
    oLock.l_start = 0;
    oLock.l_len = 0;

    // Set the command accordingly to the desired behavior if a lock cannot be
    // acquired: either return immediately or wait.
    int nCmd;
    switch( fAction_i )
    {
      case lock_action::WAIT:
        nCmd = F_SETLKW;
        break;

      case lock_action::RETURN:
        nCmd = F_SETLK;
        break;
    }

    // Try to acquire write lock.
    if( fcntl( nFileDescr, nCmd, &oLock ) == -1 )
    {
      // => The command failed. This could either be because a concurrent
      //    application own the lock (F_SETLK) or because the program has
      //    caught a signal (e.g. break-off signal 15) during waiting (F_SETLKW).

      // Close file, set failbit and return.
      fclose( pFile );
      this->setstate( this->rdstate() | std::ios_base::failbit );
      return;
    }
  }

  // Create a new filebuffer object and open the stream.
  // NOTE: Instead of using a file buffer of type std::filebuf, we need to use
  //       the libstdc++-dependent version __gnu_cxx::stdio_filebuf here since
  //       this is the only one that accepts a file descriptor.
  //       See https://gcc.gnu.org/onlinedocs/gcc-4.6.2/libstdc++/api/a00069.html
  pFileBuf = new __gnu_cxx::stdio_filebuf<char>( nFileDescr, fOpenMode_i );

  // Check if the file buffer is validly open.
  if( pFileBuf->is_open() )
  {
    // => Everything is fine.

    // Set the buffer as stream buffer to the istream.
    this->rdbuf( pFileBuf );
  }
  else
  {
    // => An error has occured during opening the file buffer.

    // Delete file buffer and reset its pointer's value to NULL.
    delete pFileBuf;
    pFileBuf = NULL;

    // Close file, set failbit and return.
    fclose( pFile );
    this->setstate( this->rdstate() | std::ios_base::failbit );
    return;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Opens the file identified by the file name argument,
///             associating it with the stream object, so that input/output
///             operations are performed on its content.

/// \details    Variant of
///             \ref oflstream::open( const char*, lock_action, std::ios_base::openmode )
///             that accepts the file name as an std::string object.
// *****************************************************************************

inline void filesys::oflstream::open( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  this->open( sFilePath_i.c_str(), fAction_i, fOpenMode_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Closes the file currently associated with the object,
///             disassociating it from the stream.
// *****************************************************************************

inline void filesys::oflstream::close()
{
  // Unset the stream buffer of the input stream.
  this->rdbuf( NULL );

  // Delete the std::filebuf object.
  if( pFileBuf != NULL )
  {
    // Use UNIX fcntl() to unlock the full file.
    // See e.g. https://linux.die.net/man/2/fcntl
    {
      // Prepare lock info structure.
      flock oLock;
      oLock.l_type = F_UNLCK;
      oLock.l_whence = SEEK_SET;
      oLock.l_start = 0;
      oLock.l_len = 0;

      // Unlock the file.
      fcntl( fileno( pFileBuf->file() ), F_SETLK, &oLock );
    }

    // Delete the file buffer. Set its pointer back to NULL.
    // Note that deleting the file buffer SHOULD implicitly close the file
    // (like a call to fclose()) but it was found that it does not.
    delete pFileBuf;
    pFileBuf = NULL;

    // Close file explicitly.
    fclose( pFile );
  }
}

// -----------------------------------------------------------------------------

// UNIX: FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Evaluates a canonical path (that may include wildcards) into a
///             list of accessible files or directories that exist on the drive.
///
/// \details    The function checks whether files or directories that match the
///             given (absolute or relative) path exist on the file system and
///             whether they are accessible.
///
/// \remark     The function is mainly used for expanding paths that include
///             wildcards. Since it cycles through each directories entries, it
///             is rather slow. For checking if a file exists and is accessible
///             whose path contains no wildcards, it is recommended to use e.g.
///             std::filebuf.open() and std::filebuf.is_open() or fopen().
///
/// \param[in]  szCanonPath_i
///             An absolute or relative canonical path as it can be retrieved
///             from \ref GetCanonicalPath(). The path may contain the
///             following wildcards:
///             - ? : zero or one unknown character except dot(.)
///             - * : Match any number of unknown characters (regardless of
///                   the position where it appears, including at the start
///                   and/or multiple times)
///             Note that the string is changed by the function.
/// \param[in]  fCaseType_i
///             Flag what letter case convention is used for matching the
///             file names. Possible values are:
///             - \ref case_type::OS_CONVENTION : the convention of the respective
///               operating system is used
///             - \ref case_type::CASE_SENSITIVE : the comparison is performed
///               in a case sensitive form
///             - \ref case_type::CASE_INSENSITIVE : the comparison is performed
///               in a not case sensitive form
/// \param[out] oaFileInfo_o
///             Returns a list of information about the files or directories
///             matching the canonical path name. If one element was found to
///             match the canonical path exactly (amongst other that only match
///             with respect to case insensitivity), it is placed at position 0.
///
/// \return     'true' if any file or directory element was found, 'false'
///             if no such element exists.
// *****************************************************************************

inline bool filesys::GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o )
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
      oaFileInfo_o[0].sPath.assign( "./" );
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
        if( ( pDir = opendir( oaFileInfo_o[s].sPath.c_str() ) ) )
        {
          // Loop over all files within the path section.
          // Use multithreading safe reenrent version of POSIX "readdir()".
          while( readdir_r( pDir, &oEntry, &pResult ) == 0 )
          {
            // Break off if all items of the directory were read.
            if( pResult == NULL )
              break;

            // Skip items "." and ".." that may occur from wildcard evaluation.
            if( fWildcard &&
                ( strcmp( oEntry.d_name, "." ) == 0 ||
                  strcmp( oEntry.d_name, ".." ) == 0 ) )
              continue;

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
                    if( ( pCheckDir = opendir( sCheckPath.c_str() ) ) )
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

  // Clear result list of failed elements and, if an exactly matching
  // element exists (with respect to case sensitvity), put it to position 0.
  {
    size_t nExactPos = 0;
    size_t nPos = 0;
    for( size_t s = 0; s < oaFileInfo_o.size(); s++ )
      if( oaFileInfo_o[s].fMatch )
      {
        oaFileInfo_o[nPos++] = oaFileInfo_o[s];
        if( oaFileInfo_o[s].fMatchType == match_type::EXACT )
          nExactPos = nPos - 1;
      }
    oaFileInfo_o.resize( nPos );

    if( nExactPos != 0 )
    {
      SFileInfo oBuf = oaFileInfo_o[0];
      oaFileInfo_o[0] = oaFileInfo_o[nExactPos];
      oaFileInfo_o[nExactPos] = oBuf;
    }
  }

  // Return whether any element was found.
  return !oaFileInfo_o.empty();
}

#endif

// -----------------------------------------------------------------------------

// *****************************************************************************
// *                                                                           *
// *                          WINDOWS IMPLEMENTATION                           *
// *                                                                           *
// *****************************************************************************

#ifdef FILESYS_WINDOWS

// -----------------------------------------------------------------------------

// WINDOWS: INCLUSIONS

// -----------------------------------------------------------------------------

// Flag to tell <windows.h> not to overwrite "min()" and "max()" with a macro
#define NOMINMAX

#include <windows.h>
#include <string.h>
#include <fstream>     // std::ifstream, std::ofstream

// -----------------------------------------------------------------------------

// WINDOWS: GLOBAL MACROS AND CONSTANTS

// -----------------------------------------------------------------------------

namespace filesys
{
  static const char cPathSep = '\\';          ///< Path separator character.
  static const char szDevNull[] = "NUL";      ///< Null device string.
}

// -----------------------------------------------------------------------------

// WINDOWS: CLASS iflstream

// -----------------------------------------------------------------------------

class filesys::iflstream : public std::ifstream
{
  public:

    // Constructors, destructor:
    explicit iflstream( const char* szFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    explicit iflstream( const std::string& sFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    ~iflstream();

    // Locked file opening and closing.
    void open( const char* szFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    void open( const std::string& sFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::in );
    void close();
};

// .............................................................................

inline filesys::iflstream::iflstream( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  this->open( szFilePath_i, fAction_i, fOpenMode_i );
}

// .............................................................................

inline filesys::iflstream::iflstream( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  iflstream( sFilePath_i.c_str(), fAction_i, fOpenMode_i )
{}

// .............................................................................

inline filesys::iflstream::~iflstream()
{
  this->close();
}

// .............................................................................

inline void filesys::iflstream::open( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  // TODO: apply shared file lock (LockFile function, see
  //       https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-lockfile)

  this->std::ifstream::open( szFilePath_i, fOpenMode_i );
}

// .............................................................................

inline void filesys::iflstream::open( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  this->open( sFilePath_i.c_str(), fAction_i, fOpenMode_i );
}

// .............................................................................

inline void filesys::iflstream::close()
{
  this->std::ifstream::close();

  // TODO: remove file lock (LockFile function, see
  //       https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-unlockfile)
}

// -----------------------------------------------------------------------------

// WINDOWS: CLASS oflstream

// -----------------------------------------------------------------------------

class filesys::oflstream : public std::ofstream
{
  public:

    // Constructors, destructor:
    explicit oflstream( const char* szFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    explicit oflstream( const std::string& sFilePath_i,
                        lock_action fAction_i = lock_action::WAIT,
                        std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    ~oflstream();

    // Locked file opening and closing.
    void open( const char* szFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    void open( const std::string& sFilePath_i,
              lock_action fAction_i = lock_action::WAIT,
              std::ios_base::openmode fOpenMode_i = std::ios_base::out );
    void close();
};

// .............................................................................

inline filesys::oflstream::oflstream( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  // Call open():
  this->open( szFilePath_i, fAction_i, fOpenMode_i );
}

// .............................................................................

inline filesys::oflstream::oflstream( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i ) :
  oflstream( sFilePath_i.c_str(), fAction_i, fOpenMode_i )
{}

// .............................................................................

inline filesys::oflstream::~oflstream()
{
  this->close();
}

// .............................................................................

inline void filesys::oflstream::open( const char* szFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  // TODO: Lock the file for exclusive access, (see LockFile function,
  //       https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-lockfile)

  this->std::ofstream::open( szFilePath_i, fOpenMode_i );
}

// .............................................................................

inline void filesys::oflstream::open( const std::string& sFilePath_i,
                                      lock_action fAction_i,
                                      std::ios_base::openmode fOpenMode_i )
{
  this->open( sFilePath_i.c_str(), fAction_i, fOpenMode_i );
}

// .............................................................................

inline void filesys::oflstream::close()
{
  this->std::ofstream::close();

  // TODO: Unlock the file, (see UnlockFile function,
  //       https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-unlockfile)
}

// -----------------------------------------------------------------------------

// WINDOWS: FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Evaluates a canonical path (that may include wildcards) into a
///             list of accessible files or directories that exist on the drive.
///
/// \details    The function checks whether files or directories that match the
///             given (absolute or relative) path exist on the file system and
///             whether they are accessible.
///
/// \remark     The function is mainly used for expanding paths that include
///             wildcards. Since it cycles through each directories entries, it
///             is rather slow. For checking if a file exists and is accessible
///             whose path contains no wildcards, it is recommended to use e.g.
///             std::filebuf.open() and std::filebuf.is_open() or fopen().
///
/// \param[in]  szCanonPath_i
///             An absolute or relative canonical path as it can be retrieved
///             from \ref GetCanonicalPath(). The path may contain the
///             following wildcards:
///             - ? : zero or one unknown character except dot(.)
///             - * : Match any number of unknown characters (regardless of
///                   the position where it appears, including at the start
///                   and/or multiple times)
///             Note that the string is changed by the function.
/// \param[in]  fCaseType_i
///             Flag what letter case convention is used for matching the
///             file names. Possible values are:
///             - \ref case_type::OS_CONVENTION : the convention of the respective
///               operating system is used
///             - \ref case_type::CASE_SENSITIVE : the comparison is performed
///               in a case sensitive form
///             - \ref case_type::CASE_INSENSITIVE : the comparison is performed
///               in a not case sensitive form
/// \param[out] oaFileInfo_o
///             Returns a list of information about the files or directories
///             matching the canonical path name. If one element was found to
///             match the canonical path exactly (amongst other that only match
///             with respect to case insensitivity), it is placed at position 0.
///
/// \return     'true' if any file or directory element was found, 'false'
///             if no such element exists.
// *****************************************************************************

inline bool filesys::GetFilesInfo( char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o )
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
            // Skip items "." and ".." that may occur from wildcard evaluation.
            if( fWildcard &&
                ( strcmp( oEntry.cFileName, "." ) == 0 ||
                  strcmp( oEntry.cFileName, ".." ) == 0 ) )
              continue;

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

  // Clear result list of failed elements and, if an exactly matching
  // element exists (with respect to case sensitvity), put it to position 0.
  {
    size_t nExactPos = 0;
    size_t nPos = 0;
    for( size_t s = 0; s < oaFileInfo_o.size(); s++ )
      if( oaFileInfo_o[s].fMatch )
      {
        oaFileInfo_o[nPos++] = oaFileInfo_o[s];
        if( oaFileInfo_o[s].fMatchType == match_type::EXACT )
          nExactPos = nPos - 1;
      }
    oaFileInfo_o.resize( nPos );

    if( nExactPos != 0 )
    {
      SFileInfo oBuf = oaFileInfo_o[0];
      oaFileInfo_o[0] = oaFileInfo_o[nExactPos];
      oaFileInfo_o[nExactPos] = oBuf;
    }
  }

  // Return whether any element was found.
  return !oaFileInfo_o.empty();
}

#endif

// -----------------------------------------------------------------------------

// *****************************************************************************
// *                                                                           *
// *                          MAC-OS-X-IMPLEMENTATION                          *
// *                                                                           *
// *****************************************************************************

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
/// \brief      Like \ref GetFilesInfo( char*, case_type,
///             std::vector<SFileInfo>& ) but accepts a constant string as input
///             path that is not modified.
///
/// \details    Slightly worse performance than \ref GetFilesInfo( char*,
///             case_type, std::vector<SFileInfo>& ) since it requires an
///             additional memory allocation and value duplication.
// *****************************************************************************

inline bool filesys::GetFilesInfo( const char* szCanonPath_i, case_type fCaseType_i, std::vector<SFileInfo>& oaFileInfo_o )
{
  // Create a input string duplicate buffer string.
  const size_t nStrLen = strlen( szCanonPath_i );
  char* pBuf = (char*) malloc( sizeof(char) * ( nStrLen + 1 ) );
  if( pBuf == NULL )
    throw std::bad_alloc();
  pBuf[nStrLen] = 0;
  memcpy( pBuf, szCanonPath_i, nStrLen );

  // Call non-const argument version of GetFilesInfo().
  bool fReturn = GetFilesInfo( pBuf, fCaseType_i, oaFileInfo_o );

  // Free buffer memory and return.
  free( pBuf );
  return fReturn;
}

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

inline int filesys::strtouppercmp( const char* szString1_i, const char* szString2_i )
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

inline int filesys::strglobcmp( const char* szString1_i, const char* szString2_i )
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

inline int filesys::strtoupperglobcmp( const char* szString1_i, const char* szString2_i )
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

// *****************************************************************************
/// \brief      Converts an std::fstream flag into its equivalent fopen() mode
///             string.
///
/// \details    Implements the following translation table (from
///             https://gcc.gnu.org/ml/libstdc++/2007-06/msg00012.html ):
/// \verbatim
///  +---------------------------------------------------------+
///  | ios_base Flag combination            stdio equivalent   |
///  |binary  in  out  trunc  app                              |
///  +---------------------------------------------------------+
///  |             +                        "w"                |
///  |             +           +            "a"                |
///  |         +   +           +            "a+"               |
///  |             +     +                  "w"                |
///  |         +                            "r"                |
///  |         +   +                        "r+"               |
///  |         +   +     +                  "w+"               |
///  +---------------------------------------------------------+
///  |   +         +                        "wb"               |
///  |   +         +           +            "ab"               |
///  |   +     +   +           +            "a+b"              |
///  |   +         +     +                  "wb"               |
///  |   +     +                            "rb"               |
///  |   +     +   +                        "r+b"              |
///  |   +     +   +     +                  "w+b"              |
///  +---------------------------------------------------------+
/// \endverbatim
///
/// \param[in]  fOpenMode_i
///             Open mode flag, see e.g.
///             http://www.cplusplus.com/reference/fstream/ofstream/ofstream/
/// \param[out] szMode_o
///             C-string that should be at least four characters long. This
///             returns the mode string that can be used in a call to
///             std::fopen(). See
///             http://www.cplusplus.com/reference/cstdio/fopen/?kw=fopen
// *****************************************************************************

inline void filesys::OpenModeConvert( std::ios_base::openmode fOpenMode_i, char* szMode_o )
{
  switch( fOpenMode_i )
  {
    case std::ios_base::out :
    case std::ios_base::out | std::ios_base::trunc :
      strcpy( szMode_o, "w" );
      break;

    case std::ios_base::out | std::ios_base::app :
      strcpy( szMode_o, "a" );
      break;

    case std::ios_base::out | std::ios_base::in | std::ios_base::app :
      strcpy( szMode_o, "a+" );
      break;

    case std::ios_base::in :
      strcpy( szMode_o, "r" );
      break;

    case std::ios_base::out | std::ios_base::in :
      strcpy( szMode_o, "r+" );
      break;

    case std::ios_base::out | std::ios_base::in | std::ios_base::trunc :
      strcpy( szMode_o, "w+" );
      break;

    case std::ios_base::out | std::ios_base::binary :
    case std::ios_base::out | std::ios_base::trunc | std::ios_base::binary :
      strcpy( szMode_o, "wb" );
      break;

    case std::ios_base::out | std::ios_base::app | std::ios_base::binary :
      strcpy( szMode_o, "ab" );
      break;

    case std::ios_base::out | std::ios_base::in | std::ios_base::app | std::ios_base::binary :
      strcpy( szMode_o, "a+b" );
      break;

    case std::ios_base::in | std::ios_base::binary :
      strcpy( szMode_o, "rb" );
      break;

    case std::ios_base::out | std::ios_base::in | std::ios_base::binary :
      strcpy( szMode_o, "r+b" );
      break;

    case std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary :
      strcpy( szMode_o, "w+b" );
      break;
  }
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

#endif // FILESYS_READ_ONCE
