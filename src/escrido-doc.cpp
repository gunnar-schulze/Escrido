// -----------------------------------------------------------------------------
/// \file       escrido-doc.cpp
///
/// \brief      Module implementation file for
///             the Escrido documentation page management.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2015 trinckle 3D GmbH
// -----------------------------------------------------------------------------

#include "escrido-doc.h"

#include <cstring>       // strlen
#include <sstream>       // std::stringstream, std::stringbuf
#include <fstream>       // std::ifstream, std::ofstream
#include <iostream>      // std::cout, std::cin, std::cerr, std::endl
#include <cctype>        // tolower, toupper
#include <algorithm>     // std::sort
#include <unordered_set> // std::unordered_set

// -----------------------------------------------------------------------------

// CLASS CGroupNode

// -----------------------------------------------------------------------------

escrido::CGroupNode::CGroupNode( const std::string& sGroupName_i ):
  sGroupName ( sGroupName_i )
{}

// .............................................................................

escrido::CGroupNode::~CGroupNode()
{
  this->Clear();
}

// .............................................................................

escrido::CGroupNode* escrido::CGroupNode::AddChildGroup( const std::string& sGroupName_i )
{
  CGroupNode* pNewSubGroup = new CGroupNode( sGroupName_i );

  apChildNodeList.push_back( pNewSubGroup );

  return pNewSubGroup;
}

// .............................................................................

escrido::CGroupNode* escrido::CGroupNode::GetChildGroup( const std::string& sGroupName_i ) const
{
  for( size_t g = 0; g < apChildNodeList.size(); ++g )
    if( apChildNodeList[g]->sGroupName == sGroupName_i )
      return apChildNodeList[g];

  return NULL;
}

// .............................................................................

void escrido::CGroupNode::Clear()
{
  for( size_t g = 0; g < apChildNodeList.size(); ++g )
    delete apChildNodeList[g];

  apChildNodeList.resize( 0 );
}

// .............................................................................

// *****************************************************************************
/// \brief      Orders the group node on base of a reference list.
///
/// \details    The function orderds the containing doc pages and the subgroup
///             nodes based on the given reference list.
///
///             Elements that are noted in the reference list are positoned at
///             the front in an order equal to the one of the reference list.
///             Other elements come after that in alphanumerical order.
// *****************************************************************************

void escrido::CGroupNode::Order( const std::vector <CDocPage*>& apDocPageList_i,
                                 const std::vector <std::string>& asRefList_i )
{
  // An object for alphanumeric sorting.
  struct SSortObj
  {
    const std::string* psTitle;
    size_t nIdx;

    SSortObj( const std::string* psTitle_i, size_t nIdx_i ):
      psTitle ( psTitle_i ),
      nIdx    ( nIdx_i )
    {};

    // Alphanumeric comparison
    bool operator<( const SSortObj& oRHS_i )
    {
      const std::string& sLHS = *this->psTitle;
      const std::string& sRHS = *oRHS_i.psTitle;

      size_t nMinLen = sLHS.size();
      if( sRHS.size() < nMinLen )
        nMinLen = sRHS.size();

      for( size_t c = 0; c < nMinLen; ++c )
        if( sLHS[c] != sRHS[c] )
        {
          return ( toupper( sLHS[c] ) < toupper( sRHS[c] ) );
        }

      // For strings equal up to nMinLen use string length:
      return sLHS.size() < sRHS.size();
    };
  };

  // Step 1: order doc pages inside by the @order tags.
  {
    // Create a new doc page list
    std::vector<size_t> anNewDocPageIdxList;

    // Step 1.1: loop over reference list
    for( size_t r = 0; r < asRefList_i.size(); ++r )
    {
      const std::string& sRef = asRefList_i[r];

      // Check if the reference occurs within the original list of doc pages.
      for( size_t p = 0; p < this->naDocPageIdxList.size(); ++p )
        if( apDocPageList_i[this->naDocPageIdxList[p]]->GetIdent() == sRef )
        {
          // Add doc page index to new doc page list.
          anNewDocPageIdxList.push_back( this->naDocPageIdxList[p] );

          // Remove doc page from original list. (Change of order does not matter.)
          this->naDocPageIdxList[p] = this->naDocPageIdxList.back();
          this->naDocPageIdxList.pop_back();

          break;
        }
    }

    // Step 1.2: order remaining doc pages alphanumerically by title.
    {
      // Create a sort object list of the remaining doc pages.
      std::vector <SSortObj> aoRemDocPageList;
      for( size_t p = 0; p < this->naDocPageIdxList.size(); ++p )
        aoRemDocPageList.emplace_back( &(apDocPageList_i[this->naDocPageIdxList[p]]->GetTitle()),
                                       this->naDocPageIdxList[p] );

      // Sort the list.
      std::sort( aoRemDocPageList.begin(), aoRemDocPageList.end() );

      // Add result to new doc pages list.
      for( size_t p = 0; p < aoRemDocPageList.size(); ++p )
        anNewDocPageIdxList.push_back( aoRemDocPageList[p].nIdx );
    }

    // Step 1.3: swap doc page index lists.
    this->naDocPageIdxList.swap( anNewDocPageIdxList );
  }

  // Step 2: order subgroups.
  {
    // Create a new child node list
    std::vector<CGroupNode*> anNewChildNodeList;

    // Step 2.1: loop over reference list
    for( size_t r = 0; r < asRefList_i.size(); ++r )
    {
      const std::string& sRef = asRefList_i[r];

      // Check if the reference occurs within the original list of children.
      for( size_t c = 0; c < this->apChildNodeList.size(); ++c )
        if( this->apChildNodeList[c]->sGroupName == sRef )
        {
          // Add subgroup to new child node list.
          anNewChildNodeList.push_back( this->apChildNodeList[c] );

          // Remove child node from original list. (Change of order does not matter.)
          this->apChildNodeList[c] = this->apChildNodeList.back();
          this->apChildNodeList.pop_back();

          break;
        }
    }

    // Step 2.2: order remaining child nodes alphanumerically by title.
    {
      // Create a sort object list of the remaining doc pages.
      std::vector <SSortObj> aoRemChildNodeList;
      for( size_t c = 0; c < this->apChildNodeList.size(); ++c )
        aoRemChildNodeList.emplace_back( &(this->apChildNodeList[c]->sGroupName),
                                         c );

      // Sort the list.
      std::sort( aoRemChildNodeList.begin(), aoRemChildNodeList.end() );

      // Add result to new doc pages list.
      for( size_t c = 0; c < aoRemChildNodeList.size(); ++c )
        anNewChildNodeList.push_back( this->apChildNodeList[aoRemChildNodeList[c].nIdx] );
    }

    // Step 2.3: swap child node lists.
    this->apChildNodeList.swap( anNewChildNodeList );
  }

  // Step 3: apply to all children.
  for( size_t c = 0; c < this->apChildNodeList.size(); ++c )
    this->apChildNodeList[c]->Order( apDocPageList_i, asRefList_i );
}

// -----------------------------------------------------------------------------

// CLASS CGroupTree

// -----------------------------------------------------------------------------

escrido::CGroupTree::CGroupTree( const std::vector <CDocPage*>& apDocPageList_i ) :
  apDocPageList ( apDocPageList_i ),
  oRoot         ( "Contents" ),
  nMaxLvl       ( 0 )
{}

// .............................................................................

// *****************************************************************************
/// @brief      Creates a full group tree based on the current state of the
///             doc pages list.
// *****************************************************************************

void escrido::CGroupTree::Update()
{
  this->Clear();

  for( size_t f = 0; f < this->apDocPageList.size(); f++ )
  {
    // Get list of group memberships (in priorized order) of the doc page.
    const std::vector<std::string> asDocPageGroupNames = apDocPageList[f]->GetGroupNames();

    // Update maximum group level depth.
    if( asDocPageGroupNames.size() > this->nMaxLvl )
      this->nMaxLvl = asDocPageGroupNames.size();

    // Cycle over tree in search for the respective group bin.
    CGroupNode* pGroupNode = &oRoot;
    for( size_t g = 0; g < asDocPageGroupNames.size(); ++g )
    {
      // Group name on this level.
      const std::string& sGroupName = asDocPageGroupNames[g];

      // Find or create subgroup as child of the current node.
      CGroupNode* pSubGroup = pGroupNode->GetChildGroup( sGroupName );
      if( pSubGroup == NULL )
        pSubGroup = pGroupNode->AddChildGroup( sGroupName );

      // Proceed with subgroup.
      pGroupNode = pSubGroup;
    }

    // Add the doc page index to the terminal (sub-)group
    pGroupNode->naDocPageIdxList.push_back( f );
  }
}

// .............................................................................

void escrido::CGroupTree::Clear()
{
  oRoot.Clear();
  this->nMaxLvl = 0;
}

// .............................................................................

void escrido::CGroupTree::Order( const std::vector <std::string>& asRefList_i )
{
  oRoot.Order( apDocPageList, asRefList_i );
}

// .............................................................................

const escrido::CGroupNode* escrido::CGroupTree::FirstGroupNode( size_t& nLvl_o ) const
{
  nLvl_o = 0;

  return &oRoot;
}

// .............................................................................

