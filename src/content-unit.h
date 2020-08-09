#ifndef CONTENT_UNIT_ALLREADY_READ_IN
#define CONTENT_UNIT_ALLREADY_READ_IN

// -----------------------------------------------------------------------------
/// \file       content-unit.h
///
/// \brief      Declaration file for
///             documentation <em>content units</em>, i.e. general data blocks
///             of the Escrido documentation.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2016 trinckle 3D GmbH
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

// INCLUSIONS

// -----------------------------------------------------------------------------

#include <string>
#include <vector>

#include "reftable.h"

// -----------------------------------------------------------------------------

// CLASSES OVERVIEW

// -----------------------------------------------------------------------------

// *********************
// *                   *
// *    SWriteInfo     *
// *                   *
// *********************

// *********************
// *                   *
// *   CContentChunk   *
// *                   *
// *********************

// *********************
// *                   *
// *     CTagBlock     *
// *                   *
// *********************

// *********************
// *                   *
// *   CContentUnit    *
// *                   *
// *********************

namespace escrido
{
  struct SWriteInfo;
  class CContentChunk;
  class CTagBlock;
  class CContentUnit;
}

// -----------------------------------------------------------------------------

// TYPES, CONSTANTS AND ENUMERATIONS

// -----------------------------------------------------------------------------

// Content chunk types:
enum class cont_chunk_type
{
  UNDEFINED,
  HTML_TEXT,
  PLAIN_TEXT,
  DELIM_TITLE_LINE,
  NEW_LINE,
  START_PARAGRAPH,
  END_PARAGRAPH,
  START_TABLE,
  END_TABLE,
  NEW_TABLE_CELL,
  NEW_TABLE_ROW,
  START_UL,
  END_UL,
  UL_ITEM,
  REF,
  START_CODE,
  END_CODE,
  LINK,
  START_VERBATIM,
  END_VERBATIM
};

// Tag types:
enum class tag_type
{
  ATTRIBUTE,
  AUTHOR,
  BRIEF,
  COPYRIGHT,
  CODE,
  END_CODE,
  DATE,
  DETAILS,
  EXAMPLE,
  FEATURE,
  IMAGE,
  INGROUP,
  INTERNAL,
  LINE_BREAK,
  NAMESPACE,
  NOTE,
  OUTPUT,
  ORDER,
  PARAGRAPH,
  PARAM,
  REF,
  LINK,
  REMARK,
  RETURN,
  SEE,
  SECTION,
  SIGNATURE,
  SUBSECTION,
  SUBSUBSECTION,
  TABLE,
  END_TABLE,
  UL_ITEM,
  VERSION,
  VERBATIM,
  END_VERBATIM
};

// Write modes inside a block tag:
enum class tag_block_write_mode : unsigned char
{
  TITLE_LINE,
  PLAIN_TEXT,
  PARAGRAPH,
  TABLE,
  UL,
  VERBATIM
};

// Flag states for controlled appending of a one-word name followed by an
// optional text in quotation marks to a content chunk
// (e.g. @ref <name> ["(text)"]):
enum class append_name_text_mode
{
  OFF,
  INIT,
  NAME,
  AFTER_NAME,
  TEXT
};

// Flag state controlling the start of a verbatim tag block like EXAMPLE:
// if the tag is followed by a direct line break, this should not be
// counted since it is supposed to be only for beauty reasons:
enum class verbatim_start_mode
{
  OFF,
  INIT
};

// Flag for skipping the first whitespace of a content chunk, e.g. plain text
// chunk in CODE or LINK tags.
enum class skip_first_white
{
  INIT,
  OFF
};

// Parser state types:
enum class parse_state : unsigned char
{
  DEFAULT,
  LINE_BREAK,
  NEW_LINE
};

// Content unit type:
enum class cont_unit_type
{
  UNSET,
  SINGLE_LINE,
  MULTI_LINE
};

namespace escrido
{
  // Struct for definine tag type strings.
  struct STagType
  {
    tag_type fType;
    char szName[32];
  };

