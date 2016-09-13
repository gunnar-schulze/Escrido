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

#include <cstring>      // strlen
#include <sstream>      // std::stringstream, std::stringbuf
#include <fstream>      // std::ifstream, std::ofstream
#include <iostream>     // std::cout, std::cin, std::cerr, std::endl
#include <cctype>       // tolower, toupper

// -----------------------------------------------------------------------------

// CLASS CGroup

// -----------------------------------------------------------------------------

escrido::CGroup::CGroup( const std::string& sGroupName_i ):
  sGroupName ( sGroupName_i )
{}

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

escrido::CContentUnit& escrido::CDocPage::GetContentUnit()
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

const std::string escrido::CDocPage::GetGroupName() const
{
  if( this->oContUnit.HasTagBlock( tag_type::INGROUP ) )
    return this->oContUnit.GetFirstTagBlock( tag_type::INGROUP )->GetPlainText();  // TODO: GetPlainFirstLine
  else
    return std::string();
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

void escrido::CDocPage::WriteHTMLMetaDataList( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Start description list.
  WriteHTMLTagLine( "<dl>", oOutStrm_i, oWriteInfo_i++ );

  // Author:
  if( oContUnit.HasTagBlock( tag_type::AUTHOR ) )
  {
    WriteHTMLTagLine( "<dt class=\"author\">Author</dt>", oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::AUTHOR )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // Date:
  if( oContUnit.HasTagBlock( tag_type::DATE ) )
  {
    WriteHTMLTagLine( "<dt class=\"date\">Date</dt>", oOutStrm_i, oWriteInfo_i );
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
    WriteHTMLTagLine( "<dt class=\"version\">Version</dt>", oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "<dd>", oOutStrm_i, oWriteInfo_i++ );
    oContUnit.GetFirstTagBlock( tag_type::VERSION )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTagLine( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // Copyright:
  if( oContUnit.HasTagBlock( tag_type::COPYRIGHT ) )
  {
    WriteHTMLTagLine( "<dt class=\"copyright\">Copyright</dt>", oOutStrm_i, oWriteInfo_i );
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
/// \brief      Writes HTML output of PARAGRAPH, SECTION and DETAILS paragraphs
///             in the order they appear.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // PARAGRAPH, SECTION and DETAILS paragraphs:
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
/// \brief      Writes LaTeX output of PARAGRAPH, SECTION and DETAILS paragraphs
///             in the order they appear.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeXParSectDet( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // PARAGRAPH, SECTION and DETAILS paragraphs:
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
  oRefTable_o.AppendRef( sIdent, this->GetURL( sOutputPostfix_i ) );

  // Add reference to sections.
  const CTagBlock* pTagBlock = oContUnit.GetFirstTagBlock( tag_type::SECTION );
  while( pTagBlock != NULL )
  {
    std::string sIdent = MakeIdentifier( pTagBlock->GetPlainFirstWord() );
    oRefTable_o.AppendRef( sIdent, this->GetURL( sOutputPostfix_i ) + "#" + sIdent );
    pTagBlock = oContUnit.GetNextTagBlock( pTagBlock, tag_type::SECTION );
  }

  // Add reference to subsections.
  pTagBlock = oContUnit.GetFirstTagBlock( tag_type::SUBSECTION );
  while( pTagBlock != NULL )
  {
    std::string sIdent = MakeIdentifier( pTagBlock->GetPlainFirstWord() );
    oRefTable_o.AppendRef( sIdent, this->GetURL( sOutputPostfix_i ) + "#" + sIdent );
    pTagBlock = oContUnit.GetNextTagBlock( pTagBlock, tag_type::SUBSECTION );
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
  // (as "function", "data type" or "class", may be in quotation marks), an
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
  fGroupOrdered ( false )
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

void escrido::CDocumentation::WriteHTMLDoc( const std::string& sTemplateDir_i,
                                            const std::string& sOutputDir_i,
                                            const std::string& sOutputPostfix_i,
                                            bool fShowInternal_i ) const
{
  // Create a write information container.
  SWriteInfo oWriteInfo;
  oWriteInfo.fShowInternal = fShowInternal_i;
  oWriteInfo.nIndent = 0;

  // Create a reference table.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    this->paDocPageList[p]->AddToRefTable( oWriteInfo.oRefTable, sOutputPostfix_i );

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

  // Create all other pages.
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
        ReplacePlaceholder( "*escrido-metadata*", *pMainpage, &CDocPage::WriteHTMLMetaDataList, oWriteInfo, sTemplatePage );

      // Replace other placeholders in this page.
      ReplacePlaceholder( "*escrido-headline*", *pPage, &CDocPage::WriteHTMLHeadline, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-page-text*", *pPage, &CDocPage::WriteHTMLParSectDet, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-type*", GetCapForm( pPage->GetPageTypeLit() ), sTemplatePage );
      ReplacePlaceholder( "*escrido-groupname*", pPage->GetGroupName(), sTemplatePage );
      ReplacePlaceholder( "*escrido-title*", pPage->GetTitle(), sTemplatePage );
      ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteTableOfContentHTML, oWriteInfo, sTemplatePage );

      ReplacePlaceholder( "*escrido-brief*", *pPage, &CDocPage::WriteHTMLTagBlock, tag_type::BRIEF, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-return*", *pPage, &CDocPage::WriteHTMLTagBlock, tag_type::RETURN, oWriteInfo, sTemplatePage );

      ReplacePlaceholder( "*escrido-attributes*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::ATTRIBUTE, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-params*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::PARAM, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-see*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::SEE, oWriteInfo, sTemplatePage );
      ReplacePlaceholder( "*escrido-signatures*", *pPage, &CDocPage::WriteHTMLTagBlockList, tag_type::SIGNATURE, oWriteInfo, sTemplatePage );

      // Save data.
      WriteOutput( sOutputDir_i + pPage->GetURL( sOutputPostfix_i ), sTemplatePage );
    }

    // Output
    std::cout << std::endl;
  }
}

// .............................................................................

void escrido::CDocumentation::WriteLaTeXDoc( const std::string& sTemplateDir_i,
                                             const std::string& sOutputDir_i,
                                             bool fShowInternal_i ) const
{
  // Create a write information container.
  SWriteInfo oWriteInfo;
  oWriteInfo.fShowInternal = fShowInternal_i;
  oWriteInfo.nIndent = 0;

  // Create a reference table.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    this->paDocPageList[p]->AddToRefTable( oWriteInfo.oRefTable, "" );

  // Find "mainpage".
  CPageMainpage* pMainpage = NULL;
  CContentUnit* pMainContentUnit = NULL;
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

        if( pMainContentUnit->HasTagBlock( tag_type::BRIEF ) )
          ReplacePlaceholder( "*escrido-mainbrief*", pMainContentUnit->GetFirstTagBlock( tag_type::BRIEF )->GetPlainText(), sTemplateDoc );
      }

      // Loop over groups:
      for( size_t g = 0; g < this->oaGroupList.size(); g++ )
      {
        // Expand "*escrido-pages*" and
        ReplacePlaceholder( "*escrido-pages*", "\\pagegroupheadline{*escrido-grouptitle*}%\n\n*escrido-pages*", sTemplateDoc );

        // Insert group name as chapter title:
        if( oaGroupList[g].sGroupName.empty() )
          ReplacePlaceholder( "*escrido-grouptitle*", "Introduction", sTemplateDoc );
        else
          ReplacePlaceholder( "*escrido-grouptitle*", oaGroupList[g].sGroupName, sTemplateDoc );

        // Write pages of the group.
        for( size_t p = 0; p < oaGroupList[g].naDocPageIdxList.size(); p++ )
        {
          // Get a pointer to this page.
          CDocPage* pPage = paDocPageList[oaGroupList[g].naDocPageIdxList[p]];

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
            ReplacePlaceholder( "*escrido-headline*", *pPage, &CDocPage::WriteLaTeXHeadline, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-page-text*", *pPage, &CDocPage::WriteLaTeXParSectDet, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-type*", GetCapForm( pPage->GetPageTypeLit() ), sTemplatePage );
            ReplacePlaceholder( "*escrido-groupname*", pPage->GetGroupName(), sTemplatePage );
            ReplacePlaceholder( "*escrido-title*", pPage->GetTitle(), sTemplatePage );

            ReplacePlaceholder( "*escrido-brief*", *pPage, &CDocPage::WriteLaTeXTagBlock, tag_type::BRIEF, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-return*", *pPage, &CDocPage::WriteLaTeXTagBlock, tag_type::RETURN, oWriteInfo, sTemplatePage );

            ReplacePlaceholder( "*escrido-attributes*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::ATTRIBUTE, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-params*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::PARAM, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-see*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::SEE, oWriteInfo, sTemplatePage );
            ReplacePlaceholder( "*escrido-signatures*", *pPage, &CDocPage::WriteLaTeXTagBlockList, tag_type::SIGNATURE, oWriteInfo, sTemplatePage );

            // Enter page into base document.
            ReplacePlaceholder( "*escrido-page*", sTemplatePage, sTemplateDoc );
          }

          // Output
          std::cout << std::endl;
        }
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

void escrido::CDocumentation::WriteTableOfContentHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  if( !fGroupOrdered )
    this->FillGroupListOrdered();

  WriteHTMLTagLine( "<h1>Contents</h1>", oOutStrm_i, oWriteInfo_i );

  // Loop over groups:
  for( size_t g = 0; g < this->oaGroupList.size(); g++ )
  {
    // If groups are used: show group name.
    if( this->oaGroupList.size() > 1 )
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>" << oaGroupList[g].sGroupName << "</h2>" << std::endl;

    // Write page types "mainpage" and "page".
    WriteTOCPageType( this->oaGroupList[g], "mainpage", oOutStrm_i, oWriteInfo_i );
    WriteTOCPageType( this->oaGroupList[g], "page", oOutStrm_i, oWriteInfo_i );

    // Populate a list of all user-defined page types that appear in this group.
    std::vector<std::string>saPageTypeList;
    for( size_t p = 0; p < this->oaGroupList[g].naDocPageIdxList.size(); p++ )
    {
      std::string sPageTypeID = paDocPageList[this->oaGroupList[g].naDocPageIdxList[p]]->GetPageTypeID();

      bool fExists = false;
      for( size_t pt = 0; pt < saPageTypeList.size(); pt++ )
        if( saPageTypeList[pt] == sPageTypeID )
        {
          fExists = true;
          break;
        }

      if( !fExists )
        saPageTypeList.push_back( sPageTypeID );
    }

    // Write all other page types.
    for( size_t pt = 0; pt < saPageTypeList.size(); pt++ )
      if( saPageTypeList[pt] != "mainpage" && saPageTypeList[pt] != "page" )
        WriteTOCPageType( this->oaGroupList[g], saPageTypeList[pt], oOutStrm_i, oWriteInfo_i );
  }
}

// .............................................................................

void escrido::CDocumentation::WriteTOCPageType( const CGroup& oGroup_i,
                                                const std::string& sPageTypeID_i,
                                                std::ostream& oOutStrm_i,
                                                const SWriteInfo& oWriteInfo_i ) const
{
  // Check whether any pages of this type exist and (if so) retrieve the
  // lierary form of the page type name.
  bool fPageExist = false;
  std::string sPageTypeLit;
  for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); p++ )
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
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h3>" << GetCapPluralForm( sPageTypeLit ) << "</h3>" << std::endl;

    WriteHTMLTagLine( "<ul>", oOutStrm_i, oWriteInfo_i );
    ++oWriteInfo_i;

    // Loop over all pages of the group:
    for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); p++ )
    {
      CDocPage* pPage = paDocPageList[oGroup_i.naDocPageIdxList[p]];
      if( pPage->GetPageTypeID() == sPageTypeID_i )
      {
        // Get brief description, if it exists.
        std::string sBrief = pPage->GetBrief();

        // Write list item tag, either with or w/o brief description as title.
        if( sBrief.empty() )
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li>";
        else
          WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<li title=\"" << sBrief << "\">";

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

// *****************************************************************************
/// \brief      Fills the group list oaGroupList in an ordered form.
// *****************************************************************************

void escrido::CDocumentation::FillGroupListOrdered() const
{
  // Step 1: create group list and fill in all pages.
  oaGroupList.clear();
  for( size_t f = 0; f < this->paDocPageList.size(); f++ )
  {
    bool fFound = false;
    for( size_t g = 0; g < oaGroupList.size(); g++ )
      if( this->paDocPageList[f]->GetGroupName() == oaGroupList[g].sGroupName )
      {
        oaGroupList[g].naDocPageIdxList.push_back( f );
        fFound = true;
        break;
      }

    if( !fFound )
    {
      oaGroupList.emplace_back( this->paDocPageList[f]->GetGroupName() );
      oaGroupList.back().naDocPageIdxList.push_back( f );
    }
  }

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
          size_t nEnd = sOrderText.find_first_of( " \t,", nBeg );
          if( nEnd == std::string::npos )
          {
            saOrderRefList.emplace_back( sOrderText.substr( nBeg ) );
            break;
          }
          else
          {
            saOrderRefList.emplace_back( sOrderText.substr( nBeg, nEnd - nBeg ) );
            nBeg = sOrderText.find_first_not_of( " \t,", nEnd );
          }
        }

        pTagBlockOrder = oMainpageCUnit.GetNextTagBlock( pTagBlockOrder, tag_type::ORDER );
      }
    }
  }

  // Step 3: order pages of each group by the @order tags.
  // (This implements the most primitive sorting algorithm but it only has to work on
  // small sets.)
  if( !saOrderRefList.empty() )
    for( size_t g = 0; g < oaGroupList.size(); g++ )
      for( size_t p1 = 0; p1 < oaGroupList[g].naDocPageIdxList.size(); p1++ )
      {
        // Get identifier of page p1.
        const std::string sIdentP1 = paDocPageList[oaGroupList[g].naDocPageIdxList[p1]]->GetIdent();

        // Check if order reference list contains p1.
        bool fRef = false;
        for( size_t r1 = 0; r1 < saOrderRefList.size(); r1++ )
          if( saOrderRefList[r1] == sIdentP1 )
          {
            // For each element after r1...
            for( size_t r2 = r1 + 1; r2 < saOrderRefList.size(); r2++ )
            {
              // ... check if there is an element p2 before p1...
              for( size_t p2 = 0; p2 < p1; p2++ )
              {
                // Get identifier of page p2.
                const std::string sIdentP2 = paDocPageList[oaGroupList[g].naDocPageIdxList[p2]]->GetIdent();

                if( saOrderRefList[r2] == sIdentP2 )
                {
                  // ... and exchange p1 and p2.
                  size_t nBuf = oaGroupList[g].naDocPageIdxList[p1];
                  oaGroupList[g].naDocPageIdxList[p1] = oaGroupList[g].naDocPageIdxList[p2];
                  oaGroupList[g].naDocPageIdxList[p2] = nBuf;
                  break;
                }
              }
            }
            break;
          }
      }


  // Step 4: order groups by the @order tags
  if( !saOrderRefList.empty() )
  {
    // Create a list of group indices that has the correct order.
    std::vector <size_t> naOrderGroupIdxList;

    // Eventually add the group with empty name first, if this one exists.
    for( size_t g = 0; g < oaGroupList.size(); g++ )
      if( oaGroupList[g].sGroupName.empty() )
      {
        naOrderGroupIdxList.push_back( g );
        break;
      }

    // Add other groups in the order of their appearance in the order reference list.
    for( size_t r = 0; r < saOrderRefList.size(); r++ )
    {
      // Check if there is a group assoziated with this reference
      size_t nGroupIdx;
      bool fFound = false;
      for( nGroupIdx = 0; nGroupIdx < oaGroupList.size(); nGroupIdx++ )
      {
        for( size_t p = 0; p < oaGroupList[nGroupIdx].naDocPageIdxList.size(); p++ )
        {
          // Check whether the reference points to this page.
          if( saOrderRefList[r] == paDocPageList[oaGroupList[nGroupIdx].naDocPageIdxList[p]]->GetIdent() )
          {
            fFound = true;
            break;
          }
        }
        if( fFound )
          break;
      }

     // Appending to ordered group list.
     if( fFound )
     {
       // Check whether the index is already listed.
       bool fNewIndex = true;
       for( size_t og = 0; og < naOrderGroupIdxList.size(); og++ )
         if( naOrderGroupIdxList[og] == nGroupIdx )
         {
           fNewIndex = false;
           break;
         }

       if( fNewIndex )
         naOrderGroupIdxList.push_back( nGroupIdx );
     }
    } // Creation of ordered group index list.

    // Append all group indices that are not in the order group index list yet.
    for( int g = 0; g < oaGroupList.size(); g++ )
    {
      bool fListed = false;
      for( size_t og = 0; og < naOrderGroupIdxList.size(); og++ )
        if( naOrderGroupIdxList[og] == g )
        {
          fListed = true;
          break;
        }

      if( !fListed )
        naOrderGroupIdxList.push_back( g );
    }

    // Create a full copy of the group list.
    std::vector <CGroup> oaGroupListCpy( oaGroupList );

    // Refill the group list in the correct order.
    for( size_t g = 0; g < oaGroupList.size(); g++ )
      oaGroupList[g] = oaGroupListCpy[naOrderGroupIdxList[g]];
  }

  fGroupOrdered = true;
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
/// \brief      Exchanges a placeholder inside a to-be-modified string by a
///             given function.
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
///             string will be replaced by the return value of
///             WriteMethod_i().
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

    // Replace all occurances of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by a
///             given block tag.
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
///             string will be replaced by the return value of
///             WriteMethod_i().
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

    // Replace all occurances of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Exchanges a placeholder inside a to-be-modified string by a
///             given function.
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
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethod_i().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the return value of
///             WriteMethod_i().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocumentation& oDocumentation_i,
                                  void (CDocumentation::*WriteMethod_i)( std::ostream&, const SWriteInfo& ) const,
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
    ( oDocumentation_i.*WriteMethod_i )( sReplacement, oWriteInfo_i );

    // Replace all occurances of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Used inside string replacement to asjust the indentation of
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
