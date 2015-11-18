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
//                        +--*     CPageFunc     *
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
  class CDocPage;            // Parent class of the various documentation pages.
  class CPageMainpage;       // Documentation main page class.
  class CPageFunc;           // Documentation page class for functions.
  class CDocumentation;      // A complete documentation.
}

// -----------------------------------------------------------------------------

// GLOBAL CONSTANTS

// -----------------------------------------------------------------------------

namespace escrido
{
  // Parser state types:
  enum class headline_parse_state : unsigned char
  {
    START,
    IDENTIFIER,
    POST_IDENT,
    TITLE
  };

  // Page types:
  enum class page_type : unsigned char
  {
    PAGE,
    MAINPAGE,
    FUNCTION
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
                           void (CDocPage::*WriteMethodHTML)( std::ostream&, const SWriteHTMLInfo& ) const,
                           const SWriteHTMLInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );
  void ReplacePlaceholder( const char* szPlaceholder_i,
                           const CDocumentation& oDocumentation_i,
                           void (CDocumentation::*WriteMethodHTML)( std::ostream&, const SWriteHTMLInfo& ) const,
                           const SWriteHTMLInfo& oWriteInfo_i,
                           std::string& sTemplateData_io );
  const std::string GetPageTypeStr( const page_type& fPageType_i );
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

    //TEST
public:

    // Headline related:
    std::string sIdent;            ///< The page identifier. (Often given as first word after the page tag.)
    std::string sTitle;            ///< The page title. (Often given as follow-up words after the page tag.)
    headline_parse_state fState;   ///< State for parsing headline.

    // Content related:
    CContentUnit oContUnit;        ///< The documentation page's content unit.

  public:

    // Constructor:
    CDocPage();

    // Appending of content (while parsing):
    void AppendContentUnit( const CContentUnit& oContUnit_i );

    // Methods for accessing selected content:
    void AppendHeadlineChar( const char cIdentChar_i );
    const std::string& GetIdent() const;
    const std::string& GetTitle() const;
    CContentUnit&      GetContentUnit();
    const std::string  GetBrief() const;
    const std::string  GetNamespace() const;
    const std::string  GetGroupName() const;

    // Output method:
    virtual const page_type   GetPageType() const;
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;
    virtual void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;

    // Appending to reference table:
    void AddToRefTable( CRefTable& oRefTable_o, const std::string& sOutputPostfix_i ) const;

    // Debug output:
    void DebugOutput() const;
};

// -----------------------------------------------------------------------------

// CLASS CPageFunc

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
    virtual const page_type   GetPageType() const;
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;
    virtual void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
};

// -----------------------------------------------------------------------------

// CLASS CPageFunc

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Documentation page class for functions.
// *****************************************************************************

class escrido::CPageFunc : public CDocPage
{
  private:

    CParamList oParamList;             ///< The parameter list.

  public:

    // Constructor, destructor:
    CPageFunc( const CParamList& oParamList_i );

    // Methods for accessing selected content:

    // Output method:
    virtual const page_type   GetPageType() const;
    virtual const std::string GetURL( const std::string& sOutputPostfix_i ) const;
    virtual void WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
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

    // Output method:
    void WriteWebDoc( const std::string& sTemplateDir_i, const std::string& sOutputDir_i, const std::string& sOutputPostfix_i, bool fShowInternal_i ) const;

    // Debug output:
    void DebugOutput() const;

  private:

    // Helper functions:
    void WriteTableOfContentHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const;
    void WriteTOCPageType( const CGroup& oGroup_i,
                           const page_type& fPageType_i,
                           std::ostream& oOutStrm_i,
                           const SWriteHTMLInfo& oWriteInfo_i ) const;

    void FillGroupListOrdered() const;
};


#endif /* ESCRIDO_DOC_ALLREADY_READ_IN */