  // Block tag types:
  const unsigned int nBlockTagTypeN = 25;
  const escrido::STagType oaBlockTagTypeList[nBlockTagTypeN] = {
    { tag_type::ATTRIBUTE,     "attribute" },
    { tag_type::AUTHOR,        "author" },
    { tag_type::BRIEF,         "brief" },
    { tag_type::COPYRIGHT,     "copyright" },
    { tag_type::DATE,          "date" },
    { tag_type::DETAILS,       "details"},
    { tag_type::EXAMPLE,       "example" },
    { tag_type::FEATURE,       "feature" },
    { tag_type::IMAGE,         "image" },
    { tag_type::INGROUP,       "ingroup" },
    { tag_type::INTERNAL,      "internal" },
    { tag_type::NAMESPACE,     "namespace" },
    { tag_type::NOTE,          "note" },
    { tag_type::ORDER,         "order" },
    { tag_type::OUTPUT,        "output" },
    { tag_type::PARAGRAPH,     "par" },
    { tag_type::PARAM,         "param"},
    { tag_type::REMARK,        "remark" },
    { tag_type::RETURN,        "return" },
    { tag_type::SEE,           "see" },
    { tag_type::SECTION,       "section" },
    { tag_type::SIGNATURE,     "signature" },
    { tag_type::SUBSECTION,    "subsection" },
    { tag_type::SUBSUBSECTION, "subsubsection" },
    { tag_type::VERSION,       "version" } };

  // Inline tag types:
  const unsigned int nInlineTagTypeN = 10;
  const escrido::STagType oaInlineTagTypeList[nInlineTagTypeN] = {
    { tag_type::CODE,         "code" },
    { tag_type::END_CODE,     "endcode" },
    { tag_type::LINK,         "link" },
    { tag_type::LINE_BREAK,   "lb" },
    { tag_type::REF,          "ref" },
    { tag_type::TABLE,        "table" },
    { tag_type::END_TABLE,    "endtable" },
    { tag_type::VERBATIM,     "verbatim" },
    { tag_type::END_VERBATIM, "endverbatim" } };
}

// -----------------------------------------------------------------------------

// FUNCTIONS OVERVIEW

// -----------------------------------------------------------------------------

namespace escrido
{
  std::ostream& WriteHTMLIndents( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i );
  void          WriteHTMLTagLine( const char* szTagText_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i );
  std::string   HTMLEscape( const std::string& sText_i );
  std::string   LaTeXEscape( const std::string& sText_i );
  std::string   ConvertHTMLToLaTeX( const std::string& sText_i );
  std::string   RemoveHTMLTags( const std::string& sText_i );
  bool          ReplaceIfMatch( std::string& sText_i, size_t& nPos_i, const char* szPattern_i, const char* szReplacement_i );
  bool          All( const std::string& sText_i, std::string& sAll_o );
  bool          FirstWord( const std::string& sText_i, std::string& sFirstWord_o );
  bool          FirstQuote( const std::string& sText_i, std::string& sFirstQuote_o );
  bool          FirstLine( const std::string& sText_i, std::string& sFirstLine_o );
  bool          AllButFirstWord( const std::string& sText_i, std::string& sAllButFirstWord_o );
  bool          AllButFirstQuote( const std::string& sText_i, std::string& sAllButFirstQuote_o );
  std::string   MakeIdentifier( const std::string& sWord_i );
  bool          GetBlockTagType( const char* szTagName_i, tag_type& fTagType_o );
  bool          GetInlineTagType( const char* szTagName_i, tag_type& fTagType_o );
  std::string   GetCapForm( const std::string& sName_i );
  std::string   GetCapPluralForm( const std::string& sName_i );
}

// -----------------------------------------------------------------------------

// STRUCT SWriteInfo

// -----------------------------------------------------------------------------

struct escrido::SWriteInfo
{
  CRefTable                  oRefTable;
  bool                       fShowInternal;
  mutable const CTagBlock*   pTagBlock;
  mutable signed int         nIndent;

  const SWriteInfo& operator++() const;   // Prefix:  ++c
  const SWriteInfo operator++(int) const; // Postfix: c++
  const SWriteInfo& operator--() const;   // Prefix: --c
  const SWriteInfo operator--(int) const; // Postfix: c--
};

// -----------------------------------------------------------------------------

// CLASS CContentChunk

// -----------------------------------------------------------------------------

class escrido::CContentChunk
{
  private:

    cont_chunk_type fType;                  ///< Content chunk type.
    std::string sContent;                   ///< General type content.

    skip_first_white fSkipFirstWhite;       ///< Flag for skipping the first whitespace.

  public:

    // Constructor:
    CContentChunk();
    CContentChunk( const cont_chunk_type fType_i );

    // Member administration:
    cont_chunk_type GetType() const;
    std::string& GetContent();
    void SetSkipFirstWhiteMode( skip_first_white fSkipFirstWhite_i );

