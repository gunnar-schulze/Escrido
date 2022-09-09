// -----------------------------------------------------------------------------
/// \file       content-unit.cpp
///
/// \brief      Implementation file for
///             documentation <em>content units</em>, i.e. general data blocks
///             of the Escrido documentation.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2016 trinckle 3D GmbH
// -----------------------------------------------------------------------------

#include "content-unit.h"

#include <list>             // std::list
#include <string.h>         // strlen()
#include <iostream>         // cin, cout, cerr, endl
#include <fstream>          // std::ofstream

// -----------------------------------------------------------------------------

// STRUCT SWriteInfo

// -----------------------------------------------------------------------------

escrido::SWriteInfo::SWriteInfo( const std::vector <std::pair<std::string, std::string>>& oRelabelList_i ):
  oRelabelList ( oRelabelList_i )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the label text or a registered replacement, if this
///             exists.
// *****************************************************************************

const char* escrido::SWriteInfo::Label( const char* szLabel_i ) const
{
  for( size_t l = 0; l < oRelabelList.size(); ++l )
    if( oRelabelList[l].first == szLabel_i )
      return oRelabelList[l].second.c_str();

  return szLabel_i;
}

// .............................................................................


const escrido::SWriteInfo& escrido::SWriteInfo::operator++() const
{
  nIndent += 2;
  return *this;
}

// .............................................................................

const escrido::SWriteInfo escrido::SWriteInfo::operator++( int ) const
{
  SWriteInfo oCpy = *this;
  nIndent += 2;
  return oCpy;
}

// .............................................................................

const escrido::SWriteInfo& escrido::SWriteInfo::operator--() const
{
  nIndent -= 2;
  return *this;
}

// .............................................................................

const escrido::SWriteInfo escrido::SWriteInfo::operator--( int ) const
{
  SWriteInfo oCpy = *this;
  nIndent -= 2;
  return oCpy;
}

// -----------------------------------------------------------------------------

// CLASS CContentChunk

// -----------------------------------------------------------------------------

escrido::CContentChunk::CContentChunk():
  fType           ( cont_chunk_type::UNDEFINED ),
  fSkipFirstWhite ( skip_first_white::OFF )
{}

// .............................................................................

escrido::CContentChunk::CContentChunk( const cont_chunk_type fType_i ):
  fType           ( fType_i ),
  fSkipFirstWhite ( skip_first_white::OFF )
{}

// .............................................................................

cont_chunk_type escrido::CContentChunk::GetType() const
{
  return fType;
}

// .............................................................................

std::string& escrido::CContentChunk::GetContent()
{
  return sContent;
}

// .............................................................................

void escrido::CContentChunk::SetSkipFirstWhiteMode( skip_first_white fSkipFirstWhite_i )
{
  fSkipFirstWhite = fSkipFirstWhite_i;
}

// .............................................................................