const escrido::CGroupNode* escrido::CGroupTree::NextGroupNode( const CGroupNode* pLast_i,
                                                               size_t& nLvl_o ) const
{
  // A search stack element
  struct SNode
  {
    const size_t nLvl;
    const CGroupNode* pNode;

    SNode( size_t nLvl_i, const CGroupNode* pNode_i ):
      nLvl  ( nLvl_i ),
      pNode ( pNode_i )
      {};
  };

  // A search stack for looping over the tree.
  // Initialize with root node.
  std::vector <SNode> apStack;
  apStack.emplace_back( 0, &oRoot );

  // Walk over tree.
  while( !apStack.empty() )
  {
    // Pop last node
    const CGroupNode* pNode = apStack.back().pNode;
    size_t nLvl = apStack.back().nLvl;
    apStack.pop_back();

    // Search for the given node:
    if( pNode == pLast_i )
    {
      if( pNode->apChildNodeList.empty() )
      {
        if( apStack.empty() )
        {
          // Return NULL, signaling that no further element proceeds.
          return NULL;
        }
        else
        {
          // Return next element in stack.
          nLvl_o = apStack.back().nLvl;
          return apStack.back().pNode;
        }
      }
      else
      {
        // Return first child.
        nLvl_o = nLvl + 1;
        return pNode->apChildNodeList.front();
      }
    }

    // Add all children back-to-front to the stack (i.e. depth first looping).
    for( size_t c = pNode->apChildNodeList.size(); c > 0; --c )
      apStack.emplace_back( nLvl + 1, pNode->apChildNodeList[c-1] );
  }

  return NULL;
}

// .............................................................................

size_t escrido::CGroupTree::MaxLvl() const
{
  return nMaxLvl;
}

// .............................................................................

std::vector<std::string> escrido::CGroupTree::GetGroupNames( const CGroupNode* pGroup_i ) const
{
  // A search stack element
  struct SNode
  {
    const size_t nLvl;
    const CGroupNode* pNode;

    SNode( size_t nLvl_i, const CGroupNode* pNode_i ):
      nLvl  ( nLvl_i ),
      pNode ( pNode_i )
      {};
  };

  // String list for the function result.
  std::vector<std::string> oResult;

  // A search stack for looping over the tree.
  // Initialize with root node.
  std::vector <SNode> apStack;
  apStack.emplace_back( 0, &oRoot );

  // Walk over tree.
  while( !apStack.empty() )
  {
    // Pop last node
    const CGroupNode* pNode = apStack.back().pNode;
    size_t nLvl = apStack.back().nLvl;
    apStack.pop_back();

    // Delete all names in the output string list up to this level.
    oResult.resize( nLvl );
    if( nLvl > 0 )
      oResult[nLvl - 1] = pNode->sGroupName;

    // Search for the given node:
    if( pNode == pGroup_i )
      return oResult;

    // Add all children back-to-front to the stack (i.e. depth first looping).
    for( size_t c = pNode->apChildNodeList.size(); c > 0; --c )
      apStack.emplace_back( nLvl + 1, pNode->apChildNodeList[c-1] );
  }

  return oResult;
}

// -----------------------------------------------------------------------------

// CLASS CParam

// -----------------------------------------------------------------------------

escrido::CParam::CParam( const char* szIdent_i ):
  sIdent      ( szIdent_i ),
  fHasDefault ( false )
{}

// .............................................................................

escrido::CParam::CParam( const char* szIdent_i, const char* szDefault_i ):
  sIdent      ( szIdent_i ),
  fHasDefault ( true ),
  sDefault    ( szDefault_i )
{}

// -----------------------------------------------------------------------------

// CLASS CParamList

// -----------------------------------------------------------------------------

void escrido::CParamList::AppendParam( const CParam& oParam_i )
{
  oaParamList.emplace_back( oParam_i );
}

// -----------------------------------------------------------------------------

// CLASS CDocPage

// -----------------------------------------------------------------------------

escrido::CDocPage::CDocPage() :
  sPageTypeLit ( "page" ),
  sPageTypeID  ( "page" ),
  fState       ( headline_parse_state::START )
{}

// .............................................................................

escrido::CDocPage::CDocPage( const char* szPageTypeLit_i,
                             const char* szPageTypeID_i,
                             const char* szIdent_i,
                             headline_parse_state fState_i ) :
  sPageTypeLit ( szPageTypeLit_i ),
  sPageTypeID  ( szPageTypeID_i ),
  sIdent       ( szIdent_i ),
  fState       ( fState_i )
{}

// .............................................................................

// *****************************************************************************
/// \brief      Appends a given content unit to the documentation page.
// *****************************************************************************

void escrido::CDocPage::AppendContentUnit( const CContentUnit& oContUnit_i )
{
  oContUnit.AppendContentUnit( oContUnit_i );
}

// .............................................................................

