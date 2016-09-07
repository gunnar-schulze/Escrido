#ifndef ESCRIDO_DOC_ALLREADY_READ_IN
#define ESCRIDO_DOC_ALLREADY_READ_IN

// -----------------------------------------------------------------------------
/// \file       escrido-doc.h
///
/// \brief      Module header for
///             the Escrido documentation page management.
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
#include "content-unit.h"

// -----------------------------------------------------------------------------

// CLASSES OVERVIEW

// -----------------------------------------------------------------------------

// *********************
// *                   *
// *      CGroup       *
// *                   *
// *********************

// *********************
// *                   *
// *      CParam       *
// *                   *
// *********************

// *********************
// *                   *
// *    CParamList     *
// *                   *
// *********************

// *********************     *********************
// *                   *     *                   *
// *     CDocPage      *--+--*   CPageMainpage   *
// *                   *  |  *                   *
// *********************  |  *********************
//                        |
//                        |  *********************
//                        |  *                   *
//                        +--*     CRefPage      *
//                           *                   *
//                           *********************

// *********************
// *                   *
// *  CDocumentation   *
// *                   *
// *********************

/// Namespace of the Escrido "multi-language documentation generator" project.
namespace escrido
{
  class CGroup;
  class CParam;
  class CParamList;
  class CDocPage;            // Basic documentation page. Parent class of all pages.
  class CPageMainpage;       // Documentation main page class.
  class CRefPage;            // Documentation page for the reference to an object (function, data type etc.).
  class CDocumentation;      // A complete documentation.
}

// -----------------------------------------------------------------------------

// GLOBAL CONSTANTS

// -----------------------------------------------------------------------------

namespace escrido
{
  /// Output document types.
  enum output_doc : unsigned char
  {
    HTML,
    LATEX
  };

  // Parser state types:
  enum headline_parse_state : unsigned char
  {
    START,
    PAGETYPE,
    PAGETYPE_DQUOTED,
    POST_PAGETYPE,
    IDENTIFIER,
    POST_IDENT,
    TITLE
  };
}

// -----------------------------------------------------------------------------

// FUNCTIONS OVERVIEW

// -----------------------------------------------------------------------------

namespace escrido
{
  // General reading and writing:
  bool ReadTemp( const std::string& sTemplateDir_i,
                 const std::string& sFileName_i,
                 const std::string& sFallbackFileName_i,
                 std::string& sTemplateData_o );
  void WriteOutput( const std::string& sFileName_i,
                    const std::string& sTemplateData_i );

  // String replacement:
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const std::string& sReplacement_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocPage& oPage_i,
                           output_doc fOutputDoc_i,
                           tag_type fTagType_i,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );

  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocPage& oPage_i,
                           void (CDocPage::*WriteMethodHTML)( std::ostream&, const SWriteInfo& ) const,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocumentation& oDocumentation_i,
                           void (CDocumentation::*WriteMethodHTML)( std::ostream&, const SWriteInfo& ) const,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );

  void AdjustReplaceIndent( size_t nReplPos_i,
                            std::string& sTemplateData_io,
                            const SWriteInfo& oWriteInfo_io );

  std::string GetCapForm( const std::string& sName_i );
  std::string GetCapPluralForm( const std::string& sName_i );
}





// -----------------------------------------------------------------------------

// CLASS CGroup

// -----------------------------------------------------------------------------

class escrido::CGroup
{
  public:

    std::string sGroupName;
    std::vector <size_t> naDocPageIdxList;

  public:

    // Constructor:
    CGroup( const std::string& sGroupName_i );
};

// -----------------------------------------------------------------------------

// CLASS CParam

// -----------------------------------------------------------------------------

class escrido::CParam
{
  public:

    std::string sIdent;
    bool        fHasDefault;
    std::string sDefault;

  public:

    // Constructor:
    CParam( const char* szIdent_i );
    CParam( const char* szIdent_i, const char* szDefault_i );
};

// -----------------------------------------------------------------------------

// CLASS CParamList

// -----------------------------------------------------------------------------

class escrido::CParamList
{
  public:

    std::vector <CParam> oaParamList;

  public:

    void AppendParam( const CParam& oParam_i );

  friend class CPageFunc;
};