    // Access of formated text content:
    std::string GetPlainText() const;
    std::string GetPlainFirstWord() const;
    std::string GetPlainFirstWordOrQuote() const;
    std::string GetPlainAllButFirstWord() const;
    std::string GetPlainFirstLine() const;

    // Access of unformated text content
    std::string GetPlainContent() const;

    // Append parsing content:
    void AppendChar( const char cChar_i );

    // Output method:
    void WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteHTMLAllButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    void WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteLaTeXFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteLaTeXAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    bool WriteLaTeXAllButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;
};

// -----------------------------------------------------------------------------

// CLASS CTagBlock

// -----------------------------------------------------------------------------

class escrido::CTagBlock
{
  private:

    // Data:
    tag_type fType;                                 ///< Type of the tag block, e.g. "details" or "param".
    std::vector <CContentChunk> oaChunkList;        ///< List of chunks the block content consists of.

    // Parsing states and flags:
    std::vector <tag_block_write_mode> faWriteMode; ///< Internal write mode stack (e.g. text mode, table mode, unordered list mode).
    append_name_text_mode fAppNameTextMode;         ///< Parsing state flag for appending a name and and optional text to the latest content chunk.
    verbatim_start_mode   fVerbatimStartMode;       ///< Parsing state flag for verbatim tag block types.
    bool fNewLine;                                  ///< Parsing state flag stating whether there has been a newline recently.

  public:

    // Constructor, destructor:
    CTagBlock();
    CTagBlock( tag_type fType_i );

    // Member administration:
    bool Empty() const;
    void SetTagType( tag_type fType_i );
    tag_type GetTagType() const;
    tag_block_write_mode GetWriteMode() const;
    void CloseWrite();

    // Access of formated text content:
    std::string GetPlainText() const;
    std::string GetPlainFirstWord() const;
    std::string GetPlainFirstWordOrQuote() const;
    std::string GetPlainTitleLine() const;
    std::string GetPlainTitleLineButFirstWord() const;

    // Access of unformated text content
    std::string GetPlainContent() const;
    std::string GetPlainContentButFirstWord() const;

    // Content chunk navigation:
    const CContentChunk* GetNextContentChunk( const CContentChunk* pContentChunk ) const;

    // Append parsing content:
    void AppendChar( const char cChar_i );
    void AppendInlineTag( tag_type fTagType_i );
    void AppendNewLine();
    void AppendDoubleNewLine();

    // Output method:
    void WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTitleLineButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTitleLineButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLAllButTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    void WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTitleLineButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTitleLineButFirstWordOrQuote( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXAllButFirstWord( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXAllButTitleLine( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    void AppendCharDefault( const char cChar_i );
    void EscapeFromWriteModes( const std::vector<tag_block_write_mode>& oaWriteModes_i );

  friend class escrido::CContentUnit;
};

// -----------------------------------------------------------------------------

// CLASS CContentUnit

// -----------------------------------------------------------------------------

class escrido::CContentUnit
{
  private:

    cont_unit_type fContUnitType;                 ///< Type of the content unit (single or multi line).
    parse_state fParseState[3];                   ///< Parsing state (three look back).
    std::vector <CTagBlock> oaBlockList;          ///< List of blocks of the unit.

  public:

    // Constructor, destructor:
    CContentUnit();

    // Content managment:
    bool Empty() const;
    void ResetParseState( cont_unit_type fContUnitType_i );
    void ResetContent();
    void CloseWrite();

    // Append parsing content:
    void AppendContentUnit( const CContentUnit& oContUnit_i );
    void AppendLineBreak();
    void AppendBlank();
    void AppendTab();
    void AppendChar( const char cChar_i );
    void AppendTag( const char* szTagName_i );

    // Methods for accessing tag blocks:
    size_t           GetTagBlockN() const;
    const CTagBlock& GetTagBlock( size_t nTagBlockIdx_i ) const;
    bool             HasTagBlock( tag_type fTagType_i ) const;
    size_t           GetTagBlockN( tag_type fType_i ) const;
    const CTagBlock* GetFirstTagBlock( tag_type fTagType_i ) const;
    const CTagBlock* GetNextTagBlock( const CTagBlock* pLast_i, tag_type fTagType_i ) const;

    // Output method:
//     void WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTagBlock( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTagBlockList( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    void WriteLaTeXParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTagBlock( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTagBlockList( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    // Parsing state setting:
    void SetParseState( parse_state fParseState_i );
};




#endif /* CONTENT_UNIT_ALLREADY_READ_IN */