std::string escrido::CContentChunk::GetPlainText() const
{
  if( fType == cont_chunk_type::NEW_LINE )
    return "\n";

  std::string sReturn;
  All( sContent, sReturn );
  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first word (i.e. the first group of non-blank space
///             characters) of the chunk.
///
/// \see        escrido::FirstWord()
/// \see        escrido::MakeIdentifier()
///
/// \return     The first word (w/o any blank space) or an empty string, if no
///             first word exists.
// *****************************************************************************

std::string escrido::CContentChunk::GetPlainFirstWord() const
{
  std::string sReturn;
  FirstWord( sContent, sReturn );
  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first word (i.e. the first group of non-blank space
///             characters) or the first quote (i.e. all characters within
///             double quotation marks) of the chunk.
///
/// \see        escrido::FirstWord()
/// \see        escrido::MakeIdentifier()
///
/// \return     The first word (w/o any blank space) or quote (w/o start and end
///             quotation marks) or an empty string, if no first word or quote
///             exists.
// *****************************************************************************

std::string escrido::CContentChunk::GetPlainFirstWordOrQuote() const
{
  std::string sReturn;
  if( !FirstQuote( sContent, sReturn ) )
    FirstWord( sContent, sReturn );
  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the group of characters that form the text after the
///             first word of the chunk.
///
/// \see        escrido::AllButFirstWord()
/// \see        escrido::MakeIdentifier()
///
/// \return     The string after the first word and proceeding blank spaces or
///             an empty string if no such string exists.
// *****************************************************************************

std::string escrido::CContentChunk::GetPlainAllButFirstWord() const
{
  std::string sReturn;
  AllButFirstWord( sContent, sReturn );
  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first line (i.e. the first group of characters
///             before the first line break '\r', '\n' or '\r\n' ) of the chunk.
///
/// \see        escrido::FirstLine()
/// \see        escrido::MakeIdentifier()
///
/// \return     The first line or an empty string, if no
///             first word exists.
// *****************************************************************************

std::string escrido::CContentChunk::GetPlainFirstLine() const
{
  std::string sReturn;
  FirstLine( sContent, sReturn );
  return sReturn;
}

// .............................................................................

void escrido::CContentChunk::AppendChar( const char cChar_i )
{
  // Check "skip first whitespace" mode.
  if( fSkipFirstWhite == skip_first_white::INIT )
  {
    if( cChar_i == ' ' || cChar_i == '\t' )
      return;
    fSkipFirstWhite = skip_first_white::OFF;
  }

  // In HTML mode: skip multiple blank spaces.
  if( fType == cont_chunk_type::HTML_TEXT )
    if( cChar_i == ' ' )
      if( !sContent.empty() )
        if( sContent.back() == ' ' )
          return;

  sContent.push_back( cChar_i );
}

// .............................................................................

void escrido::CContentChunk::WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << sContent;
      break;

    case cont_chunk_type::PLAIN_TEXT:
    {
      // Do HTML escaping of plain text.
      oOutStrm_i << HTMLEscape( sContent );
      break;
    }

    case cont_chunk_type::NEW_LINE:
      oOutStrm_i << "<br>";
      break;

    case cont_chunk_type::START_PARAGRAPH:
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<p>";
      break;

    case cont_chunk_type::END_PARAGRAPH:
      oOutStrm_i << "</p>" << std::endl;
      --oWriteInfo_i;
      break;

    case cont_chunk_type::START_TABLE:
      WriteHTMLTagLine( "<table>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLTagLine( "<tr>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<td>";
      break;

    case cont_chunk_type::END_TABLE:
      oOutStrm_i << "</td>" << std::endl;
      --oWriteInfo_i;
      WriteHTMLTagLine( "</tr>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTagLine( "</table>", oOutStrm_i, --oWriteInfo_i );
      break;

    case cont_chunk_type::NEW_TABLE_CELL:
      oOutStrm_i << "</td>" << std::endl;
      --oWriteInfo_i;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<td>";
      break;

    case cont_chunk_type::NEW_TABLE_ROW:
      oOutStrm_i << "</td>" << std::endl;
      --oWriteInfo_i;
      WriteHTMLTagLine( "</tr>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTagLine( "<tr>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<td>";
      break;

    case cont_chunk_type::START_UL:
      WriteHTMLTagLine( "<ul>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<li>";
      break;

    case cont_chunk_type::END_UL:
      oOutStrm_i << "</li>" << std::endl;
      --oWriteInfo_i;
      WriteHTMLTagLine( "</ul>", oOutStrm_i, --oWriteInfo_i );
      break;

    case cont_chunk_type::UL_ITEM:
      oOutStrm_i << "</li>" << std::endl;
      WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "<li>";
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::REF:
    {
      size_t nRefIdx;
      bool fHasRef = oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->GetPlainFirstWord() ), nRefIdx );
      if( fHasRef )
        oOutStrm_i << "<a href=\"" + oWriteInfo_i.oRefTable.GetLink( nRefIdx ) + "\">";

      std::string sText = this->GetPlainAllButFirstWord();
      if( !sText.empty() )
        oOutStrm_i << sText;
      else
      {
        if( fHasRef )
          oOutStrm_i << oWriteInfo_i.oRefTable.GetText( nRefIdx );
        else
          oOutStrm_i << sContent;
      }

      if( fHasRef )
        oOutStrm_i << "</a>";
      break;
    }

    case cont_chunk_type::START_CODE:
      oOutStrm_i << "<span class=\"code\">";
      break;

    case cont_chunk_type::END_CODE:
      oOutStrm_i << "</span>";
      break;

    case cont_chunk_type::LINK:
    {
      std::string sHREF = this->GetPlainFirstWord();
      if( !sHREF.empty() )
      {
        oOutStrm_i << "<a href=\"" << sHREF << "\" target=\"_blank\">";

        std::string sText = this->GetPlainAllButFirstWord();
        if( sText.empty() )
          oOutStrm_i << sHREF;
        else
          oOutStrm_i << sText;

        oOutStrm_i << "</a>";
      }
      break;
    }

    case cont_chunk_type::START_VERBATIM:
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<pre>";
      break;

    case cont_chunk_type::END_VERBATIM:
      oOutStrm_i << "</pre>" << std::endl;
      --oWriteInfo_i;
      break;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the first word (i.e. the first group of non-whitespace
///             characters) of the chunk as HTML into the stream.
///
/// \see        escrido::FirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get first word.
  std::string sFirstWord;
  if( !FirstWord( sContent, sFirstWord ) )
    return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << sFirstWord;
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do HTML escaping.
      oOutStrm_i << HTMLEscape( sFirstWord );
      return true;

    default:
      return false;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the group of characters that form the text after the
///             first word of the chunk as HTML into the stream.
///
/// \see        escrido::AllButFirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get all-but-first-word.
  std::string sAllButFirstWord;
  if( !AllButFirstWord( sContent, sAllButFirstWord ) )
    return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << sAllButFirstWord;
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do HTML escaping.
      oOutStrm_i << HTMLEscape( sAllButFirstWord );
      return true;

    default:
      return false;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the group of characters that form the text after the
///             first word or quote of the chunk as HTML into the stream.
///
/// \see        escrido::AllButFirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteHTMLAllButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get all-but-first-word-or-quote.
  std::string sAllButFirstWordOrQuote;
  if( !AllButFirstQuote( sContent, sAllButFirstWordOrQuote ) )
    if( !AllButFirstWord( sContent, sAllButFirstWordOrQuote ) )
      return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << sAllButFirstWordOrQuote;
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do HTML escaping.
      oOutStrm_i << HTMLEscape( sAllButFirstWordOrQuote );
      return true;

    default:
      return false;
  }
}

// .............................................................................

void escrido::CContentChunk::WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << ConvertHTML2LaTeX( sContent );
      break;

    case cont_chunk_type::PLAIN_TEXT:
    {
      // Do LaTeX escaping of plain text.
      oOutStrm_i << LaTeXEscape( sContent );
      break;
    }

    case cont_chunk_type::NEW_LINE:
      oOutStrm_i << "\n";
      break;

    case cont_chunk_type::START_PARAGRAPH:
      break;

    case cont_chunk_type::END_PARAGRAPH:
      oOutStrm_i << std::endl << std::endl;
      break;

    case cont_chunk_type::START_TABLE:
    {
      // Find out maximum column number of the table.
      int nMaxColN = 0;
      {
        int nColN = 1;
        const CContentChunk* pNextContentChunk = this;
        while( true )
        {
          if( pNextContentChunk->fType == cont_chunk_type::END_TABLE ||
              pNextContentChunk->fType == cont_chunk_type::NEW_TABLE_ROW )
          {
            if( nMaxColN < nColN )
              nMaxColN = nColN;
            nColN = 1;
            break;
          }

          if( pNextContentChunk->fType == cont_chunk_type::NEW_TABLE_CELL )
            nColN++;

          pNextContentChunk = oWriteInfo_i.pTagBlock->GetNextContentChunk( pNextContentChunk );
        }
      }

      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "\\noindent\\parbox{\\textwidth}{%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\tymin=" << 1.0 / ( nMaxColN + 1 ) << "\\textwidth%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\centering%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\begin{tabulary}{\\textwidth}{";
      for( int i = 0; i < nMaxColN; i++ )
        oOutStrm_i << "L";
      oOutStrm_i << "}" << std::endl;
      ++oWriteInfo_i;
      break;
    }

    case cont_chunk_type::END_TABLE:
      oOutStrm_i << std::endl;
      WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "\\end{tabulary}" << std::endl;
      WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "}" << std::endl;
      break;

    case cont_chunk_type::NEW_TABLE_CELL:
      oOutStrm_i << " & ";
      break;

    case cont_chunk_type::NEW_TABLE_ROW:
      oOutStrm_i << " \\\\" << std::endl;
      break;

    case cont_chunk_type::START_UL:
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "\\noindent\\parbox{\\textwidth}{%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "\\begin{itemize}" << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\item";
      break;

    case cont_chunk_type::END_UL:
      oOutStrm_i << std::endl;
      WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "\\end{itemize}" << std::endl;
      WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "}" << std::endl;
      break;

    case cont_chunk_type::UL_ITEM:
      oOutStrm_i << std::endl;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\item";
      break;

    case cont_chunk_type::REF:
    {
      size_t nRefIdx;
      bool fHasRef = oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->GetPlainFirstWord() ), nRefIdx );
      if( fHasRef )
        oOutStrm_i << "\\hyperref[" << MakeIdentifier( this->GetPlainFirstWord() ) << "]{";

      std::string sText = this->GetPlainAllButFirstWord();
      if( !sText.empty() )
        oOutStrm_i << ConvertHTML2LaTeX( sText );
      else
        if( fHasRef )
          oOutStrm_i << ConvertHTML2LaTeX( oWriteInfo_i.oRefTable.GetText( nRefIdx ) );
        else
          oOutStrm_i << ConvertHTML2LaTeX( sContent );

      if( fHasRef )
        oOutStrm_i << "}";
      break;
    }

    case cont_chunk_type::START_CODE:
      oOutStrm_i << "\\code{";
      break;

    case cont_chunk_type::END_CODE:
      oOutStrm_i << "}";
      break;

    case cont_chunk_type::LINK:
    {
      std::string sHREF = this->GetPlainFirstWord();
      if( !sHREF.empty() )
      {
        oOutStrm_i << "\\url{" << sHREF << "}";

        std::string sText = this->GetPlainAllButFirstWord();
        if( !sText.empty() )
          oOutStrm_i << "{"<< sText << "}";
      }
      break;
    }

    case cont_chunk_type::START_VERBATIM:
      oOutStrm_i << "\\begin{verbatim}" << std::endl;
      break;

    case cont_chunk_type::END_VERBATIM:
      oOutStrm_i << std::endl;
      oOutStrm_i << "\\end{verbatim}" << std::endl;
      break;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the first word (i.e. the first group of non-whitespace
///             characters) of the chunk as LaTeX into the stream.
///
/// \see        escrido::FirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteLaTeXFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get first word.
  std::string sFirstWord;
  if( !FirstWord( sContent, sFirstWord ) )
    return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << ConvertHTML2LaTeX( sFirstWord );
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do LaTeX escaping.
      oOutStrm_i << LaTeXEscape( sFirstWord );
      return true;

    default:
      return false;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the group of characters that form the text after the
///             first word of the chunk as LaTeX into the stream.
///
/// \see        escrido::AllButFirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteLaTeXAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get all-but-first-word.
  std::string sAllButFirstWord ;
  if( !AllButFirstWord( sContent, sAllButFirstWord ) )
    return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << ConvertHTML2LaTeX( sAllButFirstWord );
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do LaTeX escaping.
      oOutStrm_i << LaTeXEscape( sAllButFirstWord );
      return true;

    default:
      return false;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the group of characters that form the text after the
///             first word or quote of the chunk as LaTeX into the stream.
///
/// \see        escrido::AllButFirstWord()
///
/// \return     True if one or more characters were written, false otherwise.
// *****************************************************************************

bool escrido::CContentChunk::WriteLaTeXAllButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Get all-but-first-word-or-quote.
  std::string sAllButFirstWordOrQuote;
  if( !AllButFirstQuote( sContent, sAllButFirstWordOrQuote ) )
    if( !AllButFirstWord( sContent, sAllButFirstWordOrQuote ) )
      return false;

  // Chunk dependend writing of the word.
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << ConvertHTML2LaTeX( sAllButFirstWordOrQuote );
      return true;

    case cont_chunk_type::PLAIN_TEXT:
      // Do LaTeX escaping.
      oOutStrm_i << LaTeXEscape( sAllButFirstWordOrQuote );
      return true;

    default:
      return false;
  }
}

// .............................................................................

void escrido::CContentChunk::DebugOutput() const
{
  std::cout << "chunk type: " << (int) fType << ", content: '" << sContent << "'";
}

// -----------------------------------------------------------------------------

// CLASS CTagBlock

// -----------------------------------------------------------------------------

escrido::CTagBlock::CTagBlock():
  fType              ( tag_type::PARAGRAPH ),
  fAppIdentTextMode  ( append_ident_text_mode::OFF ),
  fVerbatimStartMode ( verbatim_start_mode::OFF ),
  fNewLine           ( true )
{}

// .............................................................................

escrido::CTagBlock::CTagBlock( tag_type fType_i ):
  fType              ( fType_i ),
  fAppIdentTextMode  ( append_ident_text_mode::OFF ),
  fVerbatimStartMode ( verbatim_start_mode::OFF ),
  fNewLine           ( true )
{
  // Switch on "delimitate title line" mode for specific tag block types:
  if( fType == tag_type::SECTION ||
      fType == tag_type::SUBSECTION ||
      fType == tag_type::SUBSUBSECTION ||
      fType == tag_type::FEATURE )
    faWriteMode.emplace_back( tag_block_write_mode::TITLE_LINE );

  // Switch on "verbatim start" mode for specific tag block types:
  if( fType == tag_type::EXAMPLE ||
      fType == tag_type::OUTPUT )
    fVerbatimStartMode = verbatim_start_mode::INIT;
}

// .............................................................................

bool escrido::CTagBlock::Empty() const
{
  if( oaChunkList.size() == 0 )
    return true;
  else
    if( oaChunkList.size() == 1 )
      if( oaChunkList[0].GetPlainText().empty() )
        return true;

  return false;
}

// .............................................................................

void escrido::CTagBlock::SetTagType( tag_type fType_i )
{
  this->fType = fType_i;

  // Switch on "delimitate title line" mode for specific tag block types:
  if( fType == tag_type::SECTION ||
      fType == tag_type::SUBSECTION ||
      fType == tag_type::SUBSUBSECTION )
    faWriteMode.emplace_back( tag_block_write_mode::TITLE_LINE );
}

// .............................................................................

tag_type escrido::CTagBlock::GetTagType() const
{
  return this->fType;
}

// .............................................................................

tag_block_write_mode escrido::CTagBlock::GetWriteMode() const
{
  if( faWriteMode.empty() )
    return tag_block_write_mode::PLAIN_TEXT;
  else
    return faWriteMode.back();
}

// .............................................................................

// *****************************************************************************
/// \brief      Closes all write modes by adding the respective chunks.
// *****************************************************************************

void escrido::CTagBlock::CloseWrite()
{
  while( faWriteMode.size() > 0 )
  {
    switch( faWriteMode.back() )
    {
      case tag_block_write_mode::PARAGRAPH:
        this->oaChunkList.emplace_back( cont_chunk_type::END_PARAGRAPH );
        break;

      case tag_block_write_mode::TABLE:
        this->oaChunkList.emplace_back( cont_chunk_type::END_TABLE );
        break;

      case tag_block_write_mode::UL:
        this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
        break;

      case tag_block_write_mode::VERBATIM:
        this->oaChunkList.emplace_back( cont_chunk_type::END_VERBATIM );
        break;
    }
    faWriteMode.pop_back();
  }
}

// .............................................................................

std::string escrido::CTagBlock::GetPlainText() const
{
  std::string sReturn;
  for( size_t c = 0; c < oaChunkList.size(); c++ )
    sReturn += oaChunkList[c].GetPlainText();

  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first word (i.e. the first group of non-blank space
///             characters) of the tag block.
///
/// \return     The first word (w/o any blank space) or an empty string, if no
///             first word exists.
// *****************************************************************************

std::string escrido::CTagBlock::GetPlainFirstWord() const
{
  std::string sReturn;
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    sReturn = oaChunkList[c].GetPlainFirstWord();
    if( !sReturn.empty() )
      break;
  }

  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first word (i.e. the first group of non-blank space
///             characters) or the first quote (i.e. all characters within
///             double quotation marks) of the tag block.
///
/// \return     The first word (w/o any blank space) or quote (w/o start and end
///             quotation marks) or an empty string, if no first word or quote
///             exists.
// *****************************************************************************

std::string escrido::CTagBlock::GetPlainFirstWordOrQuote() const
{
  std::string sReturn;
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    sReturn = oaChunkList[c].GetPlainFirstWordOrQuote();
    if( !sReturn.empty() )
      break;
  }

  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the title line (i.e. the first line up to the title
///             delimitor chunk) of the tag block.
///
/// \return     The title line or an empty string, if no text in the first line
///             exists.
// *****************************************************************************

std::string escrido::CTagBlock::GetPlainTitleLine() const
{
  std::string sReturn;

  // Loop through all text chunks until the title line delimitor is reached.
  size_t c = 0;
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return sReturn;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    sReturn += oaChunkList[c].GetPlainText();
  }

  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the title line (i.e. the first line up to the title
///             delimitor chunk) without the first word of the tag block.
///
/// \return     The title line without the first word
///             or an empty string, if no first word or quote
///             exists.
// *****************************************************************************

std::string escrido::CTagBlock::GetPlainTitleLineButFirstWord() const
{
  std::string sReturn;

  // Loop through all text chunks until something after the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return sReturn;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    if( !oaChunkList[c].GetPlainFirstWord().empty() )
    {
      sReturn += oaChunkList[c].GetPlainAllButFirstWord();
      break;
    }
  }

  // Add remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return sReturn;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    sReturn += oaChunkList[c].GetPlainText();
  }

  return sReturn;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the next content chunk or NULL.
// *****************************************************************************

const escrido::CContentChunk* escrido::CTagBlock::GetNextContentChunk( const CContentChunk* pContentChunk ) const
{
  for( size_t c = 0; c < oaChunkList.size(); c++ )
    if( &oaChunkList[c] == pContentChunk )
      if( c < oaChunkList.size() )
        return &oaChunkList[c+1];

  return NULL;
}

// .............................................................................

void escrido::CTagBlock::AppendChar( const char cChar_i )
{
  // Save former parsing state.
  bool fFormerNewLine = fNewLine;

  // Setting newline parsing state to 'false' unless these are
  // the first whitespaces in a new line.
  fNewLine = fNewLine && ( cChar_i == ' ' );

  // Verbatim start mode:
  if( fVerbatimStartMode == verbatim_start_mode::INIT )
    fVerbatimStartMode = verbatim_start_mode::OFF;

  // Step 1: "append an identifier and a text" mode:
  switch( this->fAppIdentTextMode )
  {
    case append_ident_text_mode::INIT_IDENT:
    {
      // Skip white spaces, switch to appending state.
      if( cChar_i != ' ' )
      {
        // Switch "append an identifier and a text" mode to "IDENT".
        this->fAppIdentTextMode = append_ident_text_mode::IDENT;

        // Append the character to the latest chunk that accepts the identifier and/or text.
        // Note: the chunk list cannot be empty at that point since otherwise
        // the "append an identifier and a text" mode would be OFF.
        oaChunkList.back().AppendChar( cChar_i );
      }
      return;
    }

    case append_ident_text_mode::INIT_URI:
    {
      // Skip white spaces, switch to appending state.
      if( cChar_i != ' ' )
      {
        // Switch "append an identifier and a text" mode to "URI".
        this->fAppIdentTextMode = append_ident_text_mode::URI;

        // Append the character to the latest chunk that accepts the identifier and/or text.
        // Note: the chunk list cannot be empty at that point since otherwise
        // the "append an identifier and a text" mode would be OFF.
        oaChunkList.back().AppendChar( cChar_i );
      }
      return;
    }

    case append_ident_text_mode::IDENT:
    {
      // Check if character belongs to identifier [a-zA-Z]([a-zA-Z0-9_])+
      if( strchr( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", cChar_i ) != NULL )
      {
        // => Character belongs to identifier.

        // Append character to the latest chunk.
        oaChunkList.back().AppendChar( cChar_i );
      }
      else
      {
        // => Character does not belong to identifier. End of identifier reached.

        if( cChar_i == ' ' )
        {
          // => Blank space character.

          // Switch "append an identifier and a text" mode to "AFTER_IDENT_URI"
          this->fAppIdentTextMode = append_ident_text_mode::AFTER_IDENT_URI;
        }
        else
        {
          // => Non-blank character.
          //    Break off "append an identifier and a text" mode

          // Switch "append an identifier and a text" mode to "OFF"
          this->fAppIdentTextMode = append_ident_text_mode::OFF;

          // Add the character in the default way.
          AppendCharDefault( cChar_i );
        }
      }
      return;
    }

    case append_ident_text_mode::URI:
    {
      if( cChar_i != ' ' )
      {
        // => Character belongs to uri.

        // Append character to the latest chunk.
        oaChunkList.back().AppendChar( cChar_i );
      }
      else
      {
        // => Blank space character. End of uri reached.

        // Switch "append an identifier and a text" mode to "AFTER_IDENT_URI"
        this->fAppIdentTextMode = append_ident_text_mode::AFTER_IDENT_URI;
      }
      return;
    }

    case append_ident_text_mode::AFTER_IDENT_URI:
    {
      if( cChar_i != ' ' )
        if( cChar_i == '"' )
        {
          // => Alternative text follows.

          // Append one blank space to the latest chunk.
          oaChunkList.back().AppendChar( ' ' );

          // Switch "append an identifier and a text" mode to "TEXT".
          this->fAppIdentTextMode = append_ident_text_mode::TEXT;
        }
        else
        {
          // => No alternative text follows.
          //    Break off "append an identifier and a text" mode

          // Switch "append an identifier and a text" mode to "OFF"
          this->fAppIdentTextMode = append_ident_text_mode::OFF;

          // Add a blank space in the default way.
          AppendCharDefault( ' ' );

          // Add the character in the default way.
          AppendCharDefault( cChar_i );
        }
      return;
    }

    case append_ident_text_mode::TEXT:
    {
      if( cChar_i != '"' )
      {
        // => Character belongs to text.

        // Append character to the latest chunk.
        oaChunkList.back().AppendChar( cChar_i );
      }
      else
      {
        // => End of text.
        //    Break off "append an identifier and a text" mode

        // Switch "append an identifier and a text" mode to "OFF"
        this->fAppIdentTextMode = append_ident_text_mode::OFF;
      }
      return;
    }
  }

  // Step 2: special character specific behavior:
  switch( cChar_i )
  {
    // Character '-':
    // --------------
    case '-':
    {
      // Treat as normal character in verbatim tag block types:
      if( fType ==  tag_type::EXAMPLE ||
          fType ==  tag_type::OUTPUT ||
          ( !faWriteMode.empty() &&
            faWriteMode.back() == tag_block_write_mode::VERBATIM ) )
        break;

      // Special pattern: character '-' in a new line:
      // marks an unordered list item (bullet point).
      if( fFormerNewLine )
      {
        // Write mode dependend behavior:
        if( faWriteMode.empty() )
        {
          this->faWriteMode.emplace_back( tag_block_write_mode::UL );
          this->oaChunkList.emplace_back( cont_chunk_type::START_UL );
        }
        else
          switch( faWriteMode.back() )
          {
            case tag_block_write_mode::PLAIN_TEXT:
            {
              // Close text before adding unordered list:
              this->faWriteMode.pop_back();

              this->faWriteMode.emplace_back( tag_block_write_mode::UL );
              this->oaChunkList.emplace_back( cont_chunk_type::START_UL );
              break;
            }

            case tag_block_write_mode::PARAGRAPH:
            {
              // Close paragraph before adding unordered list:
              this->faWriteMode.pop_back();
              this->oaChunkList.emplace_back( cont_chunk_type::END_PARAGRAPH );

              this->faWriteMode.emplace_back( tag_block_write_mode::UL );
              this->oaChunkList.emplace_back( cont_chunk_type::START_UL );
              break;
            }

            case tag_block_write_mode::TABLE:
            {
              this->faWriteMode.emplace_back( tag_block_write_mode::UL );
              this->oaChunkList.emplace_back( cont_chunk_type::START_UL );
              break;
            }

            case tag_block_write_mode::UL:
            {
              this->oaChunkList.emplace_back( cont_chunk_type::UL_ITEM );
              break;
            }
          }

        // Skip further processing of this character.
        return;
      }

      break;
    }

    // Character '|':
    // --------------
    case '|':
    {
      // Special pattern: character '|' in a table or in
      // an unordered list inside a table:
      switch( this->faWriteMode.back() )
      {
        case tag_block_write_mode::TABLE:
        {
          // Close table cell.
          this->oaChunkList.emplace_back( cont_chunk_type::NEW_TABLE_CELL );

          // Skip further processing of this character.
          return;
        }

        case tag_block_write_mode::UL:
        {
          // Check if an unordered list is embedded into a table.
          signed int nBeforeLastWriteModeIdx = this->faWriteMode.size() - 2;
          if( nBeforeLastWriteModeIdx >= 0 )
            if( this->faWriteMode[nBeforeLastWriteModeIdx] == tag_block_write_mode::TABLE )
            {
              // => the unordered list is embedded inside a table.

              // Close unordered list and table cell.
              this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
              this->faWriteMode.pop_back();
              this->oaChunkList.emplace_back( cont_chunk_type::NEW_TABLE_CELL );

              // Skip further processing of this character.
              return;
            }
          break;
        }
      }

      break;
    }
  }

  // Default behavior: add character in the default way.
  AppendCharDefault( cChar_i );
}

// .............................................................................

void escrido::CTagBlock::AppendInlineTag( tag_type fTagType_i )
{
  // Verbatim start mode:
  if( fVerbatimStartMode == verbatim_start_mode::INIT )
    fVerbatimStartMode = verbatim_start_mode::OFF;

  // Tag specific behavior:
  switch( fTagType_i )
  {
    // Tag '@code':
    // -----------
    case tag_type::CODE:
    {
      // Write mode dependend behavior:
      if( faWriteMode.empty() )
      {
        faWriteMode.emplace_back( tag_block_write_mode::PARAGRAPH );
        oaChunkList.emplace_back( cont_chunk_type::START_PARAGRAPH );
      }

      // Add CODE chunk and PLAIN TEXT chunk.
      this->oaChunkList.emplace_back( cont_chunk_type::START_CODE );
      this->oaChunkList.emplace_back( cont_chunk_type::PLAIN_TEXT );
      this->oaChunkList.back().SetSkipFirstWhiteMode( skip_first_white::INIT );
      break;
    }

    // Tag '@endcode':
    // -----------
    case tag_type::END_CODE:
    {
      // Eventually delete one last whitespace before end code.
      if( this->oaChunkList.back().GetType() == cont_chunk_type::PLAIN_TEXT )
      {
        std::string& sCodeText = this->oaChunkList.back().GetContent();
        if( sCodeText.back() == ' ' || sCodeText.back() == '\t' )
          sCodeText.pop_back();
      }

      this->oaChunkList.emplace_back( cont_chunk_type::END_CODE );
      break;
    }

    // Tag '@link':
    // -----------
    case tag_type::LINK:
    {
      // Write mode dependend behavior:
      if( faWriteMode.empty() )
      {
        faWriteMode.emplace_back( tag_block_write_mode::PARAGRAPH );
        oaChunkList.emplace_back( cont_chunk_type::START_PARAGRAPH );
      }

      this->oaChunkList.emplace_back( cont_chunk_type::LINK );
      this->fAppIdentTextMode = append_ident_text_mode::INIT_URI;
      break;
    }

    // Tag '@lb':
    // ----------
    case tag_type::LINE_BREAK:
    {
      // Write mode dependend behavior:
      if( !faWriteMode.empty() )
        switch( this->faWriteMode.back() )
        {
          case tag_block_write_mode::PLAIN_TEXT:
          {
            this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
            break;
          }

          case tag_block_write_mode::PARAGRAPH:
          {
            this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
            break;
          }

          case tag_block_write_mode::TABLE:
          {
            this->oaChunkList.emplace_back( cont_chunk_type::NEW_TABLE_ROW );
            break;
          }

          case tag_block_write_mode::UL:
          {
            // Check if an unordered list is embedded into a table.
            signed int nBeforeLastWriteModeIdx = this->faWriteMode.size() - 2;
            if( nBeforeLastWriteModeIdx >= 0 )
              if( this->faWriteMode[nBeforeLastWriteModeIdx] == tag_block_write_mode::TABLE )
              {
                // => the unordered list is embedded inside a table.

                //  Close unordered list on '@lb' and go to a new table row.
                this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
                this->faWriteMode.pop_back();
                this->oaChunkList.emplace_back( cont_chunk_type::NEW_TABLE_ROW );
              }
            break;
          }
        }

      break;
    }

    // Tag '@ref':
    // -----------
    case tag_type::REF:
    {
      // Write mode dependend behavior:
      if( faWriteMode.empty() )
      {
        faWriteMode.emplace_back( tag_block_write_mode::PARAGRAPH );
        oaChunkList.emplace_back( cont_chunk_type::START_PARAGRAPH );
      }

      this->oaChunkList.emplace_back( cont_chunk_type::REF );
      this->fAppIdentTextMode = append_ident_text_mode::INIT_IDENT;
      break;
    }

    // Tag '@table':
    // -------------
    case tag_type::TABLE:
    {
      // Escape from write modes the TABLE mode cannot be nested inside.
      // This is partially due to HTML not allowing some kinds of element nestings.
      EscapeFromWriteModes( { tag_block_write_mode::PLAIN_TEXT,
                              tag_block_write_mode::PARAGRAPH,
                              tag_block_write_mode::VERBATIM } );

      this->faWriteMode.emplace_back( tag_block_write_mode::TABLE );
      this->oaChunkList.emplace_back( cont_chunk_type::START_TABLE );
      break;
    }

    // Tag '@endtable':
    // ----------------
    case tag_type::END_TABLE:
    {
      // Write mode dependend behavior:
      if( !faWriteMode.empty() )
        switch( this->faWriteMode.back() )
        {
          case tag_block_write_mode::TABLE:
          {
            this->faWriteMode.pop_back();
            this->oaChunkList.emplace_back( cont_chunk_type::END_TABLE );
            break;
          }

          case tag_block_write_mode::UL:
          {
            // Check if an unordered list is embedded into a table.
            signed int nBeforeLastWriteModeIdx = this->faWriteMode.size() - 2;
            if( nBeforeLastWriteModeIdx >= 0 )
              if( this->faWriteMode[nBeforeLastWriteModeIdx] == tag_block_write_mode::TABLE )
              {
                // => the unordered list is embedded inside a table.

                //  Close unordered list on '@endtable' and close the table.
                this->faWriteMode.pop_back();
                this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
                this->faWriteMode.pop_back();
                this->oaChunkList.emplace_back( cont_chunk_type::END_TABLE );
              }
            break;
          }
        }

      break;
    }

    // Tag '@verbatim':
    // -------------
    case tag_type::VERBATIM:
    {
      // Escape from write modes the VERBATIM mode cannot be nested inside.
      // This is partially due to HTML not allowing some kinds of element nestings.
      EscapeFromWriteModes( { tag_block_write_mode::PLAIN_TEXT,
                              tag_block_write_mode::PARAGRAPH,
                              tag_block_write_mode::VERBATIM } );

      this->faWriteMode.emplace_back( tag_block_write_mode::VERBATIM );
      this->oaChunkList.emplace_back( cont_chunk_type::START_VERBATIM );
      fVerbatimStartMode = verbatim_start_mode::INIT;
      break;
    }

    // Tag '@endverbatim':
    // ----------------
    case tag_type::END_VERBATIM:
    {
      // Write mode dependend behavior:
      if( !faWriteMode.empty() )
        switch( this->faWriteMode.back() )
        {
          case tag_block_write_mode::VERBATIM:
          {
            this->faWriteMode.pop_back();
            this->oaChunkList.emplace_back( cont_chunk_type::END_VERBATIM );
            break;
          }
        }

      break;
    }
  }

  // Save newline parsing state.
  fNewLine = false;
}

// .............................................................................

void escrido::CTagBlock::AppendNewLine()
{
  // Always: save newline parsing state.
  fNewLine = true;

  // Verbatim start mode dependend behavior: skip first line.
  if( fVerbatimStartMode == verbatim_start_mode::INIT )
  {
    fVerbatimStartMode = verbatim_start_mode::OFF;
    return;
  }

  // Write mode dependent behavior:
  if( !faWriteMode.empty() )
    switch( faWriteMode.back() )
    {
      case tag_block_write_mode::TITLE_LINE:
      {
        // Delimitate title line:
        this->oaChunkList.emplace_back( cont_chunk_type::DELIM_TITLE_LINE );
        this->faWriteMode.pop_back();
        break;
      }

      case tag_block_write_mode::VERBATIM:
        this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
        break;
    }

  // Tag block type specific behavior:
  switch( this->fType )
  {
    case tag_type::EXAMPLE:
      this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
      break;

    case tag_type::OUTPUT:
      this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
      break;
  }

  // => All other line breaks are dumped.
}

// .............................................................................

void escrido::CTagBlock::AppendDoubleNewLine()
{
  // Always: save newline parsing state.
  fNewLine = true;

  // Verbatim start mode dependend behavior: skip first line.
  if( fVerbatimStartMode == verbatim_start_mode::INIT )
  {
    fVerbatimStartMode = verbatim_start_mode::OFF;
    this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
    return;
  }

  // Write mode dependend behavior:
  if( !faWriteMode.empty() )
    switch( this->faWriteMode.back() )
    {
      case tag_block_write_mode::TITLE_LINE:
      {
        // Delimitate title line:
        this->faWriteMode.pop_back();
        this->oaChunkList.emplace_back( cont_chunk_type::DELIM_TITLE_LINE );
        break;
      }

      case tag_block_write_mode::PLAIN_TEXT:
      {
        this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
        this->oaChunkList.emplace_back( cont_chunk_type::NEW_LINE );
        break;
      }

      case tag_block_write_mode::PARAGRAPH:
      {
        // Double line ends paragraph.
        this->faWriteMode.pop_back();
        this->oaChunkList.emplace_back( cont_chunk_type::END_PARAGRAPH );
        break;
      }

      case tag_block_write_mode::UL:
      {
        // Double line ends unordered list.
        this->faWriteMode.pop_back();
        this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
        break;
      }
    }
}

// .............................................................................

void escrido::CTagBlock::WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  switch( this->fType )
  {
    case tag_type::ATTRIBUTE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt>";
      WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</dt>" << std::endl;
      WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
      break;
    }

    case tag_type::FEATURE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt>";
      WriteHTMLTitleLineButFirstWordOrQuote( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</dt>" << std::endl;
      WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
      WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
      break;
    }

    case tag_type::PARAM:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt>";
      WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</dt>" << std::endl;
      WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
      WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
      break;
    }

    case tag_type::SEE:
    {
      if( !this->oaChunkList.empty() )
      {
        WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li>";
        size_t nRefIdx;
        if( oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->GetPlainFirstWord() ), nRefIdx ) )
        {
          oOutStrm_i << "<a href=\"" + oWriteInfo_i.oRefTable.GetLink( nRefIdx ) + "\">";
          oOutStrm_i << oWriteInfo_i.oRefTable.GetText( nRefIdx );
          oOutStrm_i << "</a>";
        }
        else
          WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "</li>"<< std::endl;
      }
      break;
    }

    case tag_type::SIGNATURE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li>";
      WriteHTMLTitleLine( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</li>" << std::endl;
      break;
    }

    default:
    {
      for( size_t c = 0; c < this->oaChunkList.size(); c++ )
        this->oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
      break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the first word (i.e. the first group of non-whitespace
///             characters) of the tag block as HTML into the stream.
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the first word was written.
  for( size_t c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i ) )
      return;
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) of the tag block as HTML into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Write all chunks up to the title line delimitor.
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off on new line or new paragraph.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) without the first word of the tag block as
///             HTML into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLTitleLineButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the something after the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    if( oaChunkList[c].WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) without the first word or quote of the
///             tag block as HTML into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLTitleLineButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the something after the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    if( oaChunkList[c].WriteHTMLAllButFirstWordOrQuote( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes all the content without the first word of the tag block
///             as HTML into the stream.
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until something but the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      // Break if the first time a chunk has a first word and did NOT write it.
      break;
    else
      // Write all chunks containing no first word but control commands instead.
      oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );

  // Write full remaining chunks;
  for( c++; c < oaChunkList.size(); c++ )
    oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes all the content without the title line (i.e. the first
///             line up to the title delimitor chunk) of the tag block as HTML
///             into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteHTMLAllButTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the title line delimitor is found.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      break;

  // Write full remaining chunks;
  for( c++; c < oaChunkList.size(); c++ )
    oaChunkList[c].WriteHTML( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

void escrido::CTagBlock::WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  switch( this->fType )
  {
    case tag_type::ATTRIBUTE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\taglistitemexprtext{";
      WriteLaTeXFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}{";
      WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}%" << std::endl;
      break;
    }

    case tag_type::FEATURE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\taglistitemexprtext{";
      WriteLaTeXTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}{";
      WriteLaTeXAllButTitleLine( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}%" << std::endl;
      break;
    }

    case tag_type::PARAM:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\taglistitemexprtext{";
      WriteLaTeXFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}{";
      WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}%" << std::endl;
      break;
    }

    case tag_type::SEE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\item ";
      size_t nRefIdx;
      if( oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->GetPlainFirstWord() ), nRefIdx ) )
      {
        oOutStrm_i << "\\hyperref[" << MakeIdentifier( this->GetPlainFirstWord() ) << "]{";
        oOutStrm_i << ConvertHTML2LaTeX( oWriteInfo_i.oRefTable.GetText( nRefIdx ) );
        oOutStrm_i << "}%";
      }
      else
        WriteLaTeXFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << std::endl;
      break;
    }

    case tag_type::SIGNATURE:
    {
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\taglistitemline{";
      WriteLaTeXTitleLine( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "}%" << std::endl;
      break;
    }

    default:
    {
      for( size_t c = 0; c < this->oaChunkList.size(); c++ )
        this->oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
      break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the first word (i.e. the first group of non-whitespace
///             characters) of the tag block as LaTeX into the stream.
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the first word was written.
  for( size_t c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].WriteLaTeXFirstWord( oOutStrm_i, oWriteInfo_i ) )
      return;
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) of the tag block as LaTeX into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Write all chunks up to the title line delimitor.
  for( size_t c = 0; c < oaChunkList.size(); ++c )
  {
    // Break off if the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Do not display certain types of chunks.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) without the first word of the tag block as
///             LaTeX into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXTitleLineButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the something after the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off if the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Do not display certain types of chunks.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    if( oaChunkList[c].WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off if the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Do not display certain types of chunks.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the title line (i.e. the first line up to the title
///             delimitor chunk) without the first word or quote of the
///             tag block as LaTeX into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXTitleLineButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the something after the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    if( oaChunkList[c].WriteLaTeXAllButFirstWordOrQuote( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    // Skip writing start and end paragraphs.
    if( oaChunkList[c].GetType() == cont_chunk_type::START_PARAGRAPH ||
        oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
      continue;

    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes all the content without the first word of the tag block
///             as LaTeX into the stream.
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the something but the first word was written.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      break;

  // Write full remaining chunks;
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Avoid writing the last END_PARAGRAPH chunk.
    // This is for avoiding unnecessary section break.
    if( c + 1 == oaChunkList.size() )
      if( oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
        break;

    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes all the content without the title line (i.e. the first
///             line up to the title delimitor chunk) of the tag block as LaTeX
///             into the stream.
///
/// \remark     This works only for tag block types that include a title line
///             delimitor (e.g. section).
// *****************************************************************************

void escrido::CTagBlock::WriteLaTeXAllButTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Set pointer to this tag block.
  oWriteInfo_i.pTagBlock = this;

  // Loop through all text chunks until the title line delimitor is found.
  size_t c = 0;
  for( c = 0; c < oaChunkList.size(); c++ )
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      break;

  // Write full remaining chunks;
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Avoid writing the last END_PARAGRAPH chunk.
    // This is for avoiding unnecessary section break.
    if( c + 1 == oaChunkList.size() )
      if( oaChunkList[c].GetType() == cont_chunk_type::END_PARAGRAPH )
        break;

    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

void escrido::CTagBlock::DebugOutput() const
{
  std::cout << "block " << (unsigned long long) this << " tag type: ";
  for( size_t btt = 0; btt < nBlockTagTypeN; btt++ )
    if( oaBlockTagTypeList[btt].fType == fType )
    {
      std::cout << "'" << oaBlockTagTypeList[btt].szName << "'";
      break;
    }
  std::cout << "(" << (int) fType << ")" << std::endl;
  std::cout << "| ";
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    oaChunkList[c].DebugOutput();
    std::cout << " | ";
  }
  std::cout << std::endl;
}

// .............................................................................

// *****************************************************************************
/// \brief      Appends a character to the latest text content chunk.
///
/// \details    The function enforces that there is a final text chunk (either
///             of type cont_chunk_type::PLAIN_TEXT or of type
///             cont_chunk_type::HTML_TEXT) and adds the character.
///
/// \remark     The function is unlike \ref AppendChar() in the sense that it
///             works on a lower level and does not perform any parsing/grammar/
///             rules checking.
// *****************************************************************************

void escrido::CTagBlock::AppendCharDefault( const char cChar_i )
{
  // Define the default text chunk type for this block type.
  cont_chunk_type fTextChunkType;
  switch( fType )
  {
    // Block tags that accept plain text only:
    case tag_type::EXAMPLE:
    case tag_type::OUTPUT:
    case tag_type::SIGNATURE:
      fTextChunkType = cont_chunk_type::PLAIN_TEXT;
      break;

    // Other tag blocks:
    default:

      // Inline tag tag_type::VERBATIM accepts plain text,
      // all other tags accept HTML text
      if( !faWriteMode.empty() &&
          faWriteMode.back() == tag_block_write_mode::VERBATIM )
        fTextChunkType = cont_chunk_type::PLAIN_TEXT;
      else
        fTextChunkType = cont_chunk_type::HTML_TEXT;
      break;
  }

  // Eventually change write mode and append required chunks.
  if( faWriteMode.empty() )
  {
    switch( fTextChunkType )
    {
      case cont_chunk_type::PLAIN_TEXT:
        faWriteMode.emplace_back( tag_block_write_mode::PLAIN_TEXT );
        break;

      case cont_chunk_type::HTML_TEXT:
        faWriteMode.emplace_back( tag_block_write_mode::PARAGRAPH );
        oaChunkList.emplace_back( cont_chunk_type::START_PARAGRAPH );
        break;
    }
  }

  // Check if the last chunk in the chunklist is a text chunk. If not,
  // add one.
  // Attention: also tag blocks that are native with cont_chunk_type::HTML_TEXT
  //            may end on a cont_chunk_type::PLAIN_TEXT chunk, e.g. if a @code
  //            inline tag was appended. This should be treated as correct then.
  if( oaChunkList.empty() )
    oaChunkList.emplace_back( fTextChunkType );
  else
    if( !( oaChunkList.back().GetType() == cont_chunk_type::PLAIN_TEXT ||
           oaChunkList.back().GetType() == cont_chunk_type::HTML_TEXT ) )
      oaChunkList.emplace_back( fTextChunkType );

  // Append character to the text chunk.
  oaChunkList.back().AppendChar( cChar_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Escapes from a certain text write mode nesting.
///
/// \details    Some write modes may not be nested inside others, i.e.
///             a tag_block_write_mode::TABLE must not appear inside
///             a write mode tag_block_write_mode::VERBATIM. (The reason is
///             that HTML would not allow nesting of a "table" inside a
///             "par" element.) This function allows to escape from such nestings.
///
/// \param[in]  oaWriteModes_i
///             List of write modes the function will escape from.
// *****************************************************************************

void escrido::CTagBlock::EscapeFromWriteModes( const std::vector<tag_block_write_mode>& oaWriteModes_i )
{
  // Find first occurrance of any of the write modes to escape from.
  bool fEscape = false;
  size_t nEscapeLvl;
  for( nEscapeLvl = 0; nEscapeLvl < faWriteMode.size(); ++nEscapeLvl )
  {
    // Check whether the write mode of this level is identical to any of the
    // write modes given as argument.
    for( size_t awm = 0; awm < oaWriteModes_i.size(); ++awm )
      if( faWriteMode[nEscapeLvl] == oaWriteModes_i[awm] )
      {
        fEscape = true;
        break;
      }

    if( fEscape )
      break;
  }

  // Escape until write mode nEscapeLvl.
  if( fEscape )
  {
    // Traverse all writing mode from the back until the escape level.
    for( size_t wm = faWriteMode.size(); wm > nEscapeLvl; --wm )
    {
      // Add closing chunks for certain write mode types.
      switch( faWriteMode[wm-1] )
      {
        case tag_block_write_mode::PARAGRAPH:
          this->oaChunkList.emplace_back( cont_chunk_type::END_PARAGRAPH );
          break;

        case tag_block_write_mode::TABLE:
          this->oaChunkList.emplace_back( cont_chunk_type::END_TABLE );
          break;

        case tag_block_write_mode::UL:
          this->oaChunkList.emplace_back( cont_chunk_type::END_UL );
          break;

        case tag_block_write_mode::VERBATIM:
          this->oaChunkList.emplace_back( cont_chunk_type::END_VERBATIM );
          break;
      }
    }

    // Shorten write mode stack.
    faWriteMode.resize( nEscapeLvl );
  }
}

// -----------------------------------------------------------------------------

// CLASS CContentUnit

// -----------------------------------------------------------------------------

escrido::CContentUnit::CContentUnit():
  fContUnitType ( cont_unit_type::UNSET ),
  fParseState   { parse_state::LINE_BREAK, parse_state::DEFAULT, parse_state::DEFAULT }
{
  oaBlockList.resize( 1 );
}

// .............................................................................

bool escrido::CContentUnit::Empty() const
{
  if( oaBlockList.size() == 0 )
    return true;
  else
    if( oaBlockList.size() == 1 )
      if( oaBlockList[0].Empty() )
        return true;

  return false;
}

// .............................................................................

// *****************************************************************************
/// \brief      Resets the parsing state and setting but not the tag block list.
// *****************************************************************************

void escrido::CContentUnit::ResetParseState( cont_unit_type fContUnitType_i )
{
  fContUnitType = fContUnitType_i;
  fParseState[0] = parse_state::LINE_BREAK;
  fParseState[1] = parse_state::DEFAULT;
  fParseState[2] = parse_state::DEFAULT;
}

// .............................................................................

// *****************************************************************************
/// \brief      Resets the tag block list but not the parsing state and
///             settings.
// *****************************************************************************

void escrido::CContentUnit::ResetContent()
{
  // Clear tag block list and append a first empty tag block.
  oaBlockList.clear();
  oaBlockList.resize( 1 );
}

// .............................................................................

void escrido::CContentUnit::CloseWrite()
{
  if( !oaBlockList.empty() )
    oaBlockList.back().CloseWrite();
}

// .............................................................................

// *****************************************************************************
/// \brief      Appends a complete content unit and adopts parsing state and
///             settings.
// *****************************************************************************

void escrido::CContentUnit::AppendContentUnit( const CContentUnit& oContUnit_i )
{
  // Adopt parsing settings.
  fContUnitType = oContUnit_i.fContUnitType;
  fParseState[0] = oContUnit_i.fParseState[0];
  fParseState[1] = oContUnit_i.fParseState[1];
  fParseState[2] = oContUnit_i.fParseState[2];

  // Either copy or append complete tag block list.
  if( this->Empty() )
  {
    // Copy tag block list. This overwrites the first empty tag block.
    oaBlockList = oContUnit_i.oaBlockList;
  }
  else
  {
    // Append tag block list.
    oaBlockList.insert( oaBlockList.end(),
                        oContUnit_i.oaBlockList.begin(),
                        oContUnit_i.oaBlockList.end() );
  }
}

// .............................................................................

void escrido::CContentUnit::AppendLineBreak()
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT ||
      oaBlockList.back().GetWriteMode() == tag_block_write_mode::VERBATIM )
  {
    // => Verbatim mode.

    oaBlockList.back().AppendNewLine();
  }
  else
  {
    // => Non-verbatim mode.

    switch( fContUnitType )
    {
      case cont_unit_type::MULTI_LINE:
      {
        // Eventually cast an earlier line break to a newline.
        if( fParseState[0] == parse_state::LINE_BREAK )
        {
          SetParseState( parse_state::NEW_LINE );

          // Check against double or single new line.
          if( fParseState[0] == parse_state::NEW_LINE &&
              fParseState[1] == parse_state::LINE_BREAK &&
              fParseState[2] == parse_state::NEW_LINE )
          {
            // => Double new line.
            oaBlockList.back().AppendDoubleNewLine();
          }
          else
          {
            // => Single new line.

            oaBlockList.back().AppendNewLine();
          }
        }
        else
        {
          // Otherwise add a blank space.
          oaBlockList.back().AppendChar( ' ' );
        }

        // Set parsing state to actualized value.
        SetParseState( parse_state::LINE_BREAK );
        break;
      }

      case cont_unit_type::SINGLE_LINE:
      {
        SetParseState( parse_state::NEW_LINE );

        // Check against double newline.
        if( fParseState[0] == parse_state::NEW_LINE &&
            fParseState[1] == parse_state::NEW_LINE )
        {
          // => Double new line.
          oaBlockList.back().AppendDoubleNewLine();
        }
        else
        {
          // => Single new line.

          oaBlockList.back().AppendNewLine();
        }

        break;
      }
    }
  }
}

// .............................................................................

void escrido::CContentUnit::AppendBlank()
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT ||
      oaBlockList.back().GetWriteMode() == tag_block_write_mode::VERBATIM )
  {
    // => Verbatim mode.

    oaBlockList.back().AppendChar( ' ' );
  }
  else
  {
    // => Non-verbatim mode.

    if( fParseState[0] != parse_state::LINE_BREAK )
      oaBlockList.back().AppendChar( ' ' );
  }
}

// .............................................................................

void escrido::CContentUnit::AppendTab()
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT ||
      oaBlockList.back().GetWriteMode() == tag_block_write_mode::VERBATIM )
  {
    // => Verbatim mode.

    oaBlockList.back().AppendChar( ' ' );
    oaBlockList.back().AppendChar( ' ' );
  }
  else
  {
    // => Non-verbatim mode.

    if( fParseState[0] != parse_state::LINE_BREAK )
    {
      oaBlockList.back().AppendChar( ' ' );
      oaBlockList.back().AppendChar( ' ' );
    }
  }
}

// .............................................................................

void escrido::CContentUnit::AppendChar( const char cChar_i )
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT ||
      oaBlockList.back().GetWriteMode() == tag_block_write_mode::VERBATIM )
  {
    // => Verbatim mode.
  }
  else
  {
    // => Non-verbatim mode.

    // Special cases for line breaks/new lines in multi-line units.
    // (This section appears in a very similar form in AppendChar(), AppendTag()
    // and AppendLineBreak(). The handling cannot be done solely in
    // AppendLineBreak() since there is an exception in AppendChar() for
    // ignoring initial '*' characters.)
    if( fContUnitType == cont_unit_type::MULTI_LINE )
    {
      if( fParseState[0] == parse_state::LINE_BREAK )
      {
        // Handle as new line.
        SetParseState( parse_state::NEW_LINE );

        // Check against double or single new line.
        if( fParseState[0] == parse_state::NEW_LINE &&
            fParseState[1] == parse_state::LINE_BREAK &&
            fParseState[2] == parse_state::NEW_LINE )
        {
          // => Double new line.
          oaBlockList.back().AppendDoubleNewLine();
        }
        else
        {
          // => Single new line.

          oaBlockList.back().AppendNewLine();
        }

        // Ignore special character '*' at the beginning of a new line in order
        // to be consistent with the two markup forms
        // /*#
        //  *     <- Newline. Character '*' must be ignored.
        // #*/
        // and
        // /*#
        //        <- Newline.
        // #*/
        if( cChar_i == '*' )
          return;
      }
    }
  }

  // => Handle as additional character to the unit.
  oaBlockList.back().AppendChar( cChar_i );
  SetParseState( parse_state::DEFAULT );
}

// .............................................................................

void escrido::CContentUnit::AppendTag( const char* szTagName_i )
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim block.

    tag_type fTagType;
    if( GetBlockTagType( szTagName_i, fTagType ) )
    {
      // => The tag is a block tag.

      // Add a new block to the tag block list.
      oaBlockList.back().CloseWrite();
      oaBlockList.emplace_back( fTagType );
      SetParseState( parse_state::DEFAULT );
    }
    else
    {
      // => The tag is an inline tag.

      // Append tag name as a string.
      oaBlockList.back().AppendChar( '@' );
      const size_t nLen = strlen( szTagName_i );
      for( size_t c = 0; c < nLen; ++c )
        oaBlockList.back().AppendChar( szTagName_i[c] );
    }
  }
  else
  {
    // => Not inside block or not inside verbatim type.

    // Special case: write mode of inline tag tag_type::VERBATIM.
    if( oaBlockList.back().GetWriteMode() == tag_block_write_mode::VERBATIM )
    {
      // Only relevant tag in verbatim mode is the @endverbatim inline tag.
      tag_type fTagType;
      if( GetInlineTagType( szTagName_i, fTagType ) )
        if( fTagType == tag_type::END_VERBATIM )
        {
          oaBlockList.back().AppendInlineTag( fTagType );
        }
        else
        {
          // Append tag name as a string.
          oaBlockList.back().AppendChar( '@' );
          const size_t nLen = strlen( szTagName_i );
          for( size_t c = 0; c < nLen; ++c )
            oaBlockList.back().AppendChar( szTagName_i[c] );
        }

      return;
    }

    // Special cases for line breaks/new lines in multi-line units.
    // (This section appears in a very similar form in AppendChar(), AppendTag()
    // and AppendLineBreak(). The handling cannot be done solely in
    // AppendLineBreak() since there is an exception in AppendChar() for
    // ignoring initial '*' characters.)
    if( fContUnitType == cont_unit_type::MULTI_LINE )
    {
      if( fParseState[0] == parse_state::LINE_BREAK )
      {
        // Handle as new line.
        SetParseState( parse_state::NEW_LINE );

        // Check against double or single new line.
        if( fParseState[0] == parse_state::NEW_LINE &&
            fParseState[1] == parse_state::LINE_BREAK &&
            fParseState[2] == parse_state::NEW_LINE )
        {
          // => Double new line.
          oaBlockList.back().AppendDoubleNewLine();
        }
        else
        {
          // => Single new line.

          oaBlockList.back().AppendNewLine();
        }
      }
    }

    tag_type fTagType;
    if( GetBlockTagType( szTagName_i, fTagType ) )
    {
      // => The tag is a block tag.

      if( !( this->fParseState[0] == parse_state::NEW_LINE ||
             this->fParseState[0] == parse_state::LINE_BREAK ) )
      {
        std::cerr << "block tag '@" << szTagName_i << "' not starting in new line" << std::endl;
        return;
      }

      // Check, if the last tag block is an empty paragraph type block (first default block).
      if( oaBlockList.back().GetTagType() == tag_type::PARAGRAPH &&
          oaBlockList.back().Empty() )
      {
        // => The last tag block is an empty paragraph type block:
        //    convert it into the requested tag block type.
        oaBlockList.back().SetTagType( fTagType );
      }
      else
      {
        // => Add a new block to the tag block list.
        oaBlockList.back().CloseWrite();
        oaBlockList.emplace_back( fTagType );
        SetParseState( parse_state::DEFAULT );
      }
    }
    else
      if( GetInlineTagType( szTagName_i, fTagType ) )
      {
        // => The tag is an inline tag.

        oaBlockList.back().AppendInlineTag( fTagType );
      }
      else
        std::cerr << "unrecognized tag '@" << szTagName_i << "'" << std::endl;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the number of tag blocks of the complete unit.
// *****************************************************************************

size_t escrido::CContentUnit::GetTagBlockN() const
{
  return this->oaBlockList.size();
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns a specific tag block according to its index value.
// *****************************************************************************

const escrido::CTagBlock& escrido::CContentUnit::GetTagBlock( size_t nTagBlockIdx_i ) const
{
  return oaBlockList[nTagBlockIdx_i];
}

// .............................................................................

bool escrido::CContentUnit::HasTagBlock( tag_type fTagType_i ) const
{
  for( size_t t = 0; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].fType == fTagType_i )
      return true;

  return false;
}


// .............................................................................

// *****************************************************************************
/// \brief      Returns the number of tag blocks of a specific type.
// *****************************************************************************

size_t escrido::CContentUnit::GetTagBlockN( tag_type fType_i ) const
{
  unsigned int nCountN = 0;
  for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
    if( this->oaBlockList[b].GetTagType() == fType_i )
      nCountN++;
  return nCountN;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns the first tag block of a certain type.
///
/// \see        GetNextTagBlock
///
/// \param[in]  fTagType_i
///             Defines the type of tag block that is returned.
///
/// \return     A pointer to the first tag block of the given type or NULL if
///             no such block exists.
// *****************************************************************************

const escrido::CTagBlock* escrido::CContentUnit::GetFirstTagBlock( tag_type fTagType_i ) const
{
  for( size_t t = 0; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].fType == fTagType_i )
      return &oaBlockList[t];
  return NULL;
}


// *****************************************************************************
/// \brief      Returns the next tag block of a certain type after a given one.
///
/// \note       There mustn't be made any addition of tag blocks since the last
///             calling of \ref GetFirstTagBlock() in order for this function
///             to work properly.
///
/// \see        GetFirstTagBlock
///
/// \param[in]  pLast_i
///             Pointer to another tag block of that type. The function will
///             return the next tag block of the given type \b after this block.
///             If this is NULL, the function will return NULL;
/// \param[in]  fTagType_i
///             Defines the type of tag block that is returned.
///
/// \return     A pointer to the next tag block of the given type or NULL if
///             no such block exists.
// *****************************************************************************

const escrido::CTagBlock* escrido::CContentUnit::GetNextTagBlock( const CTagBlock* pLast_i, tag_type fTagType_i ) const
{
  if( pLast_i == NULL )
    return NULL;

  // Calculate position of the last tag block.
  size_t nIdx = pLast_i - &oaBlockList.front();

  // Search after the last position.
  for( size_t t = nIdx + 1; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].fType == fTagType_i )
      return &oaBlockList[t];
  return NULL;
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the "flowing text" tag blocks PARAGRAPH, SECTION,
///             SUBSECTION, SUBSUBSECTION, DETAILS and embedded EXAMPLE, IMAGE,
///             INTERNAL, NOTES, OUTPUT and REMARK in a standardized way.
///
/// \todo       TODO: Implement correct nesting of sections and subsections.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Text section type blocks: default, section or details blocks in their order
  // of their appearance.
  {
    // Flag whether the writing is inside one or multiple "details" tag block(s).
    bool fInDetails = false;

    // Loop over all text blocks.
    for( size_t t = 0; t < oaBlockList.size(); t++ )
    {
      switch( oaBlockList[t].GetTagType() )
      {
        case tag_type::PARAGRAPH:
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          break;

        case tag_type::DETAILS:
          if( !fInDetails )
          {
            WriteHTMLTagLine( "<section class=\"tagblock details\">", oOutStrm_i, oWriteInfo_i++ );
            WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                         << oWriteInfo_i.Label( "Details" )
                                                         << "</h2>" << std::endl;
            fInDetails = true;
          }
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          break;

        case tag_type::SECTION:
        {
          // Eventually leave "details" section.
          if( fInDetails )
          {
            WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
            fInDetails = false;
          }

          // Surrounding "<section>".
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<section id=\""
                                                         << oaBlockList[t].GetPlainFirstWord()
                                                         << "\" class=\"tagblock section\">" << std::endl;

          // Title line.
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</h2>" << std::endl;

          // Content and terminating "</section>".
          oaBlockList[t].WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::SUBSECTION:
        {
          // Surrounding "<section>".
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<section id=\""
                                                         << oaBlockList[t].GetPlainFirstWord()
                                                         << "\" class=\"tagblock subsection\">" << std::endl;

          // Title line.
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h3>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</h3>" << std::endl;

          // Content and terminating "</section>".
          oaBlockList[t].WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::SUBSUBSECTION:
        {
          // Surrounding "<section>".
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<section id=\""
                                                         << oaBlockList[t].GetPlainFirstWord()
                                                         << "\" class=\"tagblock subsubsection\">" << std::endl;

          // Title line.
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h4>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</h4>" << std::endl;

          // Content and terminating "</section>".
          oaBlockList[t].WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::EXAMPLE:
        {
          const std::string sTagLine = std::string( "<h4>" ) + oWriteInfo_i.Label( "Example" ) + "</h4>";

          WriteHTMLTagLine( "<div class=\"tagblock examples\">", oOutStrm_i, oWriteInfo_i++ );
          WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<pre class=\"example\">";
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</pre>" << std::endl;
          WriteHTMLTagLine( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::IMAGE:
        {
          WriteHTMLTagLine( "<figure class=\"image\">", oOutStrm_i, oWriteInfo_i++ );
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<img src=\"" << oaBlockList[t].GetPlainFirstWord() << "\">" << std::endl;
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<figcaption>";
          oaBlockList[t].WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</figcaption>" << std::endl;
          WriteHTMLTagLine( "</figure>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::INTERNAL:
        {
          if( oWriteInfo_i.fInternalTags )
          {
            const std::string sTagLine = std::string( "<h4>" ) + oWriteInfo_i.Label( "Internal" ) + "</h4>";

            WriteHTMLTagLine( "<div class=\"internal\">", oOutStrm_i, oWriteInfo_i++ );
            WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
            oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
            WriteHTMLTagLine( "</div>", oOutStrm_i, --oWriteInfo_i );
          }
          break;
        }

        case tag_type::NOTE:
        {
          const std::string sTagLine = std::string( "<h4>" ) + oWriteInfo_i.Label( "Note" ) + "</h4>";

          WriteHTMLTagLine( "<div class=\"note\">", oOutStrm_i, oWriteInfo_i++ );
          WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTagLine( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::OUTPUT:
        {
          const std::string sTagLine = std::string( "<h4>" ) + oWriteInfo_i.Label( "Output" ) + "</h4>";

          WriteHTMLTagLine( "<div class=\"output\">", oOutStrm_i, oWriteInfo_i++ );
          WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<pre class=\"output\">";
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</pre>" << std::endl;
          WriteHTMLTagLine( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::REMARK:
        {
          const std::string sTagLine = std::string( "<h4>" ) + oWriteInfo_i.Label( "Remark" ) + "</h4>";

          WriteHTMLTagLine( "<div class=\"remark\">", oOutStrm_i, oWriteInfo_i++ );
          WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTagLine( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }
      }
    }

    // Eventually leave last "details" section.
    if( fInDetails )
    {
      WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
      fInDetails = false;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a tag blocks of a certain type (available for special
///             tag types only).
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::BRIEF
///             - \ref tag_type::RETURN
///
/// \param[in]  fTagType_i
///             Type of tag blocks that is written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag block is written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLTagBlock( tag_type fTagType_i,
                                               std::ostream& oOutStrm_i,
                                               const SWriteInfo& oWriteInfo_i ) const
{
  if( this->HasTagBlock( fTagType_i ) )
  {
    // Write tag block.
    switch( fTagType_i )
    {
      case tag_type::BRIEF:
        this->GetFirstTagBlock( tag_type::BRIEF )->WriteHTML( oOutStrm_i, oWriteInfo_i );
        break;

      case tag_type::RETURN:
        WriteHTMLTagLine( "<section class=\"tagblock return\">", oOutStrm_i, oWriteInfo_i++ );
        WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                     << oWriteInfo_i.Label( "Return value" )
                                                     << "</h2>" << std::endl;
        this->GetFirstTagBlock( tag_type::RETURN )->WriteHTML( oOutStrm_i, oWriteInfo_i );
        WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
        break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a tag blocks of a certain type (available for special
///             tag types only) identified by a certain identifier.
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::FEATURE
///
/// \param[in]  fTagType_i
///             Type of tag blocks that is written.
/// \param[in]  sIdentifier_i
///             Identifier of the tag block that is written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag block is written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLTagBlock( tag_type fTagType_i,
                                               const std::string& sIdentifier_i,
                                               std::ostream& oOutStrm_i,
                                               const SWriteInfo& oWriteInfo_i ) const
{
  // Write tag block.
  switch( fTagType_i )
  {
    case tag_type::FEATURE:
    {
      // Check if at least one feature tag block of the feature type as given by
      // the identifier exists.
      bool fIdentFeatExists = false;
      for( size_t t = 0; t < oaBlockList.size(); t++ )
        if( oaBlockList[t].GetTagType() == fTagType_i &&
            oaBlockList[t].GetPlainFirstWordOrQuote() == sIdentifier_i )
        {
          fIdentFeatExists = true;
          break;
        }

      if( fIdentFeatExists )
      {
        // Write HTML elements.
        this->WriteHTMLFeatureType( sIdentifier_i, oOutStrm_i, oWriteInfo_i );
      }

      break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a list of tag blocks of one type (available for special
///             tag types only).
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::ATTRIBUTE
///             - \ref tag_type::PARAM
///             - \ref tag_type::SEE
///             - \ref tag_type::SIGNATURE
///
/// \param[in]  fTagType_i
///             Type of which all tag blocks are written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag blocks are written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLTagBlockList( tag_type fTagType_i,
                                                   std::ostream& oOutStrm_i,
                                                   const SWriteInfo& oWriteInfo_i ) const
{
  if( this->HasTagBlock( fTagType_i ) )
  {
    // Tag type FEATURE:
    if( fTagType_i == tag_type::FEATURE )
    {
      // Create a list of all feature types that were found.
      std::list <std::string> saFeatureTypeList;
      for( size_t t = 0; t < oaBlockList.size(); t++ )
        if( oaBlockList[t].GetTagType() == tag_type::FEATURE )
          saFeatureTypeList.emplace_back( oaBlockList[t].GetPlainFirstWordOrQuote() );

      // Sort the feature types alphanumerically.
      saFeatureTypeList.sort();

      // Loop over feature types.
      std::list <std::string>::iterator iFeatType = saFeatureTypeList.begin();
      while( iFeatType != saFeatureTypeList.end() )
      {
        // Write HTML elements.
        this->WriteHTMLFeatureType( *iFeatType, oOutStrm_i, oWriteInfo_i );

        // Cycle to next feature type.
        std::list <std::string>::iterator iFeat = iFeatType;
        while( iFeat != saFeatureTypeList.end() &&
               *iFeat == *iFeatType )
          ++iFeat;
        iFeatType = iFeat;
      }

      return;
    }

    // Write tag block opening and title line for some tag types.
    switch( fTagType_i )
    {
       case tag_type::ATTRIBUTE:
         WriteHTMLTagLine( "<section class=\"tagblock attributes\">", oOutStrm_i, oWriteInfo_i++ );
         WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                      << oWriteInfo_i.Label( "Attributes" )
                                                      << "</h2>" << std::endl;
         break;

       case tag_type::PARAM:
         WriteHTMLTagLine( "<section class=\"tagblock parameters\">", oOutStrm_i, oWriteInfo_i++ );
         WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                      << oWriteInfo_i.Label( "Parameters" )
                                                      << "</h2>" << std::endl;
         break;

       case tag_type::SEE:
         WriteHTMLTagLine( "<section class=\"tagblock see\">", oOutStrm_i, oWriteInfo_i++ );
         WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                      << oWriteInfo_i.Label( "See also" )
                                                      << "</h2>" << std::endl;
         break;

       case tag_type::SIGNATURE:
         WriteHTMLTagLine( "<section class=\"tagblock signatures\">", oOutStrm_i, oWriteInfo_i++ );
         WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>"
                                                      << oWriteInfo_i.Label( "Signatures" )
                                                      << "</h2>" << std::endl;
         break;
    }

    // Write tag block list.
    switch( fTagType_i )
    {
      case tag_type::SEE:
      case tag_type::SIGNATURE:
      {
        WriteHTMLTagLine( "<ul>", oOutStrm_i, oWriteInfo_i++ );

        // Loop over all tag blocks.
        for( size_t t = 0; t < oaBlockList.size(); t++ )
          if( oaBlockList[t].GetTagType() == fTagType_i )
            oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );

        WriteHTMLTagLine( "</ul>", oOutStrm_i, --oWriteInfo_i );
        break;
      }

      default:
      {
        WriteHTMLTagLine( "<dl>", oOutStrm_i, oWriteInfo_i++ );

        // Loop over all tag blocks.
        for( size_t t = 0; t < oaBlockList.size(); t++ )
          if( oaBlockList[t].GetTagType() == fTagType_i )
            oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );

        WriteHTMLTagLine( "</dl>", oOutStrm_i, --oWriteInfo_i );

        break;
      }
    }

    // Write tag block closing line.
    WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the PARAGRAPH, the SECTION, the DETAILS and embedded
///             EXAMPLE, IMAGE, INTERNAL, NOTES, OUTPUT and REMARK tag blocks in a
///             standardized way.
// *****************************************************************************

void escrido::CContentUnit::WriteLaTeXParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Text section type blocks: default, section or details blocks in their order
  // of their appearance.
  {
    // Flag whether the writing is inside one or multiple "details" tag block(s).
    bool fInDetails = false;

    // Loop over all text blocks.
    for( size_t t = 0; t < oaBlockList.size(); t++ )
    {
      switch( oaBlockList[t].GetTagType() )
      {
        case tag_type::PARAGRAPH:
          oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
          break;

        case tag_type::DETAILS:
        {
          if( !fInDetails )
          {
            oOutStrm_i << "\\tagblocksection{" << oWriteInfo_i.Label( "Details" ) << "}%" << std::endl << std::endl;
            fInDetails = true;
          }

          oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << std::endl << std::endl;

          break;
        }

        case tag_type::SECTION:
        {
          if( fInDetails )
            fInDetails = false;

          // Title line.
          oOutStrm_i << "\\tagblocksection{";
          oaBlockList[t].WriteLaTeXTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "}%" << std::endl
                     << "\\label{" << oaBlockList[t].GetPlainFirstWord() << "}%" << std::endl << std::endl;

          // Content.
          oaBlockList[t].WriteLaTeXAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << std::endl << std::endl;

          break;
        }

        case tag_type::SUBSECTION:
        {
          // Title line.
          oOutStrm_i << "\\tagblocksubsection{";
          oaBlockList[t].WriteLaTeXTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "}%" << std::endl
                     << "\\label{" << oaBlockList[t].GetPlainFirstWord() << "}%" << std::endl << std::endl;

          // Content.
          oaBlockList[t].WriteLaTeXAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << std::endl << std::endl;
          break;
        }

        case tag_type::SUBSUBSECTION:
        {
          // Title line.
          oOutStrm_i << "\\tagblocksubsubsection{";
          oaBlockList[t].WriteLaTeXTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "}%" << std::endl
                     << "\\label{" << oaBlockList[t].GetPlainFirstWord() << "}%" << std::endl << std::endl;

          // Content.
          oaBlockList[t].WriteLaTeXAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << std::endl << std::endl;
          break;
        }

        case tag_type::EXAMPLE:
        {
          oOutStrm_i << "\\verbatimtitle{" << oWriteInfo_i.Label( "Example" ) << "}" << std::endl
                     << "\\begin{lstlisting}" << std::endl
                     << oaBlockList[t].GetPlainText()
                     << "\\end{lstlisting}" << std::endl;
          break;
        }

        case tag_type::IMAGE:
        {
          oOutStrm_i << "\\begin{minipage}{\\textwidth}" << std::endl
                     << "  \\begin{center}" << std::endl
                     << "    \\includegraphics[width=\\maxwidth{\\textwidth}]{" << oaBlockList[t].GetPlainFirstWord() << "}\\\\" << std::endl
                     << "    {";
          oaBlockList[t].WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "}" << std::endl
                     << "  \\end{center}" << std::endl
                     << "\\end{minipage}" << std::endl << std::endl;
          break;
        }

        case tag_type::INTERNAL:
        {
          if( oWriteInfo_i.fInternalTags )
          {
            oOutStrm_i << "\\begin{internal}" << std::endl;
            oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
            oOutStrm_i << "\\end{internal}" << std::endl << std::endl;
          }
          break;
        }

        case tag_type::NOTE:
        {
          oOutStrm_i << "\\begin{note}" << std::endl;
          oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "\\end{note}" << std::endl << std::endl;
          break;
        }

        case tag_type::OUTPUT:
        {
          oOutStrm_i << "\\verbatimtitle{" << oWriteInfo_i.Label( "Output" ) << "}" << std::endl
                     << "\\begin{lstlisting}" << std::endl
                     << oaBlockList[t].GetPlainText()
                     << "\\end{lstlisting}" << std::endl;
          break;
        }

        case tag_type::REMARK:
        {
          oOutStrm_i << "\\begin{remark}" << std::endl;
          oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "\\end{remark}" << std::endl << std::endl;
          break;
        }
      }
    }

    if( fInDetails )
      fInDetails = false;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a tag blocks of a certain type (available for special
///             tag types only).
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::BRIEF
///             - \ref tag_type::RETURN
///
/// \param[in]  fTagType_i
///             Type of tag blocks that is written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag block is written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteLaTeXTagBlock( tag_type fTagType_i,
                                                std::ostream& oOutStrm_i,
                                                 const SWriteInfo& oWriteInfo_i ) const
{
  if( this->HasTagBlock( fTagType_i ) )
  {
    // Write tag block.
    switch( fTagType_i )
    {
      case tag_type::BRIEF:
        this->GetFirstTagBlock( tag_type::BRIEF )->WriteLaTeX( oOutStrm_i, oWriteInfo_i );
        break;

      case tag_type::RETURN:
        oOutStrm_i << "\\tagblocksection{"
                   << oWriteInfo_i.Label( "Return value" )
                   << "}" << std::endl;
        this->GetFirstTagBlock( tag_type::RETURN )->WriteLaTeX( oOutStrm_i, oWriteInfo_i );
        break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a tag blocks of a certain type (available for special
///             tag types only) identified by a certain identifier.
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::FEATURE
///
/// \param[in]  fTagType_i
///             Type of tag blocks that is written.
/// \param[in]  sIdentifier_i
///             Identifier of the tag block that is written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag block is written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteLaTeXTagBlock( tag_type fTagType_i,
                                                const std::string& sIdentifier_i,
                                                std::ostream& oOutStrm_i,
                                                const SWriteInfo& oWriteInfo_i ) const
{
  // Write tag block.
  switch( fTagType_i )
  {
    case tag_type::FEATURE:
    {
      // Check if at least one feature tag block of the feature type as given by
      // the identifier exists.
      bool fIdentFeatExists = false;
      for( size_t t = 0; t < oaBlockList.size(); t++ )
        if( oaBlockList[t].GetTagType() == fTagType_i &&
            oaBlockList[t].GetPlainFirstWordOrQuote() == sIdentifier_i )
        {
          fIdentFeatExists = true;
          break;
        }

      if( fIdentFeatExists )
      {
        // Write LaTeX elements
        this->WriteLaTeXFeatureType( sIdentifier_i, oOutStrm_i, oWriteInfo_i );
      }

      break;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes a list of tag blocks of one type (available for special
///             tag types only).
///
/// \details    This is for tag blocks of the following types only:
///             - \ref tag_type::ATTRIBUTE
///             - \ref tag_type::PARAM
///             - \ref tag_type::SEE
///             - \ref tag_type::SIGNATURE
///
/// \param[in]  fTagType_i
///             Type of which all tag blocks are written.
/// \param[in]  oOutStrm_i
///             Output stream into which the tag blocks are written.
/// \param[in]  oWriteInfo_i
///             Write-info structure with additional information.
// *****************************************************************************

void escrido::CContentUnit::WriteLaTeXTagBlockList( tag_type fTagType_i,
                                                    std::ostream& oOutStrm_i,
                                                    const SWriteInfo& oWriteInfo_i ) const
{
  if( this->HasTagBlock( fTagType_i ) )
  {
    // Tag type FEATURE:
    if( fTagType_i == tag_type::FEATURE )
    {
      // Create a list of all feature types that were found.
      std::list <std::string> saFeatureTypeList;
      for( size_t t = 0; t < oaBlockList.size(); t++ )
        if( oaBlockList[t].GetTagType() == tag_type::FEATURE )
          saFeatureTypeList.emplace_back( oaBlockList[t].GetPlainFirstWordOrQuote() );

      // Sort the feature types alphanumerically.
      saFeatureTypeList.sort();

      // Loop over feature types.
      std::list <std::string>::iterator iFeatType = saFeatureTypeList.begin();
      while( iFeatType != saFeatureTypeList.end() )
      {
        // Write LaTeX elements
        this->WriteLaTeXFeatureType( *iFeatType, oOutStrm_i, oWriteInfo_i );

        // Cycle to next feature type.
        std::list <std::string>::iterator iFeat = iFeatType;
        while( iFeat != saFeatureTypeList.end() &&
               *iFeat == *iFeatType )
          ++iFeat;
        iFeatType = iFeat;
      }

      return;
    }

    // Write tag block title line for some tag types.
    switch( fTagType_i )
    {
       case tag_type::ATTRIBUTE:
         oOutStrm_i << "\\tagblocksection{" << oWriteInfo_i.Label( "Attributes" ) << "}" << std::endl;
         break;

       case tag_type::PARAM:
         oOutStrm_i << "\\tagblocksection{" << oWriteInfo_i.Label( "Parameters" ) << "}" << std::endl;
         break;

       case tag_type::SEE:
         oOutStrm_i << "\\tagblocksection{" << oWriteInfo_i.Label( "See also" ) << "}" << std::endl;
         break;

       case tag_type::SIGNATURE:
         oOutStrm_i << "\\tagblocksection{" << oWriteInfo_i.Label( "Signatures" ) << "}" << std::endl;
         break;
    }

    // Write tag block list.
    switch( fTagType_i )
    {
      case tag_type::SEE:
      {
        oOutStrm_i << "\\begin{itemize}" << std::endl;
        ++oWriteInfo_i;

        // Write all tag blocks of the specified type.
        for( size_t t = 0; t < oaBlockList.size(); t++ )
          if( oaBlockList[t].GetTagType() == tag_type::SEE )
            oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );

        --oWriteInfo_i;
        oOutStrm_i << "\\end{itemize}" << std::endl;

        break;
      }

      default:
      {
        oOutStrm_i << "\\begin{taglist}" << std::endl;
        ++oWriteInfo_i;

        // Write all tag blocks of the specified type.
        for( size_t t = 0; t < oaBlockList.size(); t++ )
          if( oaBlockList[t].GetTagType() == fTagType_i )
            oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );

        --oWriteInfo_i;
        oOutStrm_i << "\\end{taglist}" << std::endl;

        break;
      }
    }
  }
}

// .............................................................................

void escrido::CContentUnit::DebugOutput() const
{
  switch( fContUnitType )
  {
    case cont_unit_type::SINGLE_LINE:
      std::cout << "single line content unit";
      break;

    case cont_unit_type::MULTI_LINE:
      std::cout << "multi line content unit";
      break;
  }
  std::cout << std::endl;
  for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
    this->oaBlockList[b].DebugOutput();
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the HTML content for a 'feature' tag group.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLFeatureType( const std::string& sTypeIdentifier_i,
                                                  std::ostream& oOutStrm_i,
                                                  const SWriteInfo& oWriteInfo_i ) const
{
  // Write HTML elements.
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<section class=\"tagblock features "
                                                  << GetCamelCase( sTypeIdentifier_i ) << "\">" << std::endl;
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>" << GetCapForm( sTypeIdentifier_i ) << "</h2>" << std::endl;
  WriteHTMLTagLine( "<dl>", oOutStrm_i, oWriteInfo_i++ );

  // Loop over all tag blocks.
  for( size_t t = 0; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].GetTagType() == tag_type::FEATURE )
      if( oaBlockList[t].GetPlainFirstWordOrQuote() == sTypeIdentifier_i )
        oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );

  WriteHTMLTagLine( "</dl>", oOutStrm_i, --oWriteInfo_i );
  WriteHTMLTagLine( "</section>", oOutStrm_i, --oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the LaTeX content for a 'feature' tag group.
// *****************************************************************************

void escrido::CContentUnit::WriteLaTeXFeatureType( const std::string& sTypeIdentifier_i,
                                                   std::ostream& oOutStrm_i,
                                                   const SWriteInfo& oWriteInfo_i ) const
{
  // Write tag block title line.
  oOutStrm_i << "\\tagblocksection{" << GetCapForm( sTypeIdentifier_i ) << "}" << std::endl;
  oOutStrm_i << "\\begin{taglist}" << std::endl;
  ++oWriteInfo_i;

  // Loop over all tag blocks.
  for( size_t t = 0; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].GetTagType() == tag_type::FEATURE )
      if( oaBlockList[t].GetPlainFirstWordOrQuote() == sTypeIdentifier_i )
        oaBlockList[t].WriteLaTeX( oOutStrm_i, oWriteInfo_i );

  --oWriteInfo_i;
  oOutStrm_i << "\\end{taglist}" << std::endl;
}

// .............................................................................

// *****************************************************************************
/// \brief      Sets the parsing state and shifts the "look back" states.
///
/// \details    The system can look back three parsing states. These are stored
///             by using this function.
// *****************************************************************************

void escrido::CContentUnit::SetParseState( parse_state fParseState_i )
{
  this->fParseState[2] = this->fParseState[1];
  this->fParseState[1] = this->fParseState[0];
  this->fParseState[0] = fParseState_i;
}

// -----------------------------------------------------------------------------

// FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

std::ostream& escrido::WriteHTMLIndents( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i )
{
  for( unsigned int i = 0; i < oWriteInfo_i.nIndent; i++ )
    oOutStrm_i << " ";
  return oOutStrm_i;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Writes an HTML line with the correct indentation and a following
///             line break.
// *****************************************************************************

void escrido::WriteHTMLTagLine( const std::string& sTagText_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i )
{
  WriteHTMLTagLine( sTagText_i.c_str(), oOutStrm_i, oWriteInfo_i );
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Writes an HTML line with the correct indentation and a following
///             line break.
// *****************************************************************************

void escrido::WriteHTMLTagLine( const char* szTagText_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i )
{
  for( unsigned int i = 0; i < oWriteInfo_i.nIndent; i++ )
    oOutStrm_i << " ";
  oOutStrm_i << szTagText_i << std::endl;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns a string that is HTML escaped, i.e. it will show the
///             original string when added into an html document.
// *****************************************************************************

std::string escrido::HTMLEscape( const std::string& sText_i )
{
  std::string sReturn;
  for( size_t c = 0; c < sText_i.size(); c++ )
  {
    switch( sText_i[c] )
    {
      case '<':
        sReturn += "&lt;";
        break;

      case '>':
        sReturn += "&gt;";
        break;

      case '&':
        sReturn += "&amp;";
        break;

      case ' ':
        sReturn += "&nbsp;";
        break;

      default:
        sReturn += sText_i[c];
        break;
    }
  }

  return sReturn;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns a string that is LaTeX escaped, i.e. it will show the
///             original string when added into an LaTeX document.
// *****************************************************************************

std::string escrido::LaTeXEscape( const std::string& sText_i )
{
  std::string sReturn;
  for( size_t c = 0; c < sText_i.size(); c++ )
  {
    switch( sText_i[c] )
    {
      case '$':
        sReturn += "\\$";
        break;

      case '%':
        sReturn += "\\%";
        break;

      case '_':
        sReturn += "\\_";
        break;

      case '{':
        sReturn += "\\{";
        break;

      case '}':
        sReturn += "\\}";
        break;

      case '[':
        sReturn += "{[}";
        break;

      case ']':
        sReturn += "{]}";
        break;

      case '&':
        sReturn += "\\&";
        break;

      case '#':
        sReturn += "\\#";
        break;

      // Symbol '´':
      // (use numeric form to be cross-architecture compatible)
      case '\xB4':
        sReturn += "'";
        break;

      // Symbol '°':
      // (use numeric form to be cross-architecture compatible)
      case '\xBA':
        sReturn += "{\\textdegree}";
        break;

      case '|':
        sReturn += "{\\textbar}";
        break;

      default:
        sReturn += sText_i[c];
        break;
    }
  }

  return sReturn;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Converts a string containing HTML content into pure LaTeX type
///             content and returns it.
// *****************************************************************************

std::string escrido::ConvertHTML2LaTeX( const std::string& sText_i )
{
  std::string sTextCpy( sText_i );
  for( size_t nPos = 0; nPos < sTextCpy.size(); )
  {
    // Keep order of the exchange rules from high to low precedence:

    // Very special commands:
    if( ReplaceIfMatch( sTextCpy, nPos, "LaTeX", "{\\LaTeX}" ) ) continue;

    // HTML specific commands:
    if( ReplaceIfMatch( sTextCpy, nPos, "<HR>", "\\noindent\\rule{\\textwidth}{0.4pt} " ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "<em>", "\\textit{" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "</em>", "}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "<b>", "\\textbf{" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "</b>", "}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "<sup>", "$^\\textrm{\\footnotesize " ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "</sup>", "}$" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "<sub>", "$_\\textrm{\\footnotesize " ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "</sub>", "}$" ) ) continue;

    // HTML entities:
    if( ReplaceIfMatch( sTextCpy, nPos, "&amp;", "\\&" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&gamma;", "$\\gamma$" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#42;", "*" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#124;", "{\\textbar}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#47;", "/" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#64;", "@" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&lt;", "{\\textless}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&gt;", "{\\textgreater}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#8477;", "$\\mathbb{R}$" ) ) continue;

    // Avoid certain LaTeX ligatures:
    if( ReplaceIfMatch( sTextCpy, nPos, "--", "-{}-" ) ) continue;

    // Characters:
    if( ReplaceIfMatch( sTextCpy, nPos, "$", "\\$" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "%", "\\%" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "_", "\\_" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "{", "\\{" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "}", "\\}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "[", "{[}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "]", "{]}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&", "\\&" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "#", "\\#" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "´", "'" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "°", "{\\textdegree}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "|", "{\\textbar}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "<", "{\\textless}" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, ">", "{\\textgreater}" ) ) continue;

    // Increase counter;
    nPos++;
  }

  return sTextCpy;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Converts a string containing HTML content into plain text and
///             returns it.
///
/// \details    The following modifications are applied:
///             - removing of HTML tags
///             - removing front and end whitespaces
///             - replacing multiple whitespaces by a single blank space
///             - replacing of HTML entities
// *****************************************************************************

std::string escrido::ConvertHTML2ClearText( const std::string& sText_i )
{
  // Remove HTML tags and create a text copy.
  std::string sTextCpy;
  for( size_t nPos = 0; nPos < sText_i.size(); ++nPos )
  {
    if( sText_i[nPos] == '<' )
    {
      size_t nPos2;
      for( nPos2 = nPos + 1; nPos2 < sText_i.size(); ++nPos2 )
        if( sText_i[nPos2] == '>' )
          break;

      nPos = nPos2;
    }
    else
      sTextCpy.push_back( sText_i[nPos] );
  }

  // Replace HTML entities:
  for( size_t nPos = 0; nPos < sTextCpy.size(); )
  {
    if( ReplaceIfMatch( sTextCpy, nPos, "&nbsp;", " " ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&amp;", "&" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&gamma;", "gamma" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#42;", "*" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#124;", "|" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#47;", "/" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#64;", "@" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&lt;", "<" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&gt;", ">" ) ) continue;
    if( ReplaceIfMatch( sTextCpy, nPos, "&#8477;", "R" ) ) continue;

    // Increase counter;
    nPos++;
  }

  // Contract whitespaces
  {
    // Detect front whitespaces and determine begin of content.
    size_t nBegin;
    for( nBegin = 0; nBegin < sTextCpy.size(); ++nBegin )
    {
      char nChar = sTextCpy[nBegin];

      if( !( nChar == ' ' || nChar == '\t' || nChar == '\r' || nChar == '\n' ) )
        break;
    }

    // Contract all multiple whitespaces
    {
      std::string sTextCpy2;

      for( size_t c1 = nBegin; c1 < sTextCpy.size(); ++c1 )
      {
        char nChar = sTextCpy[c1];

        if( nChar == ' ' || nChar == '\t' || nChar == '\r' || nChar == '\n' )
        {
          sTextCpy2.push_back( ' ' );

          size_t c2;
          for( c2 = c1 + 1; c2 < sTextCpy.size(); ++c2 )
          {
            nChar = sTextCpy[c2];

            if( !( nChar == ' ' || nChar == '\t' || nChar == '\r' || nChar == '\n' ) )
              break;
          }

          c1 = c2 - 1;
        }
        else
          sTextCpy2.push_back( nChar );
      }

      sTextCpy2.swap( sTextCpy );
    }

    // Eventually remove terminal whitespace.
    if( !sTextCpy.empty() && sTextCpy.back() == ' ' )
      sTextCpy.resize( sTextCpy.size() - 1 );
  }

  return sTextCpy;
}

// -----------------------------------------------------------------------------

bool escrido::ReplaceIfMatch( std::string& sText_i, size_t& nPos_i, const char* szPattern_i, const char* szReplacement_i )
{
  size_t nPatternLen = strlen( szPattern_i );
  if( sText_i.compare( nPos_i, nPatternLen, szPattern_i ) == 0 )
  {
    sText_i.replace( nPos_i, strlen( szPattern_i ), szReplacement_i );
    size_t nReplaceLen = strlen( szReplacement_i );
    nPos_i += nReplaceLen;
    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the plain text of a string. (Strips off first and last
///             whitespaces.)
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sAll_o
///             The text or an empty string, if no text exists.
///
/// \return     true, if a text exists, false otherwise.
// *****************************************************************************

bool escrido::All( const std::string& sText_i, std::string& sAll_o )
{
  sAll_o.clear();

  // Search the text after the first white space.
  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    // Detect length of the text
    size_t nLen = sText_i.find_last_not_of( ' ' ) + 1 - nBegin;

    sAll_o = sText_i.substr( nBegin, nLen );
    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the first word (i.e. the first group of non-blank space
///             characters) of a string.
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sFirstWord_o
///             The first word (w/o any blank spaces) or an empty string, if no
///             first word exists.
///
/// \return     true, if a first word exists, false otherwise.
// *****************************************************************************

bool escrido::FirstWord( const std::string& sText_i, std::string& sFirstWord_o )
{
  sFirstWord_o.clear();

  // Search the first non-white space (i.e. beginning of the first word).
  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    // Detect length of the first word.
    size_t nLen = sText_i.find_first_of( ' ', nBegin );
    if( nLen != std::string::npos )
      nLen -= nBegin;

    sFirstWord_o = sText_i.substr( nBegin, nLen );
    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the first quote (i.e. the first group characters in
///             double quotation marks) of a string.
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sFirstQuote_o
///             The first quote (w/o quotation marks) or an empty string, if no
///             first quote exists.
///
/// \return     true, if a first quote exists, false otherwise.
// *****************************************************************************

bool escrido::FirstQuote( const std::string& sText_i, std::string& sFirstQuote_o )
{
  sFirstQuote_o.clear();

  // Search for the first non-whitespace character.
  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    if( sText_i[nBegin] == '"' )
    {
      ++nBegin;

      size_t nLen = sText_i.find_first_of( '"', nBegin );
      if( nLen != std::string::npos )
      {
        nLen -= nBegin;

        sFirstQuote_o = sText_i.substr( nBegin, nLen );
        return true;
      }
    }
  }

  return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the first non-empty line (i.e. the first group of
///             characters before the first line break '\r', '\n'
///             or '\r\n' ) of a string.
///             (Strips off first and last whitespaces.)
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sFirstLine_o
///             The first line or the whole text if no first line exists or an
///             empty string, if no text exists.
///
/// \return     true, if a first line exists, false otherwise.
// *****************************************************************************

bool escrido::FirstLine( const std::string& sText_i, std::string& sFirstLine_o )
{
  sFirstLine_o.clear();

  // Search the text after the first white space.
  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    // Search the first line break or alternative the end of the string.
    size_t nEnd = sText_i.find_first_of( "\r\n", nBegin );
    if( nEnd == std::string::npos )
      nEnd = sText_i.length();
    if( nEnd == nBegin )
      return false;
    else
      --nEnd;

    // Detect length of the text by skipping last white spaces
    size_t nLen = sText_i.find_last_not_of( ' ', nEnd ) + 1 - nBegin;

    sFirstLine_o = sText_i.substr( nBegin, nLen );
    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the group of characters that forms the text after the
///             first word and the following blank spaces of a string.
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sAllButFirstWord_o
///             The text after the first word (w/o leading blank spaces) or an
///             empty string, if no first word exists.
///
/// \return     true, if a first word exists, false otherwise.
// *****************************************************************************

bool escrido::AllButFirstWord( const std::string& sText_i, std::string& sAllButFirstWord_o )
{
  sAllButFirstWord_o.clear();

  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    nBegin = sText_i.find_first_of( ' ', nBegin );
    if( nBegin != std::string::npos )
    {
      nBegin = sText_i.find_first_not_of( ' ', nBegin );
      if( nBegin != std::string::npos )
        sAllButFirstWord_o = sText_i.substr( nBegin );
    }

    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the group of characters that forms the text after the
///             first quote and the following blank spaces of a string.
///
/// \param[in]  sText_i
///             The input text.
/// \param[out] sAllButFirstQuote_o
///             The text after the first quote (w/o quotation marks) or an
///             empty string, if no first quote exists.
///
/// \return     true, if a first quote exists, false otherwise.
// *****************************************************************************

bool escrido::AllButFirstQuote( const std::string& sText_i, std::string& sAllButFirstQuote_o )
{
  sAllButFirstQuote_o.clear();

  size_t nBegin = sText_i.find_first_not_of( ' ' );
  if( nBegin != std::string::npos )
  {
    if( sText_i[nBegin] == '"' )
    {
      nBegin = sText_i.find_first_of( '"', nBegin + 1 );
      if( nBegin != std::string::npos )
      {
        nBegin = sText_i.find_first_not_of( ' ', nBegin + 1 );
        if( nBegin != std::string::npos )
          sAllButFirstQuote_o = sText_i.substr( nBegin );

        return true;
      }
    }
    else
      sAllButFirstQuote_o = sText_i.substr( nBegin );
  }

  return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Renderst the first word (i.e. the first group of non-blank space
///             characters) of a string into an identifier that corresponds to
///             the regular expression [a-zA-Z]([a-zA-Z0-9_])+ .
///
/// \return     The first word as an identifier or an empty string, if no first
///             word exists or if the first word cannot be rendered into an
///             identifier.
// *****************************************************************************

std::string escrido::MakeIdentifier( const std::string& sText_i )
{
  // Get first word.
  std::string sFirstWord;
  if( !FirstWord( sText_i, sFirstWord ) )
    return std::string( "no-identifier" );

  // Result value.
  std::string sResult;

  // Render first word into an identifier.
  bool fFirstLetter = true;
  for( size_t l = 0; l < sFirstWord.size(); l++ )
  {
    if( ( sFirstWord[l] >= 'a' && sFirstWord[l] <= 'z' ) ||
        ( sFirstWord[l] >= 'A' && sFirstWord[l] <= 'Z' ) )
    {
      sResult += sFirstWord[l];
      fFirstLetter = false;
      continue;
    }

    if( !fFirstLetter &&
        ( ( sFirstWord[l] >= '1' && sFirstWord[l] <= '9' ) ||
          sFirstWord[l] == '0' ||
          sFirstWord[l] == '_' ) )
    {
      sResult += sFirstWord[l];
      continue;
    }
  }

  return sResult;
}

// -----------------------------------------------------------------------------

bool escrido::GetBlockTagType( const char* szTagName_i, tag_type& fTagType_o )
{
  for( unsigned int t = 0; t < nBlockTagTypeN; t++ )
    if( strcmp( szTagName_i, oaBlockTagTypeList[t].szName ) == 0 )
    {
      fTagType_o = oaBlockTagTypeList[t].fType;
      return true;
    }

  return false;
}

// -----------------------------------------------------------------------------

bool escrido::GetInlineTagType( const char* szTagName_i, tag_type& fTagType_o )
{
  for( unsigned int t = 0; t < nBlockTagTypeN; t++ )
    if( strcmp( szTagName_i, oaInlineTagTypeList[t].szName ) == 0 )
    {
      fTagType_o = oaInlineTagTypeList[t].fType;
      return true;
    }

  return false;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the camel case form of a given object's name.
///
/// \details    The whole term is returned in camel case (see
///             https://en.wikipedia.org/wiki/Camel_case ).
// *****************************************************************************

std::string escrido::GetCamelCase( const std::string& sName_i )
{
  std::string sResult;

  bool fStart = true;
  bool fBetweenWords = true;

  for( size_t c = 0; c < sName_i.length(); ++c )
  {
    if( sName_i[c] == ' ' || sName_i[c] == '\t' )
    {
      fBetweenWords = true;
      continue;
    }

    if( fBetweenWords )
    {
      fBetweenWords = false;

      if( fStart )
      {
        fStart = false;
        sResult += tolower( sName_i[c] );
      }
      else
        sResult += toupper( sName_i[c] );
    }
    else
      sResult += tolower( sName_i[c] );
  }

  return sResult;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the snake case form of a given object's name.
///
/// \details    The whole term is returned in snake case (see
///             https://en.wikipedia.org/wiki/Snake_case ).
///             This means removing of front and end whitespaces, lowercase
///             setting and replacing of internal whitespaces by <em>single</em>
///             underscore characters ('_').
// *****************************************************************************

std::string escrido::GetSnakeCase( const std::string& sName_i )
{
  std::string sResult;

  // Handle empty strings
  if( sName_i.empty() )
    return sResult;

  // Identify front and end whitespaces.
  size_t nStart, nEnd;
  {
    for( nStart = 0; nStart < sName_i.length(); ++nStart )
    {
      if( sName_i[nStart] != ' ' && sName_i[nStart] != '\t' )
        break;
    }

    for( nEnd = sName_i.length() - 1; nEnd >= nStart; --nEnd )
    {
      if( sName_i[nEnd] != ' ' && sName_i[nEnd] != '\t' )
        break;
    }
    ++nEnd;
  }

  // Replace inner whitespaces by underscores and lowercase letters
  for( size_t c = nStart; c < nEnd; ++c )
  {
    if( sName_i[c] == ' ' || sName_i[c] == '\t' )
    {
      sResult += '_';

      for( c = c + 1; c < nEnd; ++c )
        if( sName_i[c] != ' ' && sName_i[c] != '\t' )
        {
          --c;
          break;
        }
    }
    else
      sResult += tolower( sName_i[c] );
  }

  return sResult;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the capitalized form of a given object's name.
///
/// \details    Each word of the result is capitalized, e.g. "Data Type" from
///             "data type".
// *****************************************************************************

std::string escrido::GetCapForm( const std::string& sName_i )
{
  std::string sResult = sName_i;

  // Capitalize the string.
  unsigned int fState = 0;
  for( size_t i = 0; i< sResult.size(); i++ )
    switch( fState )
    {
      // State "before a word":
      case 0:
        if( ( sResult[i] != ' ' ) && ( sResult[i] != '\t' ) )
        {
          sResult[i] = toupper( sResult[i] );
          fState = 1;
        }
        continue;

      // State "within a word":
      case 1:
        if( ( sResult[i] == ' ' ) || ( sResult[i] == '\t' ) )
          fState = 0;
        else
          sResult[i] = tolower( sResult[i] );
        continue;
    }

  return sResult;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Returns the capitalized plural form of a given object's name.
///
/// \details    Each word of the result is capitalized. The function uses the
///             english default mechanisms of building the plural form, e.g.
///             "Data Types" from "data type", "Classes" from "class".
// *****************************************************************************

std::string escrido::GetCapPluralForm( const std::string& sName_i )
{
  std::string sResult = GetCapForm( sName_i );

  // Create plural form of string.
  size_t nLastChar = sResult.find_last_not_of( " \t" );
  if( nLastChar != std::string::npos )
  {
    // Crop away end whitespaces.
    sResult.resize( nLastChar + 1 );

    if( sResult[nLastChar] == 's' )
      sResult += "es";
    else
      sResult.push_back( 's' );
  }

  return sResult;
}