void escrido::CDocPage::AppendHeadlineChar( const char cIdentChar_i )
{
  // Parse the page headline. Typically it is one identifier word followed by
  // multiple words of the page title.
  switch( fState )
  {
    case headline_parse_state::START:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
        return;
      else
      {
        fState = headline_parse_state::IDENTIFIER;
        sIdent.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::IDENTIFIER:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
      {
        fState = headline_parse_state::POST_IDENT;
        return;
      }
      else
      {
        sIdent.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::POST_IDENT:
    {
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
        return;
      else
      {
        fState = headline_parse_state::TITLE;
        sTitle.push_back( cIdentChar_i );
        return;
      }
    }

    case headline_parse_state::TITLE:
      sTitle.push_back( cIdentChar_i );
      return;
  }
}

// .............................................................................

const std::string escrido::CDocPage::GetPageTypeLit() const
{
  return sPageTypeLit;
}

// .............................................................................

const std::string escrido::CDocPage::GetPageTypeID() const
{
  return sPageTypeID;
}

// .............................................................................

const std::string& escrido::CDocPage::GetIdent() const
{
  return sIdent;
}

// .............................................................................

const std::string& escrido::CDocPage::GetTitle() const
{
  return sTitle;
}

// .............................................................................

const escrido::CContentUnit& escrido::CDocPage::GetContentUnit() const
{
  return oContUnit;
}

// .............................................................................

const std::string escrido::CDocPage::GetBrief() const
{
  if( this->oContUnit.HasTagBlock( tag_type::BRIEF ) )
    return this->oContUnit.GetFirstTagBlock( tag_type::BRIEF )->GetPlainText();
  else
    return std::string();
}

// .............................................................................

const std::string escrido::CDocPage::GetNamespace() const
{
  if( this->oContUnit.HasTagBlock( tag_type::NAMESPACE ) )
    return this->oContUnit.GetFirstTagBlock( tag_type::NAMESPACE )->GetPlainFirstWord();
  else
    return std::string();
}

// .............................................................................

const std::vector <std::string> escrido::CDocPage::GetGroupNames() const
{
  std::vector <std::string> oResult;

  if( this->oContUnit.HasTagBlock( tag_type::INGROUP ) )
  {
    const CTagBlock* oTagBlock = this->oContUnit.GetFirstTagBlock( tag_type::INGROUP );
    oResult.push_back( oTagBlock->GetPlainTitleLine() );

    while( ( oTagBlock = this->oContUnit.GetNextTagBlock( oTagBlock, tag_type::INGROUP ) ) != NULL )
    {
      oResult.push_back( oTagBlock->GetPlainTitleLine() );
    }
  }

  return oResult;
}

// .............................................................................

const std::vector <std::string> escrido::CDocPage::GetFeatureNames() const
{
  std::vector <std::string> oResult;

  if( this->oContUnit.HasTagBlock( tag_type::FEATURE ) )
  {
    const CTagBlock* oTagBlock = this->oContUnit.GetFirstTagBlock( tag_type::FEATURE );
    oResult.push_back( oTagBlock->GetPlainFirstWordOrQuote() );

    while( ( oTagBlock = this->oContUnit.GetNextTagBlock( oTagBlock, tag_type::FEATURE ) ) != NULL )
    {
      oResult.push_back( oTagBlock->GetPlainFirstWordOrQuote() );
    }
  }

  return oResult;
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns a clear text of the page brief tag.
// *****************************************************************************

const std::string escrido::CDocPage::GetClearTextBrief( const SWriteInfo& oWriteInfo_i ) const
{
  if( this->oContUnit.HasTagBlock( tag_type::BRIEF ) )
  {
    const CTagBlock& oBriefTagBlock = *(this->oContUnit.GetFirstTagBlock( tag_type::BRIEF ));

    std::ostringstream oSStream;
    oBriefTagBlock.WriteHTML( oSStream, oWriteInfo_i );

    return ConvertHTML2ClearText( oSStream.str() );
  }
  else
    return std::string();
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns a clear text of the page content (without brief).
///
/// \details    The text returned is a clear text representation of the content
///             of the page. The BRIEF tag block is omitted since this is
///             captured separately by method @ref GetClearTextBrief().
///
///             The order of the content is the following: first come the
///             "flowing text" tag blocks (PARAGRAPH, SECTION, SUBSECTION,
///             SUBSUBSECTION, DETAILS and the embedded EXAMPLE, IMAGE,
///             INTERNAL, NOTE, OUTPUT and REMARK tag blocks,
///             see @ref CContentUnit::WriteHTMLParSectDet()) in their original
///             order, followed by the other tag block in original order.
// *****************************************************************************

const std::string escrido::CDocPage::GetClearTextContent( const SWriteInfo& oWriteInfo_i ) const
{
  // A flag list of all processed blocks
  std::vector <bool> afTagBlockDone( this->oContUnit.GetTagBlockN(), false );

  // String content
  std::string sContent;

  // First add all "real" text content.
  for( size_t t = 0; t < this->oContUnit.GetTagBlockN(); ++t )
  {
    const CTagBlock& oTagBlock = this->oContUnit.GetTagBlock( t );

    tag_type fTagType = oTagBlock.GetTagType();
    if( fTagType == tag_type::DETAILS ||
        fTagType == tag_type::EXAMPLE ||
        fTagType == tag_type::IMAGE ||
        fTagType == tag_type::NOTE ||
        fTagType == tag_type::OUTPUT ||
        fTagType == tag_type::PARAGRAPH ||
        fTagType == tag_type::REMARK ||
        fTagType == tag_type::SECTION ||
        fTagType == tag_type::SUBSECTION ||
        fTagType == tag_type::SUBSUBSECTION )
    {
      // Write HTML to string
      std::ostringstream oSStream;
      oTagBlock.WriteHTML( oSStream, oWriteInfo_i );
      sContent += oSStream.str();

      // Tag block as done.
      afTagBlockDone[t] = true;
    }
    else
      if( fTagType == tag_type::BRIEF )
      {
        // Block BRIEF tag block
        afTagBlockDone[t] = true;
      }
  }

  // Now append all remaining blocks
  for( size_t t = 0; t < this->oContUnit.GetTagBlockN(); ++t )
    if( !afTagBlockDone[t] )
    {
      const CTagBlock& oTagBlock = this->oContUnit.GetTagBlock( t );

      // Write HTML to string
      std::ostringstream oSStream;
      oTagBlock.WriteHTML( oSStream, oWriteInfo_i );
      sContent += oSStream.str();
    }

  // Convert to clear text and return.
  return ConvertHTML2ClearText( sContent );
}

// .............................................................................

const std::string escrido::CDocPage::GetURL( const std::string& sOutputPostfix_i ) const
{
  return std::string( "page_" + sIdent + sOutputPostfix_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of meta data such as AUTHOR, DATE etc.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLMetaDataList( std::ostream& oOutStrm_i,
                                               const SWriteInfo& oWriteInfo_i ) const
{
  // Start description list.
  WriteHTMLTagLine( "<dl>", oOutStrm_i, oWriteInfo_i++ );

  // Author:
  if( oContUnit.HasTagBlock( tag_type::AUTHOR ) )
  {
    const std::string sTagLine = std::string( "<dt class=\"author\">" ) +
                                 oWriteInfo_i.Label( "Author" ) + "</dt>";

    WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::AUTHOR )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // Date:
  if( oContUnit.HasTagBlock( tag_type::DATE ) )
  {
    const std::string sTagLine = std::string( "<dt class=\"date\">" ) +
                                 oWriteInfo_i.Label( "Date" ) + "</dt>";

    WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::DATE )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }
  else
  {
    // TODO: Put actual date

  }

  // Version:
  if( oContUnit.HasTagBlock( tag_type::VERSION ) )
  {
    const std::string sTagLine = std::string( "<dt class=\"version\">" ) +
                                 oWriteInfo_i.Label( "Version" ) + "</dt>";

    WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::VERSION )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // Copyright:
  if( oContUnit.HasTagBlock( tag_type::COPYRIGHT ) )
  {
    const std::string sTagLine = std::string( "<dt class=\"copyright\">" ) +
                                 oWriteInfo_i.Label( "Copyright" ) + "</dt>";

    WriteHTMLTagLine( sTagLine, oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::COPYRIGHT )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // End description list.
  WriteHTMLTagLine( "</dl>", oOutStrm_i, --oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of page headline.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLHeadline( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h1 id=\"" << sIdent << "\">" << sTitle << "</h1>" << std::endl;
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of PARAGRAPH, SECTION, SUBSECTION,
///             SUBSUBSECTION, DETAILS and embedded EXAMPLE, IMAGE, INTERNAL,
///             NOTES, OUTPUT and REMARK paragraphs in the order they appear.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteHTMLParSectDet( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of one tag block of a given type of the page.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLTagBlock( tag_type fTagType_i,
                                           std::ostream& oOutStrm_i,
                                           const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteHTMLTagBlock( fTagType_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of one tag block of a given type and
///             specified by a given identifier of the page.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLTagBlock( tag_type fTagType_i,
                                           const std::string& sIdentifier_i,
                                           std::ostream& oOutStrm_i,
                                           const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteHTMLTagBlock( fTagType_i, sIdentifier_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of all tag blocks of a given type of the page.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLTagBlockList( tag_type fTagType_i,
                                             std::ostream& oOutStrm_i,
                                             const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteHTMLTagBlockList( fTagType_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output of page headline.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXHeadline( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\pageheadline{" << sTitle << "}" << std::endl;
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "\\label{" << sIdent << "}" << std::endl;
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output of PARAGRAPH, SECTION, DETAILS and embedded
///             EXAMPLE, IMAGE, INTERNAL, NOTES, OUTPUT and REMARK paragraphs
///             in the order they appear.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteLaTeXParSectDet( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output of one tag block of a given type of the page.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXTagBlock( tag_type fTagType_i,
                                            std::ostream& oOutStrm_i,
                                            const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteLaTeXTagBlock( fTagType_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output of one tag block of a given type and
///             specified by a given identifier of the page.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXTagBlock( tag_type fTagType_i,
                                            const std::string& sIdentifier_i,
                                            std::ostream& oOutStrm_i,
                                            const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteLaTeXTagBlock( fTagType_i, sIdentifier_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output of all tag blocks of a given type of the page.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXTagBlockList( tag_type fTagType_i,
                                                std::ostream& oOutStrm_i,
                                                const SWriteInfo& oWriteInfo_i ) const
{
  oContUnit.WriteLaTeXTagBlockList( fTagType_i, oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

void escrido::CDocPage::AddToRefTable( CRefTable& oRefTable_o, const std::string& sOutputPostfix_i ) const
{
  // Add reference to the page.
  oRefTable_o.AppendRef( sIdent,
                         this->GetURL( sOutputPostfix_i ),
                         sTitle );

  // Add reference to sections.
  const CTagBlock* pTagBlock = oContUnit.GetFirstTagBlock( tag_type::SECTION );
  while( pTagBlock != NULL )
  {
    std::string sIdent = MakeIdentifier( pTagBlock->GetPlainFirstWord() );
    oRefTable_o.AppendRef( sIdent,
                           this->GetURL( sOutputPostfix_i ) + "#" + sIdent,
                           pTagBlock->GetPlainTitleLineButFirstWord() );
    pTagBlock = oContUnit.GetNextTagBlock( pTagBlock, tag_type::SECTION );
  }

  // Add reference to subsections.
  pTagBlock = oContUnit.GetFirstTagBlock( tag_type::SUBSECTION );
  while( pTagBlock != NULL )
  {
    std::string sIdent = MakeIdentifier( pTagBlock->GetPlainFirstWord() );
    oRefTable_o.AppendRef( sIdent,
                           this->GetURL( sOutputPostfix_i ) + "#" + sIdent,
                           pTagBlock->GetPlainTitleLineButFirstWord() );
    pTagBlock = oContUnit.GetNextTagBlock( pTagBlock, tag_type::SUBSECTION );
  }

  // Add reference to subsubsections.
  pTagBlock = oContUnit.GetFirstTagBlock( tag_type::SUBSUBSECTION );
  while( pTagBlock != NULL )
  {
    std::string sIdent = MakeIdentifier( pTagBlock->GetPlainFirstWord() );
    oRefTable_o.AppendRef( sIdent,
                           this->GetURL( sOutputPostfix_i ) + "#" + sIdent,
                           pTagBlock->GetPlainTitleLineButFirstWord() );
    pTagBlock = oContUnit.GetNextTagBlock( pTagBlock, tag_type::SUBSUBSECTION );
  }
}

// .............................................................................

void escrido::CDocPage::DebugOutput() const
{
  std::cout << this->GetPageTypeID() << ":" << std::endl;
  oContUnit.DebugOutput();
}

// -----------------------------------------------------------------------------

// CLASS CPageMainpage

// -----------------------------------------------------------------------------

escrido::CPageMainpage::CPageMainpage():
  CDocPage( "mainpage", "mainpage", "mainpage", headline_parse_state::POST_IDENT )
{}

// .............................................................................

const std::string escrido::CPageMainpage::GetURL( const std::string& sOutputPostfix_i ) const
{
  return std::string( "index" + sOutputPostfix_i );
}

// -----------------------------------------------------------------------------

// CLASS CRefPage

// -----------------------------------------------------------------------------

escrido::CRefPage::CRefPage():
  CDocPage( "", "", "", headline_parse_state::START )
{}

// .............................................................................

void escrido::CRefPage::AppendHeadlineChar( const char cIdentChar_i )
{
  // Parse the program construct headline. This is one program construct label
  // (as "function", "data type" or "class", possibly in quotation marks), an
  // identifier word followed by the individual name of the construct.
  switch( fState )
  {
    case headline_parse_state::START:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
        return;
      else
      if( cIdentChar_i == '"' )
      {
        fState = headline_parse_state::PAGETYPE_DQUOTED;
        return;
      }
      else
      {
        fState = headline_parse_state::PAGETYPE;
        sPageTypeLit.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::PAGETYPE:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
      {
        fState = headline_parse_state::POST_PAGETYPE;
        BuildPageTypeID();
        return;
      }
      else
      {
        sPageTypeLit.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::PAGETYPE_DQUOTED:
      if( cIdentChar_i == '"' )
      {
        fState = headline_parse_state::POST_PAGETYPE;
        BuildPageTypeID();
        return;
      }
      else
      {
        sPageTypeLit.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::POST_PAGETYPE:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
        return;
      else
      {
        fState = headline_parse_state::IDENTIFIER;
        sIdent.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::IDENTIFIER:
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
      {
        fState = headline_parse_state::POST_IDENT;
        return;
      }
      else
      {
        sIdent.push_back( cIdentChar_i );
        return;
      }

    case headline_parse_state::POST_IDENT:
    {
      if( cIdentChar_i == ' ' || cIdentChar_i == '\t' )
        return;
      else
      {
        fState = headline_parse_state::TITLE;
        sTitle.push_back( cIdentChar_i );
        return;
      }
    }

    case headline_parse_state::TITLE:
      sTitle.push_back( cIdentChar_i );
      return;
  }
}

// .............................................................................

const std::string escrido::CRefPage::GetURL( const std::string& sOutputPostfix_i ) const
{
  return std::string( sPageTypeID + "_" + sIdent + sOutputPostfix_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Builds an identifier from the literal page type.
/// \
/// \note       This function is very basic and can be improved to cover more
///             complex literary page types.
// *****************************************************************************

void escrido::CRefPage::BuildPageTypeID()
{
  sPageTypeID = sPageTypeLit;
  for( size_t c = 0; c < sPageTypeID.size(); c++ )
    if( sPageTypeID[c] == ' ' || sPageTypeID[c] == '\t' )
      sPageTypeID[c] = '_';
    else
      sPageTypeID[c] == tolower( sPageTypeID[c] );
}

// -----------------------------------------------------------------------------

// CLASS CDocumentation

// -----------------------------------------------------------------------------

escrido::CDocumentation::CDocumentation() :
  fGroupOrdered ( false ),
  oGroupTree    ( this->paDocPageList ),
  fNavOrderList ( false )
{}

// .............................................................................

escrido::CDocumentation::~CDocumentation()
{
  for( size_t s = 0; s < paDocPageList.size(); s++ )
    delete paDocPageList[s];
}

// .............................................................................

// *****************************************************************************
/// \brief      Appends a given content unit to the last documentation page
///             registered.
// *****************************************************************************

void escrido::CDocumentation::PushContentUnit( const CContentUnit& oContUnit_i )
{
  if( !paDocPageList.empty() )
    paDocPageList.back()->AppendContentUnit( oContUnit_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Appends a new documentation page of a specified type to the
///             documentation.
///
/// \param[in]  szDocPageType_i
///             Identifier tag of the new documentation page.
// *****************************************************************************

void escrido::CDocumentation::NewDocPage( const char* szDocPageType_i )
{
  // Copy C string into C++ string to make comparison code simpler.
  const std::string sDocPageType( szDocPageType_i );

  CDocPage* pNewPage = NULL;

  // Compare with known special documentation page names.
  if( sDocPageType == "_page_" )
    pNewPage = new CDocPage();
  if( sDocPageType == "_mainpage_" )
    pNewPage = new CPageMainpage();
  if( sDocPageType == "_refpage_" )
    pNewPage = new CRefPage();

  // Otherwise give a warning and use a defaul documentation page.
  if( pNewPage == NULL )
  {
    std::cerr << "unrecognized page type '@" << szDocPageType_i << "' treated as '@_page_'" << std::endl;
    pNewPage = new CDocPage();
  }
  else
    std::cout << "new " << sDocPageType << std::endl;

  // Append to list.
  paDocPageList.push_back( pNewPage );
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns a pointer to the last page registered (or NULL).
// *****************************************************************************

escrido::CDocPage* escrido::CDocumentation::Back()
{
  if( paDocPageList.empty() )
    return NULL;
  else
    return paDocPageList.back();
}

// .............................................................................

// *****************************************************************************
/// \brief      Returns a list of the names of all features present within the
///             document.
// *****************************************************************************

const std::vector <std::string> escrido::CDocumentation::GetFeatureNames() const
{
  std::unordered_set <std::string> asFeaturesFull;
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
  {
    const std::vector <std::string> asFeaturesPage = this->paDocPageList[p]->GetFeatureNames();

    for( size_t f = 0; f < asFeaturesPage.size(); ++f )
      asFeaturesFull.insert( asFeaturesPage[f] );
  }

  std::vector <std::string> asResult;
  for( std::unordered_set <std::string>::iterator iFeatName = asFeaturesFull.begin();
       iFeatName != asFeaturesFull.end();
       ++iFeatName )
    asResult.push_back( *iFeatName );

  return asResult;
}

// .............................................................................

// *****************************************************************************
/// \brief      Removes all documentation pages belonging to namespaces that
///             are not in the white list.
///
/// \param[in]  saNSWhiteList_i
///             White list of namespaces that are \b not deleted.
// *****************************************************************************

void escrido::CDocumentation::RemoveNamespaces( const std::vector<std::string>& saNSWhiteList_i )
{
  for( size_t f = 0; f < this->paDocPageList.size(); )
  {
    bool fRemove = true;
    for( size_t n = 0; n < saNSWhiteList_i.size(); n++ )
      if( this->paDocPageList[f]->GetNamespace() == saNSWhiteList_i[n] )
      {
        fRemove = false;
        break;
      }

    if( fRemove )
      this->paDocPageList.erase( this->paDocPageList.begin() + f );
    else
      f++;
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Removes all documentation pages belonging to groups that
///             are in the black list.
///
/// \param[in]  saGroupBlackList_i
///             Black list of groups that are deleted.
// *****************************************************************************

void escrido::CDocumentation::RemoveGroups( const std::vector<std::string>& saGroupBlackList_i )
{
  for( size_t f = 0; f < this->paDocPageList.size(); )
  {
    std::vector<std::string> asGroupPage = this->paDocPageList[f]->GetGroupNames();

    bool fRemove = false;
    for( size_t pg = 0; pg < asGroupPage.size(); ++pg )
    {
      for( size_t bg = 0; bg < saGroupBlackList_i.size(); bg++ )
      {
        if( asGroupPage[pg] == saGroupBlackList_i[bg] )
        {
          fRemove = true;
          goto break_loop;
        }
      }
    }

    // GOTO target for breaking the double loop:
    break_loop:

    if( fRemove )
      this->paDocPageList.erase( this->paDocPageList.begin() + f );
    else
      f++;
  }
}

// .............................................................................

void escrido::CDocumentation::CreateRefTable( const std::string& sOutputPostfix_i,
                                              SWriteInfo& oWriteInfo_io ) const
{
  // Create a reference table.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    this->paDocPageList[p]->AddToRefTable( oWriteInfo_io.oRefTable, sOutputPostfix_i );
}

// .............................................................................

void escrido::CDocumentation::WriteHTMLDoc( const std::string& sTemplateDir_i,
                                            const std::string& sOutputDir_i,
                                            const std::string& sOutputPostfix_i,
                                            const SWriteInfo& oWriteInfo_i ) const
{
  // Reset indentation in write info.
  oWriteInfo_i.nIndent = 0;

  // Get list of names of all "feature" tags present in the documentation.
  std::vector <std::string> aoFeatureNames = this->GetFeatureNames();

  // Find "mainpage".
  CPageMainpage* pMainpage = NULL;
  std::string sMainTitle = "Document Title";
  {
    for( size_t p = 0; p < this->paDocPageList.size(); p++ )
      if( this->paDocPageList[p]->GetIdent() == "mainpage" )
      {
        pMainpage = static_cast<CPageMainpage*>( this->paDocPageList[p] );
        sMainTitle = pMainpage->GetTitle();
        break;
      }
  }

  // Create all pages.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
  {
    // Get a pointer to this page.
    CDocPage* pPage = paDocPageList[p];

    // Output
    std::cout << "writing page '" << pPage->GetIdent() << "' ";

    // Deduce template file name.
    std::string sTemplateFileName;
    if( pPage->GetPageTypeID() == "mainpage" )
      sTemplateFileName = "index.html";
    else
      sTemplateFileName = pPage->GetPageTypeID() + ".html";

    // Try to read template.
    std::string sTemplatePage;
    if( ReadTemp( sTemplateDir_i, sTemplateFileName, "default.html", sTemplatePage ) )
    {
      // Replace mainpage placeholders in this page.
      ReplacePlaceholder( "*escrido-maintitle*", sMainTitle, sTemplatePage );
      if( pMainpage != NULL )
      {
        ReplacePlaceholder( "*escrido-metadata*", *pMainpage, &CDocPage::WriteHTMLMetaDataList, oWriteInfo_i, sTemplatePage );

        const CContentUnit* pMainContentUnit = &( pMainpage->GetContentUnit() );

        if( pMainContentUnit->HasTagBlock( tag_type::AUTHOR ) )
          ReplacePlaceholder( "*escrido-mainauthor*", pMainContentUnit->GetFirstTagBlock( tag_type::AUTHOR )->GetPlainText(), sTemplatePage );

        if( pMainContentUnit->HasTagBlock( tag_type::DATE ) )
          ReplacePlaceholder( "*escrido-maindate*", pMainContentUnit->GetFirstTagBlock( tag_type::DATE )->GetPlainText(), sTemplatePage );

        if( pMainContentUnit->HasTagBlock( tag_type::VERSION ) )
          ReplacePlaceholder( "*escrido-mainversion*", pMainContentUnit->GetFirstTagBlock( tag_type::VERSION )->GetPlainText(), sTemplatePage );

        if( pMainContentUnit->HasTagBlock( tag_type::COPYRIGHT ) )
          ReplacePlaceholder( "*escrido-maincopyright*", pMainContentUnit->GetFirstTagBlock( tag_type::COPYRIGHT )->GetPlainText(), sTemplatePage );

        if( pMainContentUnit->HasTagBlock( tag_type::BRIEF ) )
          ReplacePlaceholder( "*escrido-mainbrief*", pMainContentUnit->GetFirstTagBlock( tag_type::BRIEF )->GetPlainText(), sTemplatePage );
      }

      // Replace other placeholders in this page.
      ReplacePlaceholder( "*escrido-headline*", *pPage, &CDocPage::WriteHTMLHeadline, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-page-text*", *pPage, &CDocPage::WriteHTMLParSectDet, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-type*", GetCapForm( pPage->GetPageTypeLit() ), sTemplatePage );
      ReplacePlaceholder( "*escrido-groupname#*", pPage->GetGroupNames(), sTemplatePage );
      ReplacePlaceholder( "*escrido-title*", pPage->GetTitle(), sTemplatePage );
      ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteHTMLTableOfContent, pPage, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-pagination-url-prev*", *this, &CDocumentation::WriteHTMLPaginatorURLPrev, pPage, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-pagination-url-next*", *this, &CDocumentation::WriteHTMLPaginatorURLNext, pPage, oWriteInfo_i, sTemplatePage );

      ReplacePlaceholder( "*escrido-brief*", *pPage, &CDocPage::WriteHTMLTagBlock, tag_type::BRIEF, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-return*", *pPage, &CDocPage::WriteHTMLTagBlock, tag_type::RETURN, oWriteInfo_i, sTemplatePage );

      ReplacePlaceholder( "*escrido-attributes*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::ATTRIBUTE, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-params*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::PARAM, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-see*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::SEE, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-signatures*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::SIGNATURE, oWriteInfo_i, sTemplatePage );
      ReplacePlaceholder( "*escrido-features*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::FEATURE, oWriteInfo_i, sTemplatePage );

      // Construct and replace specific 'features' placeholder:
      for( size_t f = 0; f < aoFeatureNames.size(); ++f )
      {
        const std::string sPlaceholder = "*escrido-feature-" + GetCamelCase( aoFeatureNames[f] ) + "*";
        ReplacePlaceholder( sPlaceholder.c_str(), *pPage, &CDocPage::WriteHTMLTagBlock, tag_type::FEATURE, aoFeatureNames[f], oWriteInfo_i, sTemplatePage );
      }

      // Save data.
      WriteOutput( sOutputDir_i + pPage->GetURL( sOutputPostfix_i ), sTemplatePage );
    }

    // Output
    std::cout << std::endl;
  }
}

// .............................................................................

void escrido::CDocumentation::WriteHTMLSearchIndex( const std::string& sOutputDir_i,
                                                    const std::string& sOutputPath_i,
                                                    const std::string& sOutputPostfix_i,
                                                    const SWriteInfo& oWriteInfo_i,
                                                    const search_index_encoding fEncoding_i ) const
{
  // Reset indentation in write info.
  oWriteInfo_i.nIndent = 0;

  // Generate combined output file name
  std::string sCombined = sOutputDir_i + sOutputPath_i;

  // Output
  std::cout << "writing file '" << sOutputPath_i << "'" << std::endl;

  // Open output file.
  std::ofstream oOutFile( sCombined.c_str(), std::ofstream::out );

  if( fEncoding_i == search_index_encoding::JS )
    oOutFile << "const searchIndex = ";

  // Write file opening (i.e. JSON array opening):
  oOutFile << "[" << std::endl;

  // Write index data for all pages
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
  {
    // Get a reference to this page.
    const CDocPage& oPage = *paDocPageList[p];

    // Write opening curly bracket.
    oOutFile << "   {" << std::endl;

    // Write general page information
    oOutFile << "      \"title\":\"" << oPage.GetTitle() << "\"," << std::endl;
    oOutFile << "      \"brief\":\"" << CleanAndJSONEscape( oPage.GetClearTextBrief( oWriteInfo_i ) ) << "\"," << std::endl;
    oOutFile << "      \"url\":\"" << oPage.GetURL( sOutputPostfix_i ) << "\"," << std::endl;
    oOutFile << "      \"content\":\"" << CleanAndJSONEscape( oPage.GetClearTextContent( oWriteInfo_i ) ) << "\"," << std::endl;

    // Write closing curly bracket.
    if( p + 1 == this->paDocPageList.size() )
      oOutFile << "   }" << std::endl;
    else
      oOutFile << "   }," << std::endl;
  }

  // Write file closing (i.e. JSON array closing):
  oOutFile << "]";

  if( fEncoding_i == search_index_encoding::JS )
    oOutFile << ";";

  oOutFile.close();
}

// .............................................................................

void escrido::CDocumentation::WriteLaTeXDoc( const std::string& sTemplateDir_i,
                                             const std::string& sOutputDir_i,
                                             const SWriteInfo& oWriteInfo_i ) const
{
  // Reset indentation in write info.
  oWriteInfo_i.nIndent = 0;

  // Get list of names of all "feature" tags present in the documentation.
  std::vector <std::string> aoFeatureNames = this->GetFeatureNames();

  // Find "mainpage".
  CPageMainpage* pMainpage = NULL;
  const CContentUnit* pMainContentUnit = NULL;
  {
    for( size_t p = 0; p < this->paDocPageList.size(); p++ )
      if( this->paDocPageList[p]->GetIdent() == "mainpage" )
      {
        pMainpage = static_cast<CPageMainpage*>( this->paDocPageList[p] );
        pMainContentUnit = &( pMainpage->GetContentUnit() );
        break;
      }
  }

  // Create LaTeX file.
  {
    // Try to read template for the base document.
    std::string sTemplateDoc;
    if( ReadTemp( sTemplateDir_i, "latex.tex", "latex.tex", sTemplateDoc ) )
    {
      // Include packages necessarily required by the system.
      ReplacePlaceholder( "*escrido_latex_packages*", "\
% Package for graphics inclusion:\n\
\\usepackage{graphicx}\n\
% Program code listings:\n\
\\usepackage{listings}\n\
% Auto-aligned tables:\n\
\\usepackage{tabulary}\n\
% Hyperlinks and hyper references:\n\
\\usepackage{hyperref}", sTemplateDoc );

      // Include new commands and new environments necessariliy required by the system.
      // Since they may be altered by the user, these settings are stored in
      // a template file.
      std::string sTepmplateNewCommands;
      if( ReadTemp( sTemplateDir_i, "latex_commands.tex", sTepmplateNewCommands ) )
        ReplacePlaceholder( "*escrido_latex_commands*", sTepmplateNewCommands, sTemplateDoc );

      // Replace mainpage placeholders in base document.
      if( pMainpage != NULL )
      {
        ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplateDoc );

        if( pMainContentUnit->HasTagBlock( tag_type::AUTHOR ) )
          ReplacePlaceholder( "*escrido-mainauthor*", pMainContentUnit->GetFirstTagBlock( tag_type::AUTHOR )->GetPlainText(), sTemplateDoc );

        if( pMainContentUnit->HasTagBlock( tag_type::DATE ) )
          ReplacePlaceholder( "*escrido-maindate*", pMainContentUnit->GetFirstTagBlock( tag_type::DATE )->GetPlainText(), sTemplateDoc );

        if( pMainContentUnit->HasTagBlock( tag_type::VERSION ) )
          ReplacePlaceholder( "*escrido-mainversion*", pMainContentUnit->GetFirstTagBlock( tag_type::VERSION )->GetPlainText(), sTemplateDoc );

        if( pMainContentUnit->HasTagBlock( tag_type::COPYRIGHT ) )
          ReplacePlaceholder( "*escrido-maincopyright*", pMainContentUnit->GetFirstTagBlock( tag_type::COPYRIGHT )->GetPlainText(), sTemplateDoc );

        if( pMainContentUnit->HasTagBlock( tag_type::BRIEF ) )
          ReplacePlaceholder( "*escrido-mainbrief*", pMainContentUnit->GetFirstTagBlock( tag_type::BRIEF )->GetPlainText(), sTemplateDoc );
      }

      // Loop over group tree to write groups content
      size_t nLvl;
      const CGroupNode* pGroup = this->oGroupTree.FirstGroupNode( nLvl );
      while( pGroup != NULL )
      {
        // Write group headline if
        // - groups are used at all
        // - this group contains any pages
        if( ( this->oGroupTree.MaxLvl() > 0 ) &&
            !pGroup->naDocPageIdxList.empty() )
        {
          // Expand "*escrido-pages*" in order to add a group headline
          ReplacePlaceholder( "*escrido-pages*",
                              "\\pagegroupheadline{*escrido-grouptitle*}%\n\n*escrido-pages*",
                              sTemplateDoc );

          // Write group name
          if( nLvl == 0 )
          {
            // Root level group becomes 'Introduction':
            ReplacePlaceholder( "*escrido-grouptitle*", "Introduction", sTemplateDoc );
          }
          else
          {
            // Get list of parent names of this group.
            std::vector <std::string> asGroupNameList = this->oGroupTree.GetGroupNames( pGroup );

            // Generate a combined name string from that.
            std::string sCombGroupName;
            for( size_t gn = 0; gn < asGroupNameList.size(); ++gn )
            {
              sCombGroupName += asGroupNameList[gn];
              if( gn + 1 < asGroupNameList.size() )
                sCombGroupName += " - ";
            }

            // Set combined group name
            ReplacePlaceholder( "*escrido-grouptitle*", sCombGroupName, sTemplateDoc );
          }
        }

        // Write pages of the group.
        for( size_t p = 0; p < pGroup->naDocPageIdxList.size(); p++ )
        {
          // Get a pointer to this page.
          CDocPage* pPage = paDocPageList[pGroup->naDocPageIdxList[p]];

          // Output
          std::cout << "writing page '" << pPage->GetIdent() << "' ";

          // Deduce template file name.
          std::string sTemplateFileName;
          if( pPage->GetPageTypeID() == "mainpage" )
            sTemplateFileName = "page.tex";
          else
            sTemplateFileName = pPage->GetPageTypeID() + ".tex";

          // Try to read template.
          std::string sTemplatePage;
          if( ReadTemp( sTemplateDir_i, sTemplateFileName, "default.tex", sTemplatePage ) )
          {
            // Expand "*escrido-pages*":
            ReplacePlaceholder( "*escrido-pages*", "*escrido-page*\n*escrido-pages*", sTemplateDoc );

            // Replace mainpage placeholders in this page.
            if( pMainpage != NULL )
            {
              ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplatePage );

              if( pMainContentUnit->HasTagBlock( tag_type::AUTHOR ) )
                ReplacePlaceholder( "*escrido-mainauthor*", pMainContentUnit->GetFirstTagBlock( tag_type::AUTHOR )->GetPlainText(), sTemplatePage );

              if( pMainContentUnit->HasTagBlock( tag_type::DATE ) )
                ReplacePlaceholder( "*escrido-maindate*", pMainContentUnit->GetFirstTagBlock( tag_type::DATE )->GetPlainText(), sTemplatePage );

              if( pMainContentUnit->HasTagBlock( tag_type::BRIEF ) )
                ReplacePlaceholder( "*escrido-mainbrief*", pMainContentUnit->GetFirstTagBlock( tag_type::BRIEF )->GetPlainText(), sTemplatePage );
            }

            // Replace other placeholders in this page.
            ReplacePlaceholder( "*escrido-headline*", *pPage, &CDocPage::WriteLaTeXHeadline, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-page-text*", *pPage, &CDocPage::WriteLaTeXParSectDet, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-type*", GetCapForm( pPage->GetPageTypeLit() ), sTemplatePage );
            ReplacePlaceholder( "*escrido-groupname#*", pPage->GetGroupNames(), sTemplatePage );
            ReplacePlaceholder( "*escrido-title*", pPage->GetTitle(), sTemplatePage );

            ReplacePlaceholder( "*escrido-brief*", *pPage, &CDocPage::WriteLaTeXTagBlock, tag_type::BRIEF, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-return*", *pPage, &CDocPage::WriteLaTeXTagBlock, tag_type::RETURN, oWriteInfo_i, sTemplatePage );

            ReplacePlaceholder( "*escrido-attributes*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::ATTRIBUTE, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-params*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::PARAM, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-see*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::SEE, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-signatures*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::SIGNATURE, oWriteInfo_i, sTemplatePage );
            ReplacePlaceholder( "*escrido-features*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::FEATURE, oWriteInfo_i, sTemplatePage );

            // Construct and replace specific 'features' placeholder:
            for( size_t f = 0; f < aoFeatureNames.size(); ++f )
            {
              const std::string sPlaceholder = "*escrido-feature-" + GetCamelCase( aoFeatureNames[f] ) + "*";
              ReplacePlaceholder( sPlaceholder.c_str(), *pPage, &CDocPage::WriteLaTeXTagBlock, tag_type::FEATURE, aoFeatureNames[f], oWriteInfo_i, sTemplatePage );
            }

            // Enter page into base document.
            ReplacePlaceholder( "*escrido-page*", sTemplatePage, sTemplateDoc );
          }

          // Output
          std::cout << std::endl;
        }

        // Get next group
        pGroup = this->oGroupTree.NextGroupNode( pGroup, nLvl );
      }

      // Delete final "*escrido-pages*":
      ReplacePlaceholder( "*escrido-pages*", "", sTemplateDoc );

      // Save data.
      WriteOutput( sOutputDir_i + "latex.tex", sTemplateDoc );
    }
    else
      std::cerr << "unable to read LaTeX template file 'latex.tex'." << std::endl;
  }
}

// .............................................................................

void escrido::CDocumentation::DebugOutput() const
{
  for( size_t f = 0; f < this->paDocPageList.size(); f++ )
    this->paDocPageList[f]->DebugOutput();
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes the navigation table into a page.
///
/// \note       This method works in analogy to \ref FillGroupTreeOrdered().
///             Any change of ordering the pages in one of the methods must be
///             translated to the other.
// *****************************************************************************

void escrido::CDocumentation::WriteHTMLTableOfContent( const CDocPage* pWritePage_i,
                                                       std::ostream& oOutStrm_i,
                                                       const SWriteInfo& oWriteInfo_i ) const
{
  if( !fGroupOrdered )
    this->FillGroupTreeOrdered();

  // Loop over group tree:
  signed int nPrevLvl = -1;
  size_t nLvl;
  const CGroupNode* pGroup = this->oGroupTree.FirstGroupNode( nLvl );
  while( pGroup != NULL )
  {
    // Closing tags of group div containers
    if( (signed int) nLvl <= nPrevLvl )
      for( int i = 0; i <= ( nPrevLvl - nLvl ); ++i )
        WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "</div>" << std::endl;

    // Opening tag of group div container
    WriteHTMLIndents( oOutStrm_i, oWriteInfo_i++ ) << "<div>" << std::endl;

    // If group name is not empty: display it
    if( !(pGroup->sGroupName.empty()) )
    {
      // Determine HTML heading level from the group nesting level.
      // Don't go below level 5 since level 6 is reserved for page type.
      size_t nHeadingLvl = nLvl + 1;
      if( nHeadingLvl > 5 )
        nHeadingLvl = 5;

      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) <<
        "<h" << nHeadingLvl << ">" << pGroup->sGroupName << "</h" << nHeadingLvl << ">" << std::endl;
    }

    // Write all pages of page types "mainpage" and "page" of this group.
    WriteHTMLTOCPageType( *pGroup, "mainpage", pWritePage_i, oOutStrm_i, oWriteInfo_i );
    WriteHTMLTOCPageType( *pGroup, "page", pWritePage_i, oOutStrm_i, oWriteInfo_i );

    // Populate a list of all user-defined page types that appear in this group.
    std::vector<std::string>saPageTypeList;
    for( size_t p = 0; p < pGroup->naDocPageIdxList.size(); ++p )
    {
      std::string sPageTypeID = paDocPageList[pGroup->naDocPageIdxList[p]]->GetPageTypeID();

      bool fExists = false;
      for( size_t pt = 0; pt < saPageTypeList.size(); ++pt )
        if( saPageTypeList[pt] == sPageTypeID )
        {
          fExists = true;
          break;
        }

      if( !fExists )
        saPageTypeList.push_back( sPageTypeID );
    }

    // Write all pages of all other page types of this group.
    for( size_t pt = 0; pt < saPageTypeList.size(); ++pt )
      if( saPageTypeList[pt] != "mainpage" && saPageTypeList[pt] != "page" )
        WriteHTMLTOCPageType( *pGroup, saPageTypeList[pt], pWritePage_i, oOutStrm_i, oWriteInfo_i );

    // Store previous level
    nPrevLvl = nLvl;

    // Get next group
    pGroup = this->oGroupTree.NextGroupNode( pGroup, nLvl );
  }

  // Close remaining group div containers
  for( int i = 0; i <= ( nPrevLvl - 0 ); ++i )
    WriteHTMLIndents( oOutStrm_i, --oWriteInfo_i ) << "</div>" << std::endl;
}

// .............................................................................

void escrido::CDocumentation::WriteHTMLTOCPageType( const CGroupNode& oGroup_i,
                                                    const std::string& sPageTypeID_i,
                                                    const CDocPage* pWritePage_i,
                                                    std::ostream& oOutStrm_i,
                                                    const SWriteInfo& oWriteInfo_i ) const
{
  // Check whether any pages of this type exist and (if so) retrieve the
  // literal form of the page type name.
  bool fPageExist = false;
  std::string sPageTypeLit;
  for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); ++p )
    if( paDocPageList[oGroup_i.naDocPageIdxList[p]]->GetPageTypeID() == sPageTypeID_i )
    {
      // Get the literary form of the page type name.
      sPageTypeLit = paDocPageList[oGroup_i.naDocPageIdxList[p]]->GetPageTypeLit();

      // Note that at least one page exists.
      fPageExist = true;
      break;
    }

  // Show list of pages if at least one page of that type exists.
  if( fPageExist )
  {
    // For all but "page" and "mainpage": write a headline.
    if( sPageTypeID_i != "page" &&
        sPageTypeID_i != "mainpage" )
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h6>" << GetCapPluralForm( sPageTypeLit ) << "</h6>" << std::endl;

    WriteHTMLTagLine( "<ul>", oOutStrm_i, oWriteInfo_i );
    ++oWriteInfo_i;

    // Loop over all pages of the group:
    for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); ++p )
    {
      CDocPage* pPage = paDocPageList[oGroup_i.naDocPageIdxList[p]];
      if( pPage->GetPageTypeID() == sPageTypeID_i )
      {
        // Get brief description, if it exists.
        std::string sBrief = pPage->GetClearTextBrief( oWriteInfo_i );

        // Write list item tag, either with or w/o brief description as title.
        {
          if( sBrief.empty() )
            WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li";
          else
            WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li title=\"" << sBrief << "\"";

          // Mark if the entry is the page currently written.
          if( pPage == pWritePage_i )
            oOutStrm_i << " class=\"activepage\"";

          // Close <li> tag.
          oOutStrm_i << ">";
        }

        // Create link, if available:
        bool fLink = false;
        {
          size_t nRefIdx;
          if( oWriteInfo_i.oRefTable.GetRefIdx( pPage->GetIdent(), nRefIdx ) )
          {
            fLink = true;
            oOutStrm_i << "<a href=\""  << oWriteInfo_i.oRefTable.GetLink( nRefIdx ) << "\">";
          }
        }

        oOutStrm_i << pPage->GetTitle();

        // Eventually close link.
        if( fLink )
          oOutStrm_i << "</a>";

        oOutStrm_i << "</li>" << std::endl;
      }
    }

    WriteHTMLTagLine( "</ul>", oOutStrm_i, --oWriteInfo_i );
  }
}

// .............................................................................

void escrido::CDocumentation::WriteHTMLPaginatorURLPrev( const CDocPage* pWritePage_i,
                                                          std::ostream& oOutStrm_i,
                                                          const SWriteInfo& oWriteInfo_i ) const
{
  if( !fNavOrderList )
    this->FillNavOrderList();

  // Loop over all elements of the doc page nav order list.
  for( size_t np = 0; np < anNavOrderPageIdxList.size(); ++np )
  {
    // Check if page is this page.
    if( this->paDocPageList[anNavOrderPageIdxList[np]] == pWritePage_i )
    {
      // Get pointer to previous page.
      // Take care of wrap-around at first element.
      const CDocPage* pNextPage;
      if( np == 0 )
        pNextPage = this->paDocPageList[anNavOrderPageIdxList.back()];
      else
        pNextPage = this->paDocPageList[anNavOrderPageIdxList[np - 1]];

      // Get reference index and output link url.
      size_t nRefIdx;
      if( oWriteInfo_i.oRefTable.GetRefIdx( pNextPage->GetIdent(), nRefIdx ) )
      {
        oOutStrm_i << oWriteInfo_i.oRefTable.GetLink( nRefIdx );
      }
    }
  }
}

// .............................................................................

void escrido::CDocumentation::WriteHTMLPaginatorURLNext( const CDocPage* pWritePage_i,
                                                         std::ostream& oOutStrm_i,
                                                         const SWriteInfo& oWriteInfo_i ) const
{
  if( !fNavOrderList )
    this->FillNavOrderList();

  // Loop over all elements of the doc page nav order list.
  for( size_t np = 0; np < anNavOrderPageIdxList.size(); ++np )
  {
    // Check if page is this page.
    if( this->paDocPageList[anNavOrderPageIdxList[np]] == pWritePage_i )
    {
      // Get pointer to next page.
      // Take care of wrap-around after last element.
      const CDocPage* pNextPage;
      if( np + 1 < anNavOrderPageIdxList.size() )
        pNextPage = this->paDocPageList[anNavOrderPageIdxList[np + 1]];
      else
        pNextPage = this->paDocPageList[anNavOrderPageIdxList[0]];

      // Get reference index and output link url.
      size_t nRefIdx;
      if( oWriteInfo_i.oRefTable.GetRefIdx( pNextPage->GetIdent(), nRefIdx ) )
      {
        oOutStrm_i << oWriteInfo_i.oRefTable.GetLink( nRefIdx );
      }
    }
  }
}

// .............................................................................

// *****************************************************************************
/// \brief      Fills the group tree oGroupTree in an ordered form.
// *****************************************************************************

void escrido::CDocumentation::FillGroupTreeOrdered() const
{
  // Step 1: create group tree and fill in all pages.
  oGroupTree.Update();

  // Step 2: create a reference list from the @order tags of the main page.
  std::vector <std::string> saOrderRefList;
  {
    // Check if a main page exists.
    CPageMainpage* pMainpage = NULL;
    for( size_t p = 0; p < paDocPageList.size(); p++ )
      if( paDocPageList[p]->GetIdent() == "mainpage" )
      {
        pMainpage = static_cast<CPageMainpage*>( this->paDocPageList[p] );
        break;
      }

    if( pMainpage != NULL )
    {
      // Set mainpage itself as first element of the reference list.
      saOrderRefList.emplace_back( pMainpage->GetIdent() );

      // Now cycle over @order block elements of the main page and add them
      // into the reference list.
      const CContentUnit& oMainpageCUnit = pMainpage->GetContentUnit();
      const CTagBlock* pTagBlockOrder = oMainpageCUnit.GetFirstTagBlock( tag_type::ORDER );
      while( pTagBlockOrder != NULL )
      {
        std::string sOrderText = pTagBlockOrder->GetPlainText();

        // Tokenize and add to the order list.
        size_t nBeg = sOrderText.find_first_not_of( " \t," );
        while( nBeg != std::string::npos )
        {
          size_t nDelim = sOrderText.find_first_of( ",", nBeg );
          if( nDelim == std::string::npos )
          {
            size_t nEnd = sOrderText.find_last_not_of( " \t," );

            saOrderRefList.emplace_back( sOrderText.substr( nBeg, nEnd - nBeg + 1 ) );
            break;
          }
          else
          {
            size_t nEnd = sOrderText.find_last_not_of( " \t,", nDelim );

            saOrderRefList.emplace_back( sOrderText.substr( nBeg, nEnd - nBeg + 1 ) );
            nBeg = sOrderText.find_first_not_of( " \t,", nDelim );
          }
        }

        pTagBlockOrder = oMainpageCUnit.GetNextTagBlock( pTagBlockOrder, tag_type::ORDER );
      }
    }
  }

  // Step 3: sort the group tree based on the reference list and alphanumeric
  //         order.
  oGroupTree.Order( saOrderRefList );

  fGroupOrdered = true;
}

// .............................................................................

// *****************************************************************************
/// \brief      Fills anNavOrderPageIdxList, the list of indices to paDocPageList in
///             the order as the pages appear in the navigation.
///
/// \note       This method works in analogy to \ref WriteHTMLTableOfContent().
///             Any change of ordering the pages in one of the methods must be
///             translated to the other.
// *****************************************************************************

void escrido::CDocumentation::FillNavOrderList() const
{
  // Make sure the group list is prepared.
  if( !fGroupOrdered )
    this->FillGroupTreeOrdered();

  // Clear the page nav order list.
  anNavOrderPageIdxList.clear();

  // Loop over group tree:
  size_t nLvl;
  const CGroupNode* pGroup = this->oGroupTree.FirstGroupNode( nLvl );
  while( pGroup != NULL )
  {
    // Create a list of all page types within this group.
    // Start with "mainpage" and "page" type, like in WriteHTMLTableOfContent()
    std::vector<std::string>saPageTypeList = { "mainpage", "page" };

    // Populate the list with all user-defined page types that appear in this group.
    for( size_t p = 0; p < pGroup->naDocPageIdxList.size(); p++ )
    {
      std::string sPageTypeID = paDocPageList[pGroup->naDocPageIdxList[p]]->GetPageTypeID();

      bool fExists = false;
      for( size_t pt = 0; pt < saPageTypeList.size(); ++pt )
        if( saPageTypeList[pt] == sPageTypeID )
        {
          fExists = true;
          break;
        }

      if( !fExists )
        saPageTypeList.push_back( sPageTypeID );
    }

    // Loop over the list of page types.
    for( size_t pt = 0; pt < saPageTypeList.size(); ++pt )
    {
      const std::string& sPageTypeID = saPageTypeList[pt];

      // Loop over all pages in the group
      for( size_t p = 0; p < pGroup->naDocPageIdxList.size(); ++p )
      {
        size_t nDocPageIdx = pGroup->naDocPageIdxList[p];

        // If the page is in the current page type: add it to the order list.
        if( paDocPageList[nDocPageIdx]->GetPageTypeID() == sPageTypeID )
          anNavOrderPageIdxList.push_back( nDocPageIdx );
      }
    }

    // Get next group
    pGroup = this->oGroupTree.NextGroupNode( pGroup, nLvl );
  }

  fNavOrderList = true;
}

// .............................................................................

// *****************************************************************************
/// \brief      Creates a cleaned and JSON escaped string for the search index.
// *****************************************************************************

std::string escrido::CDocumentation::CleanAndJSONEscape( const std::string& sText_i ) const
{
  std::string sResult( sText_i );

  for( size_t c = 0; c < sResult.size(); ++c )
  {
    switch( sResult[c] )
    {
      case '\r':
      {
        if( c + 1 < sResult.size() && sResult[c + 1] == '\n' )
          sResult.replace( c, 2, 1, ' ' );
        else
          sResult.replace( c, 1, 1, ' ' );
        break;
      }

      case '\n':
        sResult.replace( c, 1, 1, ' ' );
        break;

      case '\t':
        sResult.replace( c, 1, 1, ' ' );
        break;

      case '"':
        sResult.insert( c++, 1, '\\' );
        break;

      case '\\':
        sResult.insert( c++, 1, '\\' );
        break;
    }

    // HIER WEITERMACHEN
  }

  return sResult;
}

// -----------------------------------------------------------------------------

// FUNCTIONS IMPLEMENTATION

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Reads a text file (the template file) into a memory buffer.
///
/// \param[in]  sTemplateDir_i
///             String containting the <em>template directory</em>.
/// \param[in]  sFileName_i
///             String containing the name of the template file that is search
///             for.
/// \param[out] sTemplateData_o
///             This string returns the final content of the template file.
///
/// \return     'true' if the template file or the alternative template file
///             could be loaded, 'false' otherwise.
// *****************************************************************************

bool escrido::ReadTemp( const std::string& sTemplateDir_i,
                        const std::string& sFileName_i,
                        std::string& sTemplateData_o )
{
  // Try opening the template file.
  std::ifstream oInFile( sTemplateDir_i + sFileName_i, std::ifstream::in | std::ifstream::binary );
  if( !oInFile.is_open() )
  {
    std::cerr << "cannot load template file '" << sTemplateDir_i + sFileName_i << "' - skipping page" << std::endl;
    return false;
  }

  // Use iterator-template way of read the file completely (good 'best practice" method);
  // (Attention: the extra brackets arround the first constructor are essential; DO NOT REMOVE;)
  sTemplateData_o.clear();
  sTemplateData_o.assign( (std::istreambuf_iterator<char>( oInFile )),
                           std::istreambuf_iterator<char>() );

  // Close file again.
  oInFile.close();

  return true;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Reads a text file (the template file) or an alternative text
///             file into a memory buffer.
///
/// \details    The function tries to read one file and, if it cannot find the
///             first one, another file into a string. Used to read in template
///             files.
///
/// \param[in]  sTemplateDir_i
///             String containting the <em>template directory</em>.
/// \param[in]  sFileName_i
///             String containing the name of the template file that is search
///             for in the first place.
/// \param[in]  sFallbackFileName_i
///             String containing a second filename that serves as alternative,
///             if file sFileName_i could be found.
/// \param[out] sTemplateData_o
///             This string returns the final content of the template file.
///
/// \return     'true' if the template file or the alternative template file
///             could be loaded, 'false' otherwise.
// *****************************************************************************

bool escrido::ReadTemp( const std::string& sTemplateDir_i,
                        const std::string& sFileName_i,
                        const std::string& sFallbackFileName_i,
                        std::string& sTemplateData_o )
{
  // Try opening the template file.
  std::ifstream oInFile( sTemplateDir_i + sFileName_i, std::ifstream::in | std::ifstream::binary );
  if( !oInFile.is_open() )
  {
    // Cannot open template file. Try default file instead.
    oInFile.open( sTemplateDir_i + sFallbackFileName_i, std::ifstream::in | std::ifstream::binary );
    if( !oInFile.is_open() )
    {
      std::cerr << "cannot load template files '" << sTemplateDir_i + sFileName_i << "' or '" << sTemplateDir_i + sFallbackFileName_i << "' - skipping page" << std::endl;
      return false;
    }
    else
      std::cout << "(template file '" << sTemplateDir_i + sFallbackFileName_i << "')";
  }

  // Use iterator-template way of read the file completely (good 'best practice" method);
  // (Attention: the extra brackets arround the first constructor are essential; DO NOT REMOVE;)
  sTemplateData_o.clear();
  sTemplateData_o.assign( (std::istreambuf_iterator<char>( oInFile )),
                          std::istreambuf_iterator<char>() );

  // Close file again.
  oInFile.close();

  return true;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Writes a buffer into a text file (the output).
// *****************************************************************************

void escrido::WriteOutput( const std::string& sFileName_i,
                           const std::string& sTemplateData_i )
{
  // Open output file.
  std::ofstream oOutFile( sFileName_i.c_str(), std::ofstream::out );

  // Write template data.
  oOutFile.write( sTemplateData_i.data(), sTemplateData_i.size() );

  oOutFile.close();
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by a
///             string out of a list of strings.
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const std::vector<std::string>& asReplacementList_i,
                                  std::string& sTemplateData_io )
{
  // Step 1: replace without numbering wildcard character.
  {
    // Prepare placeholder.
    std::string sPlaceholder( szPlaceholder_i );
    size_t nPos = sPlaceholder.find( '#' );
    if( nPos != std::string::npos )
      sPlaceholder.erase( nPos, 1 );

    // Prepare replacement: either first element or empty string.
    std::string sReplacement;
    if( !asReplacementList_i.empty() )
      sReplacement = asReplacementList_i[0];

    ReplacePlaceholder( sPlaceholder.c_str(), sReplacement, sTemplateData_io );
  }

  // Step 2: replace with different index numbers at wildcard position.
  if( strchr( szPlaceholder_i, '#' ) != NULL )
  {
    // Either check for indices [0,9] or how many elements are in the replacement
    // vector.
    size_t nIdxN = 10;
    if( asReplacementList_i.size() > nIdxN )
      nIdxN = asReplacementList_i.size();

    for( size_t i = 0; i < nIdxN; ++i )
    {
      // Prepare placeholder.
      std::string sPlaceholder = szPlaceholder_i;
      size_t nPos = sPlaceholder.find( '#' );
      sPlaceholder.replace( nPos, 1, std::to_string( i ) );

      // Prepare replacement: either ith element or empty string.
      std::string sReplacement;
      if( i < asReplacementList_i.size() )
        sReplacement = asReplacementList_i[i];

      ReplacePlaceholder( sPlaceholder.c_str(), sReplacement, sTemplateData_io );
    }
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by a
///             given string.
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const std::string& sReplacement_i,
                                  std::string& sTemplateData_io )
{
  const size_t nPlaceholderLen = strlen( szPlaceholder_i );
  size_t nReplPos = sTemplateData_io.find( szPlaceholder_i );
  while( nReplPos != std::string::npos )
  {
    sTemplateData_io.replace( nReplPos, nPlaceholderLen, sReplacement_i );
    nReplPos = sTemplateData_io.find( szPlaceholder_i, nReplPos + sReplacement_i.length() );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by the
///             output of a given member function of a given
///             \ref CDocPage object.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oPage_i
///             Reference to the \ref CDocPage documentation page class
///             containing the generation method.
/// \param[in]  WriteMethod_i
///             Pointer to the generation method (member of oPage_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethod_i().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the output of WriteMethod_i().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocPage& oPage_i,
                                  void (CDocPage::*WriteMethod_i)( std::ostream&, const SWriteInfo& ) const,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  size_t nReplPos = sTemplateData_io.find( szPlaceholder_i );
  if( nReplPos != std::string::npos )
  {
    // Check for indentation counter adjustment.
    AdjustReplaceIndent( nReplPos, sTemplateData_io, oWriteInfo_i );

    // Create replacement string.
    std::stringstream sReplacement;
    ( oPage_i.*WriteMethod_i )( sReplacement, oWriteInfo_i );

    // Replace all occurrences of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by the
///             output of a given member function of a given
///             \ref CDocPage object, specified by tag type.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oPage_i
///             Reference to the \ref CDocPage documentation page class
///             containing the generation method.
/// \param[in]  WriteMethod_i
///             Pointer to the generation method (member of oPage_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  fTagType_i
///             Type of the tag whose code is inserted.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethod_i().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the output of WriteMethod_i().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocPage& oPage_i,
                                  void (CDocPage::*WriteMethod_i)( tag_type, std::ostream&, const SWriteInfo& ) const,
                                  tag_type fTagType_i,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  size_t nReplPos = sTemplateData_io.find( szPlaceholder_i );
  if( nReplPos != std::string::npos )
  {
    // Check for indentation counter adjustment.
    AdjustReplaceIndent( nReplPos, sTemplateData_io, oWriteInfo_i );

    // Create replacement string.
    std::stringstream sReplacement;
    ( oPage_i.*WriteMethod_i )( fTagType_i, sReplacement, oWriteInfo_i );

    // Replace all occurrences of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by the
///             output of a given member function of a given
///             \ref CDocPage object, specified by tag type and an identifier.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oPage_i
///             Reference to the \ref CDocPage documentation page class
///             containing the generation method.
/// \param[in]  WriteMethod_i
///             Pointer to the generation method (member of oPage_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  fTagType_i
///             Type of the tag whose code is inserted.
/// \param[in]  sIdentifier_i
///             String that is used in addition to fTagType_i to identify the
///             tag whose code is inserted.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethod_i().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the output of WriteMethod_i().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocPage& oPage_i,
                                  void (CDocPage::*WriteMethod_i)( tag_type, const std::string&, std::ostream&, const SWriteInfo& ) const,
                                  tag_type fTagType_i,
                                  const std::string& sIdentifier_i,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  size_t nReplPos = sTemplateData_io.find( szPlaceholder_i );
  if( nReplPos != std::string::npos )
  {
    // Check for indentation counter adjustment.
    AdjustReplaceIndent( nReplPos, sTemplateData_io, oWriteInfo_i );

    // Create replacement string.
    std::stringstream sReplacement;
    ( oPage_i.*WriteMethod_i )( fTagType_i, sIdentifier_i, sReplacement, oWriteInfo_i );

    // Replace all occurrences of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by the
///             output of a given member function of a given
///             \ref CDocumentation object.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oDocumentation_i
///             Reference to the \ref CDocumentation class containing the
///             generation method.
/// \param[in]  WriteMethod_i
///             Pointer to the generation method (member of oDocumentation_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  pPage_i
///             Pointer to the page currently in progress.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethod_i().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the output of WriteMethod_i().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocumentation& oDocumentation_i,
                                  void (CDocumentation::*WriteMethod_i)( const CDocPage*, std::ostream&, const SWriteInfo& ) const,
                                  const CDocPage* pPage_i,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  size_t nReplPos = sTemplateData_io.find( szPlaceholder_i );
  if( nReplPos != std::string::npos )
  {
    // Check for indentation counter adjustment.
    AdjustReplaceIndent( nReplPos, sTemplateData_io, oWriteInfo_i );

    // Create replacement string.
    std::stringstream sReplacement;
    ( oDocumentation_i.*WriteMethod_i )( pPage_i, sReplacement, oWriteInfo_i );

    // Replace all occurrences of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Used inside string replacement to adjust the indentation of
///             blocks accordingly to the replacement marker.
// *****************************************************************************

void escrido::AdjustReplaceIndent( size_t nReplPos_i,
                                   std::string& sTemplateData_io,
                                   const SWriteInfo& oWriteInfo_io )
{
  size_t nCountBlanks = 0;
  size_t j = nReplPos_i;
  while( true )
  {
    if( j == 0 )
      break;
    else
      --j;

    if( sTemplateData_io[j] == '\r' || sTemplateData_io[j] == '\n' )
    {
      // => An indentation before the replacement string has been found.

      // Remove the indentation in sTemplateData_io.
      sTemplateData_io.erase( j+1, nCountBlanks );

      // Set indentation counter to this level.
      oWriteInfo_io.nIndent = nCountBlanks;
      break;
    }

    if( sTemplateData_io[j] == ' ' )
      ++nCountBlanks;
    else
      break;
  }
}
