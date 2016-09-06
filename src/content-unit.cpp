// -----------------------------------------------------------------------------
/// \file       content-unit.cpp
///
/// \brief      Implementation file for
///             documentation <em>content units</em>, i.e. general data blocks
///             of the Escrido documentation.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2015 trinckle 3D GmbH
// -----------------------------------------------------------------------------

#include "content-unit.h"

#include <string.h>
#include <iostream>         // cin, cout, cerr, endl
#include <fstream>          // std::ofstream


// -----------------------------------------------------------------------------

// STRUCT SWriteInfo

// -----------------------------------------------------------------------------

const escrido::SWriteInfo& escrido::SWriteInfo::operator++() const
{
  nIndent += 2;
  return *this;
}

// .............................................................................

const escrido::SWriteInfo& escrido::SWriteInfo::operator--() const
{
  nIndent -= 2;
  return *this;
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

std::string escrido::CContentChunk::GetPlainText() const
{
  if( fType == cont_chunk_type::NEW_LINE )
    return "\n";

  return sContent;
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

void escrido::CContentChunk::SetSkipFirstWhiteMode( skip_first_white fSkipFirstWhite_i )
{
  fSkipFirstWhite = fSkipFirstWhite_i;
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
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<p>";
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::END_PARAGRAPH:
      oOutStrm_i << "</p>" << std::endl;
      --oWriteInfo_i;
      break;

    case cont_chunk_type::START_TABLE:
      WriteHTMLTag( "<table>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      WriteHTMLTag( "<tr>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      WriteHTMLTag( "<td>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::END_TABLE:
      oOutStrm_i << std::endl;
      WriteHTMLTag( "</td>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTag( "</tr>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTag( "</table>", oOutStrm_i, --oWriteInfo_i );
      break;

    case cont_chunk_type::NEW_TABLE_CELL:
      oOutStrm_i << std::endl;
      WriteHTMLTag( "</td>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTag( "<td>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::NEW_TABLE_ROW:
      oOutStrm_i << std::endl;
      WriteHTMLTag( "</td>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTag( "</tr>", oOutStrm_i, --oWriteInfo_i );
      WriteHTMLTag( "<tr>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      WriteHTMLTag( "<td>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::START_UL:
      WriteHTMLTag( "<ul>", oOutStrm_i, oWriteInfo_i );
      ++oWriteInfo_i;
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li>";
      ++oWriteInfo_i;
      break;

    case cont_chunk_type::END_UL:
      oOutStrm_i << "</li>" << std::endl;
      --oWriteInfo_i;
      WriteHTMLTag( "</ul>", oOutStrm_i, --oWriteInfo_i );
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
        oOutStrm_i << sContent;

      if( fHasRef )
        oOutStrm_i << "</a>";
      break;
    }

    case cont_chunk_type::CODE:
      oOutStrm_i << "<span class=\"code\">";
      break;

    case cont_chunk_type::END_CODE:
      oOutStrm_i << "</span>";
      break;

    case cont_chunk_type::LINK:
    {
      const CContentChunk* pNextContentChunk = oWriteInfo_i.pTagBlock->GetNextContentChunk( this );
      if( pNextContentChunk == NULL )
        oOutStrm_i << "<a>";
      else
        oOutStrm_i << "<a href=\"" << pNextContentChunk->GetPlainText() << "\" target=\"_blank\">";
      break;
    }

    case cont_chunk_type::END_LINK:
      oOutStrm_i << "</a>";
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
  std::string sAllButFirstWord ;
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

void escrido::CContentChunk::WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  switch( fType )
  {
    case cont_chunk_type::HTML_TEXT:
      oOutStrm_i << ConvertHTMLToLaTeX( sContent );
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

      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\noindent\\parbox{\\textwidth}{%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, ++oWriteInfo_i ) << "\\tymin=" << 1.0 / ( nMaxColN + 1 ) << "\\textwidth%" << std::endl;
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
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\noindent\\parbox{\\textwidth}{%" << std::endl;
      WriteHTMLIndents( oOutStrm_i, ++oWriteInfo_i ) << "\\begin{itemize}" << std::endl;
      WriteHTMLIndents( oOutStrm_i, ++oWriteInfo_i ) << "\\item";
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
      oOutStrm_i << "\\hyperref[" << MakeIdentifier( this->GetPlainFirstWord() ) << "]{";

      std::string sText = this->GetPlainAllButFirstWord();
      if( !sText.empty() )
        oOutStrm_i << ConvertHTMLToLaTeX( sText );
      else
        oOutStrm_i << ConvertHTMLToLaTeX( sContent );

      oOutStrm_i << "}";
      break;
    }

    case cont_chunk_type::CODE:
      oOutStrm_i << "\\code{";
      break;

    case cont_chunk_type::END_CODE:
      oOutStrm_i << "}";
      break;

    case cont_chunk_type::LINK:
    {
      const CContentChunk* pNextContentChunk = oWriteInfo_i.pTagBlock->GetNextContentChunk( this );
      if( pNextContentChunk != NULL )
        oOutStrm_i << "\\url{";
      else
        oOutStrm_i << "\\url{" << pNextContentChunk->GetPlainText();
      break;
    }

    case cont_chunk_type::END_LINK:
      oOutStrm_i << "}";
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
      oOutStrm_i << ConvertHTMLToLaTeX( sFirstWord );
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
      oOutStrm_i << ConvertHTMLToLaTeX( sAllButFirstWord );
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

void escrido::CContentChunk::DebugOutput() const
{
  std::cout << "chunk type: " << (int) fType << ", content: " << sContent;
}

// -----------------------------------------------------------------------------

// CLASS CTagBlock

// -----------------------------------------------------------------------------

escrido::CTagBlock::CTagBlock():
  fType              ( tag_type::PARAGRAPH ),
  fAppNameTextMode   ( append_name_text_mode::OFF ),
  fVerbatimStartMode ( verbatim_start_mode::OFF ),
  fNewLine           ( true )
{}

// .............................................................................

escrido::CTagBlock::CTagBlock( tag_type fType_i ):
  fType              ( fType_i ),
  fAppNameTextMode   ( append_name_text_mode::OFF ),
  fVerbatimStartMode ( verbatim_start_mode::OFF ),
  fNewLine           ( true )
{
  // Switch on "delimitate title line" mode for specific tag block types:
  if( fType == tag_type::SECTION ||
      fType == tag_type::SUBSECTION ||
      fType == tag_type::IMAGE )
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
      fType == tag_type::SUBSECTION )
    faWriteMode.emplace_back( tag_block_write_mode::TITLE_LINE );
}

// .............................................................................

tag_type escrido::CTagBlock::GetTagType() const
{
  return this->fType;
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
    }
    faWriteMode.pop_back();
  }
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

  // Step 1: "append a name and a text" mode:
  switch( this->fAppNameTextMode )
  {
    case append_name_text_mode::INIT:
    {
      // Skip white spaces, switch to appending state.
      if( cChar_i != ' ' )
      {
        // Switch "append a name and a text" mode to "NAME".
        this->fAppNameTextMode = append_name_text_mode::NAME;

        // Append the character to the latest chunk that accepts the name and/or text.
        // Note: the chunk list cannot be empty at that point since otherwise
        // the "append a name and a text" mode would be OFF.
        oaChunkList.back().AppendChar( cChar_i );
      }
      return;
    }

    case append_name_text_mode::NAME:
    {
      if( cChar_i != ' ' )
      {
        // Append character to the latest chunk.
        // Note: see above.
        oaChunkList.back().AppendChar( cChar_i );
      }
      else
      {
        // Switch "append a name and a text" mode to "AFTER_NAME"
        this->fAppNameTextMode = append_name_text_mode::AFTER_NAME;
      }
      return;
    }

    case append_name_text_mode::AFTER_NAME:
    {
      if( cChar_i != ' ' )
        if( cChar_i == '"' )
        {
          // Append one blank space to the latest chunk.
          // Note: see above.
          oaChunkList.back().AppendChar( ' ' );

          // Switch "append a name and a text" mode to "TEXT".
          this->fAppNameTextMode = append_name_text_mode::TEXT;
          return;
        }
        else
        {
          // Switch "append a name and a text" mode to "OFF"
          this->fAppNameTextMode = append_name_text_mode::OFF;
          break;
        }
    }

    case append_name_text_mode::TEXT:
    {
      if( cChar_i == '"' )
      {
        // Switch "append a name and a text" mode to "OFF"
        this->fAppNameTextMode = append_name_text_mode::OFF;
      }
      else
      {
        // Append character to the latest chunk.
        // Note: see above.
        oaChunkList.back().AppendChar( cChar_i );
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
          fType ==  tag_type::OUTPUT )
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

  // Default behavior: add character to the latest text chunk.
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

    // Tag '@table':
    // -------------
    case tag_type::TABLE:
    {
      // Write mode dependend behavior:
      if( faWriteMode.empty() )
      {
        this->faWriteMode.emplace_back( tag_block_write_mode::TABLE );
        this->oaChunkList.emplace_back( cont_chunk_type::START_TABLE );
      }
      else
        switch( this->faWriteMode.back() )
        {
          case tag_block_write_mode::PLAIN_TEXT:
          {
            // Close text mode before adding a table:
            this->faWriteMode.pop_back();

            this->faWriteMode.emplace_back( tag_block_write_mode::TABLE );
            this->oaChunkList.emplace_back( cont_chunk_type::START_TABLE );
            break;
          }

          case tag_block_write_mode::PARAGRAPH:
          {
            // Close paragraph mode before adding a table:
            this->faWriteMode.pop_back();
            this->oaChunkList.emplace_back( cont_chunk_type::END_PARAGRAPH );

            this->faWriteMode.emplace_back( tag_block_write_mode::TABLE );
            this->oaChunkList.emplace_back( cont_chunk_type::START_TABLE );
            break;
          }

          // tag_block_write_mode::TITLE_LINE,
          // tag_block_write_mode::TABLE,
          // tag_block_write_mode::UL,
          default:
          {
            this->faWriteMode.emplace_back( tag_block_write_mode::TABLE );
            this->oaChunkList.emplace_back( cont_chunk_type::START_TABLE );
            break;
          }
        }

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
      this->fAppNameTextMode = append_name_text_mode::INIT;
      break;
    }

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
      this->oaChunkList.emplace_back( cont_chunk_type::CODE );
      this->oaChunkList.emplace_back( cont_chunk_type::PLAIN_TEXT );
      this->oaChunkList.back().SetSkipFirstWhiteMode( skip_first_white::INIT );
      break;
    }

    // Tag '@endcode':
    // -----------
    case tag_type::END_CODE:
    {
      // Eventually delete one last whitespace before end code.
      if( this->oaChunkList.back().GetType() ==  cont_chunk_type::PLAIN_TEXT )
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

      // Add LINK chunk and PLAIN TEXT chunk.
      this->oaChunkList.emplace_back( cont_chunk_type::LINK );
      this->oaChunkList.emplace_back( cont_chunk_type::PLAIN_TEXT );
      this->oaChunkList.back().SetSkipFirstWhiteMode( skip_first_white::INIT );
      break;
    }

    // Tag '@endlink':
    // -----------
    case tag_type::END_LINK:
    {
      // Eventually delete one last whitespace before end link.
      if( this->oaChunkList.back().GetType() ==  cont_chunk_type::PLAIN_TEXT )
      {
        std::string& sCodeText = this->oaChunkList.back().GetContent();
        if( sCodeText.back() == ' ' || sCodeText.back() == '\t' )
          sCodeText.pop_back();
      }

      this->oaChunkList.emplace_back( cont_chunk_type::END_LINK );
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
    case tag_type::PARAM:
    {
      oOutStrm_i << "<h3>";
      WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</h3>" << std::endl;
      WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      break;
    }

    case tag_type::SEE:
    {
      if( !this->oaChunkList.empty() )
      {
        oOutStrm_i << "<li>";
        size_t nRefIdx;
        if( oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->oaChunkList[0].GetPlainFirstWord() ), nRefIdx ) )
        {
          oOutStrm_i << "<a href=\"" + oWriteInfo_i.oRefTable.GetLink( nRefIdx ) + "\">";
          this->oaChunkList[0].WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</a>";
        }
        else
          this->oaChunkList[0].WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "</li>";
      }
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

    if( oaChunkList[c].WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

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

  // Loop through all text chunks until the something but the first word was written.
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
    case tag_type::PARAM:
    {
      // TODO
      /* HTML-Version
      oOutStrm_i << "<h3>";
      WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</h3>" << std::endl
                 << "<p>" << std::endl;
      WriteHTMLAllButFirstWord( oOutStrm_i, oWriteInfo_i );
      oOutStrm_i << "</p>" << std::endl;
      */
      break;
    }

    case tag_type::SEE:
    {
      // TODO
      /* HTML-Version
      if( !this->oaChunkList.empty() )
      {
        oOutStrm_i << "<li>";
        size_t nRefIdx;
        if( oWriteInfo_i.oRefTable.GetRefIdx( MakeIdentifier( this->oaChunkList[0].GetPlainFirstWord() ), nRefIdx ) )
        {
          oOutStrm_i << "<a href=\"" + oWriteInfo_i.oRefTable.GetLink( nRefIdx ) + "\">";
          this->oaChunkList[0].WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</a>";
        }
        else
          this->oaChunkList[0].WriteHTMLFirstWord( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "</li>";
      }
      */
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
  for( size_t c = 0; c < oaChunkList.size(); c++ )
  {
    // Break off on new line or new paragraph.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

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
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

    if( oaChunkList[c].WriteLaTeXAllButFirstWord( oOutStrm_i, oWriteInfo_i ) )
      break;
  }

  // Write remaining chunks up to the title line delimitor.
  for( c++; c < oaChunkList.size(); c++ )
  {
    // Break off in the title line delimitor is reached.
    if( oaChunkList[c].GetType() == cont_chunk_type::DELIM_TITLE_LINE )
      return;

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
    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
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
    oaChunkList[c].WriteLaTeX( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

void escrido::CTagBlock::DebugOutput() const
{
  std::cout << "block " << (unsigned long) this << " tag type: ";
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
    case tag_type::EXAMPLE:
      fTextChunkType = cont_chunk_type::PLAIN_TEXT;
      break;

    case tag_type::OUTPUT:
      fTextChunkType = cont_chunk_type::PLAIN_TEXT;
      break;

    // All other tag blocks accept HTML text:
    default:
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
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim tag block.

    oaBlockList.back().AppendNewLine();
  }
  else
  {
    // => Non-verbatim tag block.

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
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim tag block.

    oaBlockList.back().AppendChar( ' ' );
  }
  else
  {
    // => Non-verbatim tag block.

    if( fParseState[0] != parse_state::LINE_BREAK )
      oaBlockList.back().AppendChar( ' ' );
  }
}

// .............................................................................

void escrido::CContentUnit::AppendTab()
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim tag block.

    oaBlockList.back().AppendChar( ' ' );
    oaBlockList.back().AppendChar( ' ' );
  }
  else
  {
    // => Non-verbatim tag block.

    if( fParseState[0] != parse_state::LINE_BREAK )
    {
      oaBlockList.back().AppendChar( ' ' );
      oaBlockList.back().AppendChar( ' ' );
    }
  }
}

// .............................................................................

void escrido::CContentUnit::AppendTag( const char* szTagName_i )
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim tag block.

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
      if( GetInlineTagType( szTagName_i, fTagType ) )
      {
        // => The tag is an inline tag.

        oaBlockList.back().AppendInlineTag( fTagType );
      }
      else
        std::cerr << "unrecognized tag '@" << szTagName_i << "'" << std::endl;
  }
  else
  {
    // => Non-verbatim tag block.

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

void escrido::CContentUnit::AppendChar( const char cChar_i )
{
  // Check whether in a verbatim tag block types like EXAMPLE:
  if( oaBlockList.back().GetTagType() == tag_type::EXAMPLE ||
      oaBlockList.back().GetTagType() == tag_type::OUTPUT )
  {
    // => Verbatim tag block.
  }
  else
  {
    // => Non-verbatim tag block.

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
/*
void escrido::CContentUnit::WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Write brief block(s), if existing.
  if( HasTagBlock( tag_type::BRIEF ) )
  {
    oOutStrm_i << "<div class=\"brief\">" << std::endl
               << "  <p>" << std::endl;
    GetFirstTagBlock( tag_type::BRIEF )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "  </p>" << std::endl
               << "</div>" << std::endl;
  }

  // Write paragraph block(s), if existing.
  if( HasTagBlock( tag_type::PARAGRAPH ) )
  {
    oOutStrm_i << "<div class=\"default\">" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::PARAGRAPH )
      {
        oOutStrm_i << "  <p>" << std::endl;
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  <\p>" << std::endl;
      }
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write details block(s), if existing.
  if( HasTagBlock( tag_type::DETAILS ) )
  {
    oOutStrm_i << "<div class=\"details\">" << std::endl
               << "  <h2>Description</h2>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::DETAILS )
      {
        oOutStrm_i << "  <p>" << std::endl;
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  <\p>" << std::endl;
      }
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write internal, if existing and requested.
  if( oWriteInfo_i.fShowInternal && HasTagBlock( tag_type::INTERNAL ) )
  {
    oOutStrm_i << "<div class=\"internal\">" << std::endl
               << "  <h2>Internal Documentation</h2>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::INTERNAL )
      {
        oOutStrm_i << "  <p>" << std::endl;
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  <\p>" << std::endl;
      }
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write remark block(s), if existing.
  if( HasTagBlock( tag_type::REMARK ) )
  {
    oOutStrm_i << "<div class=\"remark\">" << std::endl
               << "  <h2>Remark</h2>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::REMARK )
      {
        oOutStrm_i << "  <p>" << std::endl;
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  <\p>" << std::endl;
      }
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write note block(s), if existing.
  if( HasTagBlock( tag_type::NOTE ) )
  {
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::NOTE )
      {
        oOutStrm_i << "<div class=\"note\">" << std::endl
                   << "  <h2>Please Note</h2>" << std::endl
                   << "  <p>" << std::endl;
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  </p>" << std::endl
                   << "</div>" << std::endl;
      }
  }

  // Write parameters,
  if( HasTagBlock( tag_type::PARAM ) )
  {
    oOutStrm_i << "<div class=\"parameters\">" << std::endl
               << "  <h2>Parameters</h2>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::PARAM )
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write return, if existing. (Only first appearance.)
  if( HasTagBlock( tag_type::RETURN ) )
  {
    oOutStrm_i << "<div class=\"return\">" << std::endl
               << "  <h2>Return Value</h2>" << std::endl
               << "  <p>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::RETURN )
      {
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        break;
      }
    oOutStrm_i << "  </p>" << std::endl
               << "</div>" << std::endl;
  }

  // Write examples, if existing.
  if( HasTagBlock( tag_type::EXAMPLE ) )
  {
    oOutStrm_i << "<div class=\"examples\">" << std::endl
               << "  <h2>Example</h2>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::EXAMPLE )
      {
        oOutStrm_i << "  <pre class=\"example\">";
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
        oOutStrm_i << "  </pre>" << std::endl;
      }
    oOutStrm_i << "</div>" << std::endl;
  }

  // Write references ("see"), if existing.
  if( HasTagBlock( tag_type::SEE ) )
  {
    oOutStrm_i << "<div class=\"see\">" << std::endl
               << "  <h2>See also</h2>" << std::endl
               << "  <ul>" << std::endl;
    for( size_t b = 0; b <  this->oaBlockList.size(); b++ )
      if( this->oaBlockList[b].GetTagType() == tag_type::SEE )
        this->oaBlockList[b].WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "  </ul>" << std::endl
               << "</div>" << std::endl;
  }
}*/

// .............................................................................

// *****************************************************************************
/// \brief      Writes the PARAGRAPH, the SECTION, the DETAILS and embedded
///             EXAMPLE, IMAGE, NOTES, OUTPUT and REMARK tag blocks in a
///             standardized way.
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
            WriteHTMLTag( "<div class=\"details\">", oOutStrm_i, oWriteInfo_i );
            ++oWriteInfo_i;
            WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>Details</h2>" << std::endl;
            fInDetails = true;
          }
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          break;

        case tag_type::SECTION:
        {
          // Eventually leave "details" div box.
          if( fInDetails )
          {
            WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
            fInDetails = false;
          }

          // Surrounding "<div>".
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<div id=\""
                                                       << oaBlockList[t].GetPlainFirstWord()
                                                       << "\" class=\"section\">" << std::endl;
          ++oWriteInfo_i;

          // Title line.
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</h2>" << std::endl;

          // Content and terminating "</div>".
          oaBlockList[t].WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::SUBSECTION:
        {
          // Surrounding "<div>".
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<div id=\""
                                                       << oaBlockList[t].GetPlainFirstWord()
                                                       << "\" class=\"subsection\">" << std::endl;
          ++oWriteInfo_i;

          // Title line.
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h3>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</h3>" << std::endl;

          // Content and terminating "</div>".
          oaBlockList[t].WriteHTMLAllButTitleLine( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::EXAMPLE:
        {
          WriteHTMLTag( "<div class=\"examples\">", oOutStrm_i, oWriteInfo_i );
          ++oWriteInfo_i;
          WriteHTMLTag( "<h4>Example</h4>", oOutStrm_i, oWriteInfo_i );
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<pre class=\"example\">";
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</pre>" << std::endl;
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::IMAGE:
        {
          WriteHTMLTag( "<figure class=\"image\">", oOutStrm_i, oWriteInfo_i );
          ++oWriteInfo_i;
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<img src=\"" << oaBlockList[t].GetPlainFirstWord() << "\">" << std::endl;
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<figcaption>";
          oaBlockList[t].WriteHTMLTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</figcaption>" << std::endl;
          WriteHTMLTag( "</figure>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::NOTE:
        {
          WriteHTMLTag( "<div class=\"note\">", oOutStrm_i, oWriteInfo_i );
          ++oWriteInfo_i;
          WriteHTMLTag( "<h4>Note</h4>", oOutStrm_i, oWriteInfo_i );
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::OUTPUT:
        {
          WriteHTMLTag( "<div class=\"output\">", oOutStrm_i, oWriteInfo_i );
          ++oWriteInfo_i;
          WriteHTMLTag( "<h4>Output</h4>", oOutStrm_i, oWriteInfo_i );
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<pre class=\"output\">";
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "</pre>" << std::endl;
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }

        case tag_type::REMARK:
        {
          WriteHTMLTag( "<div class=\"remark\">", oOutStrm_i, oWriteInfo_i );
          ++oWriteInfo_i;
          WriteHTMLTag( "<h4>Remark</h4>", oOutStrm_i, oWriteInfo_i );
          oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
          WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
          break;
        }
      }
    }

    // Eventually leave last "details" div box.
    if( fInDetails )
    {
      WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
      fInDetails = false;
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the PARAM tag blocks in a standardized way.
// *****************************************************************************

void escrido::CContentUnit::WriteHTMLParam( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Loop over all text blocks.
  for( size_t t = 0; t < oaBlockList.size(); t++ )
    if( oaBlockList[t].GetTagType() == tag_type::PARAM )
      oaBlockList[t].WriteHTML( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the PARAGRAPH, the SECTION, the DETAILS and embedded
///             EXAMPLE, IMAGE, NOTES, OUTPUT and REMARK tag blocks in a
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
            oOutStrm_i << "\\subsection{Details}%" << std::endl << std::endl;
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
          oOutStrm_i << "\\subsection{";
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
          oOutStrm_i << "\\subsubsection{";
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
          oOutStrm_i << "\\verbatimtitle{Example}" << std::endl
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
          oaBlockList[t].WriteLaTeXTitleLineButFirstWord( oOutStrm_i, oWriteInfo_i );
          oOutStrm_i << "}" << std::endl
                     << "  \\end{center}" << std::endl
                     << "\\end{minipage}" << std::endl << std::endl;
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
          oOutStrm_i << "\\verbatimtitle{Output}" << std::endl
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

void escrido::WriteHTMLTag( const char* szTagText_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i )
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

      case '´':
        sReturn += "'";
        break;

      case '°':
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

std::string escrido::ConvertHTMLToLaTeX( const std::string& sText_i )
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

  // Search the first white space after the first word.
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

// // -----------------------------------------------------------------------------
//
// // *****************************************************************************
// /// \brief      Returns the first word (i.e. the first group of non-blank space
// ///             characters) of a string.
// ///
// /// \return     The first word (w/o any blank spaces) or an empty string, if no
// ///             first word exists.
// // *****************************************************************************
//
// std::string escrido::FirstWord( const std::string& sText_i )
// {
//   // Search the first white space after the first word.
//   size_t nBegin = sText_i.find_first_not_of( ' ' );
//   if( nBegin != std::string::npos )
//   {
//     // Detect length of the first word.
//     size_t nLen = sText_i.find_first_of( ' ', nBegin );
//     if( nLen != std::string::npos )
//       nLen -= nBegin;
//
//     return sText_i.substr( nBegin, nLen );
//   }
//   else
//     return std::string();
// }
//
// // -----------------------------------------------------------------------------
//
// // *****************************************************************************
// /// \brief      Returns the group of characters that form the text after the
// ///             first word of a string
// ///
// /// \return     The string after the first word and proceeding blank spaces or
// ///             an empty string if no such string exists.
// // *****************************************************************************
//
// std::string escrido::AllButFirstWord( const std::string& sText_i )
// {
//   size_t nBegin = sText_i.find_first_not_of( ' ' );
//   if( nBegin != std::string::npos )
//   {
//     nBegin = sText_i.find_first_of( ' ', nBegin );
//     if( nBegin != std::string::npos )
//     {
//       nBegin = sText_i.find_first_not_of( ' ', nBegin );
//       if( nBegin != std::string::npos )
//         return sText_i.substr( nBegin );
//     }
//   }
//   return std::string();
// }

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
