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
// *     CGroupNode    *
// *                   *
// *********************

// *********************
// *                   *
// *     CGroupTree    *
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
  class CGroupNode;
  class CGroupTree;
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

  /// Search index encoding.
  enum class search_index_encoding
  {
    JSON,
    JS
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
                 std::string& sTemplateData_o );
  bool ReadTemp( const std::string& sTemplateDir_i,
                 const std::string& sFileName_i,
                 const std::string& sFallbackFileName_i,
                 std::string& sTemplateData_o );
  void WriteOutput( const std::string& sFileName_i,
                    const std::string& sTemplateData_i );

  // String replacement:
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const std::vector<std::string>& asReplacementList_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const std::string& sReplacement_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocPage& oPage_i,
                           void (CDocPage::*WriteMethod_i)( std::ostream&, const SWriteInfo& ) const,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocPage& oPage_i,
                           void (CDocPage::*WriteMethod_i)( tag_type, std::ostream&, const SWriteInfo& ) const,
                           tag_type fTagType_i,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocumentation& oDocumentation_i,
                           void (CDocumentation::*WriteMethod_i)( const CDocPage*, std::ostream&, const SWriteInfo& ) const,
                           const CDocPage* pPage_i,
                           const SWriteInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );

  void AdjustReplaceIndent( size_t nReplPos_i,
                            std::string& sTemplateData_io,
                            const SWriteInfo& oWriteInfo_io );
}

// -----------------------------------------------------------------------------

// CLASS CGroupNode

// -----------------------------------------------------------------------------

class escrido::CGroupNode
{
  public:

    const std::string sGroupName;
    std::vector <size_t> naDocPageIdxList;

  private:

    std::vector <CGroupNode*> apChildNodeList;

  public:

    // Constructor, destructor:
    CGroupNode( const std::string& sGroupName_i );
    ~CGroupNode();

    // Child node managment:
    CGroupNode* AddChildGroup( const std::string& sGroupName_i );
    CGroupNode* GetChildGroup( const std::string& sGroupName_i ) const;
    void Clear();
    void Order( const std::vector <CDocPage*>& apDocPageList_i,
                const std::vector <std::string>& asRefList_i );

  friend class CGroupTree;
};

// -----------------------------------------------------------------------------

// CLASS CGroupTree

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      A tree container for organizing the membership of individual
///             document pages in various groups.
// *****************************************************************************

class escrido::CGroupTree
{
  private:

    const std::vector <CDocPage*>& apDocPageList;

    CGroupNode oRoot;
    size_t nMaxLvl;

  public:

    // Constructor:
    CGroupTree( const std::vector <CDocPage*>& apDocPageList_i );

    void Update();
    void Clear();
    void Order( const std::vector <std::string>& asRefList_i );

    // Group access:
    const CGroupNode* FirstGroupNode( size_t& nLvl_o ) const;
    const CGroupNode* NextGroupNode( const CGroupNode* pLast_i, size_t& nLvl_o ) const;
    size_t MaxLvl() const;
    std::vector <std::string> GetGroupNames( const CGroupNode* pGroup_i ) const;
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
    const CContentUnit& GetContentUnit() const;
    const std::string  GetBrief() const;
    const std::string  GetNamespace() const;
    const std::vector<std::string> GetGroupNames() const;

    // Methods for accessing selected clear content:
    const std::string GetClearTextBrief( const SWriteInfo& oWriteInfo_i ) const;
    const std::string GetClearTextContent( const SWriteInfo& oWriteInfo_i ) const;

    // Output method:
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;

    void WriteHTMLMetaDataList( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLHeadline( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTagBlock( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTagBlockList( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

    void WriteLaTeXHeadline( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTagBlock( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;
    void WriteLaTeXTagBlockList( tag_type fTagType_i, std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const;

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

    std::vector <CDocPage*> paDocPageList; ///< List of all documentation pages contained.

    mutable bool fGroupOrdered;            ///< Flag whether a group ordering is available for the documentation pages.
    mutable CGroupTree oaGroupTree;        ///< Container for ordering of groups.

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
    void RemoveGroups( const std::vector<std::string>& saGroupBlackList_i );

    // Creation of reference table inside the write info object.
    void CreateRefTable( const std::string& sOutputPostfix_i,
                         SWriteInfo& oWriteInfo_io ) const;

    // Output methods:
    void WriteHTMLDoc( const std::string& sTemplateDir_i,
                       const std::string& sOutputDir_i,
                       const std::string& sOutputPostfix_i,
                       const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLSearchIndex( const std::string& sOutputDir_i,
                               const std::string& sOutputPath_i,
                               const std::string& sOutputPostfix_i,
                               const SWriteInfo& oWriteInfo_i,
                               const search_index_encoding fEncoding_i ) const;
    void WriteLaTeXDoc( const std::string& sTemplateDir_i,
                        const std::string& sOutputDir_i,
                        const SWriteInfo& oWriteInfo_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    // Helper functions:
    void WriteHTMLTableOfContent( const CDocPage* pWritePage_i,
                                  std::ostream& oOutStrm_i,
                                  const SWriteInfo& oWriteInfo_i ) const;
    void WriteHTMLTOCPageType( const CGroupNode& oGroup_i,
                               const std::string& sPageTypeID_i,
                               const CDocPage* pWritePage_i,
                               std::ostream& oOutStrm_i,
                               const SWriteInfo& oWriteInfo_i ) const;
    std::string CleanAndJSONEscape( const std::string& sText_i ) const;

    void FillGroupTreeOrdered() const;
};


#endif /* ESCRIDO_DOC_ALLREADY_READ_IN */
