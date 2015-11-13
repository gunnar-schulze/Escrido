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
/// \copyright  2015 trinckle 3D GmbH
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
// *  SWriteHTMLInfo   *
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
  struct SWriteHTMLInfo;
  class CContentChunk;
  class CTagBlock;
  class CContentUnit;
}

// -----------------------------------------------------------------------------

// GLOBAL CONSTANTS

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
  CODE,
  END_CODE
};

// Tag types:
enum class tag_type
{
  AUTHOR,
  BRIEF,
  COPYRIGHT,
  CODE,
  END_CODE,
  DATE,
  DETAILS,
  EXAMPLE,
  IMAGE,
  INGROUP,
  INTERNAL,
  LINE_BREAK,
  NAMESPACE,
  NOTE,
  OUTPUT,
  PARAGRAPH,
  PARAM,
  REF,
  LINK,
  REMARK,
  RETURN,
  SEE,
  SECTION,
  SUBSECTION,
  TABLE,
  END_TABLE,
  UL_ITEM,
  VERSION
};

// Block tag write modes:
enum class tag_block_write_mode
{
  TITLE_LINE,
  PLAIN_TEXT,
  PARAGRAPH,
  TABLE,
  UL
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
  const unsigned int nBlockTagTypeN = 21;
  const escrido::STagType oaBlockTagTypeList[nBlockTagTypeN] = {
    { tag_type::AUTHOR,     "author" },
    { tag_type::BRIEF,      "brief" },
    { tag_type::COPYRIGHT,  "copyright" },
    { tag_type::DATE,       "date" },
    { tag_type::DETAILS,    "details"},
    { tag_type::EXAMPLE,    "example" },
    { tag_type::IMAGE,      "image" },
    { tag_type::INGROUP,    "ingroup" },
    { tag_type::INTERNAL,   "internal" },
    { tag_type::NAMESPACE,  "namespace" },
    { tag_type::NOTE,       "note" },
    { tag_type::OUTPUT,     "output" },
    { tag_type::PARAGRAPH,  "par" },
    { tag_type::PARAM,      "param"},
    { tag_type::REMARK,     "remark" },
    { tag_type::RETURN,     "return" },
    { tag_type::SEE,        "see" },
    { tag_type::SECTION,    "section" },
    { tag_type::SUBSECTION, "subsection" },
    { tag_type::VERSION,    "version" } };

  // Inline tag types:
  const unsigned int nInlineTagTypeN = 6;
  const escrido::STagType oaInlineTagTypeList[nInlineTagTypeN] = {
    { tag_type::LINE_BREAK, "lb" },
    { tag_type::REF,        "ref" },
    { tag_type::TABLE,      "table" },
    { tag_type::END_TABLE,  "endtable" },
    { tag_type::CODE,       "code" },
    { tag_type::END_CODE,   "endcode" } };
}

// -----------------------------------------------------------------------------

// FUNCTIONS OVERVIEW

// -----------------------------------------------------------------------------

namespace escrido
{
  std::ostream& WriteHTMLIndents( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i );
  void          WriteHTMLTag( const char* szTagText_i, std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i );
  std::string   HTMLEscape( const std::string& sText_i );
  bool          FirstWord( const std::string& sText_i, std::string& sFirstWord_o );
  bool          AllButFirstWord( const std::string& sText_i, std::string& sAllButFirstWord_o );
  std::string   MakeIdentifier( const std::string& sWord_i );
  bool          GetBlockTagType( const char* szTagName_i, tag_type& fTagType_o );
  bool          GetInlineTagType( const char* szTagName_i, tag_type& fTagType_o );
}

// -----------------------------------------------------------------------------

// STRUCT SWriteHTMLInfo

// -----------------------------------------------------------------------------

struct escrido::SWriteHTMLInfo
{
  CRefTable          oRefTable;
  bool               fShowInternal;
  mutable signed int nIndent;     // TODO Change to unsigned int

  const SWriteHTMLInfo& operator++() const;   // Prefix: ++c
  const SWriteHTMLInfo& operator--() const;   // Prefix: --c
};

// -----------------------------------------------------------------------------

// CLASS CContentChunk

// -----------------------------------------------------------------------------

class escrido::CContentChunk
{
  private:

    cont_chunk_type fType;                  ///< Content chunk type.
    std::string sContent;                   ///< General type content.

  public:

    // Constructor:
    CContentChunk();
    CContentChunk( const cont_chunk_type fType_i );

    // Member access:
    cont_chunk_type GetType() const;
    std::string& GetContent();
    std::string GetPlainText() const;
    std::string GetPlainFirstWord() const;
    std::string GetPlainAllButFirstWord() const;

    // Append parsing content:
    void AppendChar( const char cChar_i );

    // Output method:
    void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    bool WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    bool WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;

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

    // Content managment:
    bool Empty() const;
    void SetTagType( tag_type fType_i );
    tag_type GetTagType() const;
    std::string GetPlainText() const;
    std::string GetPlainFirstWord() const;
    void CloseWrite();

    // Append parsing content:
    void AppendChar( const char cChar_i );
    void AppendInlineTag( tag_type fTagType_i );
    void AppendNewLine();
    void AppendDoubleNewLine();

    // Output method:
    void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLFirstWord( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLTitleLine( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLTitleLineButFirstWord( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLAllButFirstWord( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLAllButTitleLine( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    void AppendCharDefault( const char cChar_i );

  friend class escrido::CContentUnit;
};

// -----------------------------------------------------------------------------

// CLASS CContentUnit

// -----------------------------------------------------------------------------

class escrido::CContentUnit
{
  private:

    cont_unit_type fContUnitType;                 ///< Type of the content unit (single or mult line).
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
    void AppendTag( const char* szTagName_i );
    void AppendChar( const char cChar_i );

    // Methods for accessing tag blocks:
    size_t           GetTagBlockN() const;
    const CTagBlock& GetTagBlock( size_t nTagBlockIdx_i ) const;
    bool             HasTagBlock( tag_type fTagType_i ) const;
    size_t           GetTagBlockN( tag_type fType_i ) const;
    const CTagBlock* GetFirstTagBlock( tag_type fTagType_i ) const;
    const CTagBlock* GetNextTagBlock( const CTagBlock* pLast_i, tag_type fTagType_i ) const;

    // Output method:
    void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    // Parsing state setting:
    void SetParseState( parse_state fParseState_i );
};




#endif /* CONTENT_UNIT_ALLREADY_READ_IN */