// -----------------------------------------------------------------------------

// CLASS CDocPage

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      A basic, non-defined documentation page. This is used as
///             parent class of all specializd documentation pages.
// *****************************************************************************

class escrido::CDocPage
{
  protected:

    // Page type, identifier and title:
    std::string sPageTypeLit;      ///< Literary page type ("mainpage", "page", "data type" etc.)
    std::string sPageTypeID;       ///< Page type identifier ("data_type" etc.)
    std::string sIdent;            ///< The page identifier (as given in the headline).
    std::string sTitle;            ///< The page title (as given in the headline).

    // Head line parsing state:
    headline_parse_state fState;   ///< State for parsing headline.

    // Content related:
    CContentUnit oContUnit;        ///< The documentation page's content unit.

  public:

    // Constructor:
    CDocPage();
    CDocPage( const char* szPageTypeLit_i,
              const char* szPageTypeID_i,
              const char* szIdent_i,
              headline_parse_state fState_i );

    // Appending of content (while parsing):
    void AppendContentUnit( const CContentUnit& oContUnit_i );

    // Methods for accessing selected content:
    virtual void AppendHeadlineChar( const char cIdentChar_i );
    const std::string GetPageTypeLit() const;
    const std::string GetPageTypeID() const;
    const std::string& GetIdent() const;
    const std::string& GetTitle() const;
    CContentUnit&      GetContentUnit();
    const std::string  GetBrief() const;
    const std::string  GetNamespace() const;
    const std::string  GetGroupName() const;

    // Output method:
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;

    void WriteHTMLMetaDataList( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTagBlocks( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    virtual void WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    // Appending to reference table:
    void AddToRefTable( CRefTable& oRefTable_o, const std::string& sOutputPostfix_i ) const;

    // Debug output:
    void DebugOutput() const;
};

// -----------------------------------------------------------------------------

// CLASS CPageMainpage

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Documentation page class for general information about the whole
///             project.
// *****************************************************************************

class escrido::CPageMainpage : public CDocPage
{
  private:

  public:

    // Constructor, destructor:
    CPageMainpage();

    // Methods for accessing selected content:

    // Output method:
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;
};

// -----------------------------------------------------------------------------

// CLASS CRefPage

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Documentation page class for reference pages of general
///             programming constructs like functions, data types, classes etc.
// *****************************************************************************

class escrido::CRefPage : public CDocPage
{
  public:

    // Constructor, destructor:
    CRefPage();

    // Methods for accessing selected content:
    virtual void AppendHeadlineChar( const char cIdentChar_i );

    // Output method:
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;

  private:

    void BuildPageTypeID();
};

// -----------------------------------------------------------------------------

// CLASS CDocumentation

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      A complete documentation.
// *****************************************************************************

class escrido::CDocumentation
{
  private:

    std::vector <CDocPage*> paDocPageList;               ///< List of all contained documentation pages.

    mutable bool fGroupOrdered;                          ///< Flag whether a group ordering is available for the documentation pages.
    mutable std::vector <escrido::CGroup> oaGroupList;   ///< List of the ordering groups.

  public:

    // Constructor, desctructor:
    CDocumentation();
    ~CDocumentation();

    // Content managment (used during parsing):
    void PushContentUnit( const CContentUnit& oContUnit_i );
    void NewDocPage( const char* szDocPageType_i );
    CDocPage* Back();

    // Special methods:
    void RemoveNamespaces( const std::vector<std::string>& saNSWhiteList_i );

    // Output methods:
    void WriteWebDoc( const std::string& sTemplateDir_i, const std::string& sOutputDir_i, const std::string& sOutputPostfix_i, bool fShowInternal_i ) const;
    void WriteLaTeXDoc( const std::string& sTemplateDir_i, const std::string& sOutputDir_i, bool fShowInternal_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    // Helper functions:
    void WriteTableOfContentHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteTOCPageType( const CGroup& oGroup_i,
                           const std::string& sPageTypeID_i,
                           std::ostream& oOutStrm_i,
                           const SWriteInfo& oWriteInfo_i ) const;

    void FillGroupListOrdered() const;
};


#endif /* ESCRIDO_DOC_ALLREADY_READ_IN */