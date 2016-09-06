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
    return this->oContUnit.GetFirstTagBlock( tag_type::NAMESPACE )->GetPlainText();
  else
    return std::string();
}

// .............................................................................

const std::string escrido::CDocPage::GetGroupName() const
{
  if( this->oContUnit.HasTagBlock( tag_type::INGROUP ) )
    return this->oContUnit.GetFirstTagBlock( tag_type::INGROUP )->GetPlainText();
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
/// \brief      Writes HTML output in the format that is default for pages.
// *****************************************************************************

void escrido::CDocPage::WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Brief (abstract):
  if( oContUnit.HasTagBlock( tag_type::BRIEF ) )
  {
    oOutStrm_i << "<div class=\"brief\">" << std::endl;
    oContUnit.GetFirstTagBlock( tag_type::BRIEF )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</div>" << std::endl;
  }

  //TEST
//    oContUnit.DebugOutput();

  // PARAGRAPH, SECTION and DETAILS paragraphs:
  oContUnit.WriteHTMLParSectDet( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes HTML output of the parameters set.
// *****************************************************************************

void escrido::CDocPage::WriteHTMLParameters( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Write parameter set:
  oContUnit.WriteHTMLParam( oOutStrm_i, oWriteInfo_i );
}

// .............................................................................

// *****************************************************************************
/// \brief      Writes LaTeX output in the format that is default for pages.
// *****************************************************************************

void escrido::CDocPage::WriteLaTeX( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Add title.
  oOutStrm_i << "\\section{" << LaTeXEscape( sTitle ) << "}%" << std::endl
             << "\\label{" << this->sIdent << "}%" << std::endl << std::endl;

  // PARAGRAPH, SECTION and DETAILS paragraphs:
  oContUnit.WriteLaTeXParSectDet( oOutStrm_i, oWriteInfo_i );
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

// .............................................................................

void escrido::CPageMainpage::WriteHTML( std::ostream& oOutStrm_i, const SWriteInfo& oWriteInfo_i ) const
{
  // Start description list.
  WriteHTMLTag( "<dl>", oOutStrm_i, oWriteInfo_i );
  ++oWriteInfo_i;

  //TEST
//   oContUnit.DebugOutput();

  // Author:
  if( oContUnit.HasTagBlock( tag_type::AUTHOR ) )
  {
    WriteHTMLTag( "<dt class=\"author\">Author</dt>", oOutStrm_i, oWriteInfo_i );
    WriteHTMLTag( "<dd>", oOutStrm_i, oWriteInfo_i );
    ++oWriteInfo_i;
    oContUnit.GetFirstTagBlock( tag_type::AUTHOR )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    WriteHTMLTag( "</dd>", oOutStrm_i, --oWriteInfo_i );
  }

  // Date:
  if( oContUnit.HasTagBlock( tag_type::DATE ) )
  {
    WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt class=\"date\">Date</dt><dd>";
    oContUnit.GetFirstTagBlock( tag_type::DATE )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</dd>" << std::endl;
  }

  // Version:
  if( oContUnit.HasTagBlock( tag_type::VERSION ) )
  {
    WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt class=\"version\">Version</dt><dd>";
    oContUnit.GetFirstTagBlock( tag_type::VERSION )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</dd>" << std::endl;
  }

  // Copyright:
  if( oContUnit.HasTagBlock( tag_type::COPYRIGHT ) )
  {
    WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<dt class=\"copyright\">Copyright</dt><dd>";
    oContUnit.GetFirstTagBlock( tag_type::COPYRIGHT )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</dd>" << std::endl;
  }

  // End description list.
  WriteHTMLTag( "</dl>", oOutStrm_i, --oWriteInfo_i );

  // Brief (abstract):
  if( oContUnit.HasTagBlock( tag_type::BRIEF ) )
  {
    oOutStrm_i << "<div class=\"brief\">" << std::endl;
    oContUnit.GetFirstTagBlock( tag_type::BRIEF )->WriteHTML( oOutStrm_i, oWriteInfo_i );
    oOutStrm_i << "</div>" << std::endl;
  }

  // PARAGRAPH, SECTION and DETAILS paragraphs:
  oContUnit.WriteHTMLParSectDet( oOutStrm_i, oWriteInfo_i );
}

// -----------------------------------------------------------------------------

// CLASS CUserTypedPage

// -----------------------------------------------------------------------------

escrido::CUserTypedPage::CUserTypedPage():
  CDocPage( "", "", "", headline_parse_state::START )
{}

// .............................................................................

void escrido::CUserTypedPage::AppendHeadlineChar( const char cIdentChar_i )
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

const std::string escrido::CUserTypedPage::GetURL( const std::string& sOutputPostfix_i ) const
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

void escrido::CUserTypedPage::BuildPageTypeID()
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
  if( sDocPageType == "_utype_" )
    pNewPage = new CUserTypedPage();

  // Otherwise give a warning and use a defaul documentation page.
  if( pNewPage == NULL )
  {
    std::cerr << "unrecognized page type '@" << szDocPageType_i << "' treated as '@_page_'" << std::endl;
    pNewPage = new CDocPage();
  }

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

void escrido::CDocumentation::WriteWebDoc( const std::string& sTemplateDir_i,
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
  {
    for( size_t p = 0; p < this->paDocPageList.size(); p++ )
      if( this->paDocPageList[p]->GetIdent() == "mainpage" )
      {
        pMainpage = static_cast<CPageMainpage*>( this->paDocPageList[p] );
        break;
      }
  }

  // Create index file.
  {
    // Try to read template.
    std::string sTemplateData;
    if( ReadTemp( sTemplateDir_i, "index.html", "default.html", sTemplateData ) )
    {
      // Replace placeholders.
      if( pMainpage == NULL )
      {
        ReplacePlaceholder( "*escrido-maintitle*", "Document Title", sTemplateData );
        ReplacePlaceholder( "*escrido-title*", "Document Title", sTemplateData );
        ReplacePlaceholder( "*escrido-mainpage*", "", sTemplateData );
      }
      else
      {
        ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplateData );
        ReplacePlaceholder( "*escrido-title*", pMainpage->GetTitle(), sTemplateData );
        ReplacePlaceholder( "*escrido-mainpage*", *pMainpage, &CDocPage::WriteHTML, oWriteInfo, sTemplateData );
      }
      ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteTableOfContentHTML, oWriteInfo, sTemplateData );

      // Save data.
      WriteOutput( sOutputDir_i + "index" + sOutputPostfix_i, sTemplateData );
    }
  }

  // Create all other pages.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    if( this->paDocPageList[p]->GetPageTypeID() != "mainpage" )
    {
      // Deduce template file name.
      std::string sTemplateFileName = this->paDocPageList[p]->GetPageTypeID() + ".html";

      // Try to read template.
      std::string sTemplateData;
      if( ReadTemp( sTemplateDir_i, sTemplateFileName, "default.html", sTemplateData ) )
      {
        // Replace placeholders.
        if( pMainpage == NULL )
        {
          ReplacePlaceholder( "*escrido-maintitle*", "Document Title", sTemplateData );
        }
        else
        {
          ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplateData );
        }

        ReplacePlaceholder( "*escrido-type*", GetCapForm( paDocPageList[p]->GetPageTypeLit() ), sTemplateData );
        ReplacePlaceholder( "*escrido-groupname*", paDocPageList[p]->GetGroupName(), sTemplateData );
        ReplacePlaceholder( "*escrido-title*", paDocPageList[p]->GetTitle(), sTemplateData );
        ReplacePlaceholder( "*escrido-page*", *paDocPageList[p], &CDocPage::WriteHTML, oWriteInfo, sTemplateData );
        ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteTableOfContentHTML, oWriteInfo, sTemplateData );
        ReplacePlaceholder( "*escrido-params*", *paDocPageList[p], &CDocPage::WriteHTMLParameters, oWriteInfo, sTemplateData );

        // Save data.
        WriteOutput( sOutputDir_i + this->paDocPageList[p]->GetURL( sOutputPostfix_i ), sTemplateData );
      }
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
    // Try to read template.
    std::string sTemplateData;
    if( ReadTemp( sTemplateDir_i, "latex.tex", "latex.tex", sTemplateData ) )
    {
      // Replace LaTeX settings placeholders.
      ReplacePlaceholder( "*escrido_latex_packages*", "\
% Math symbols:\n\
\\usepackage{amsmath}\n\
\\usepackage{amssymb}\n\
\\usepackage{amsfonts}\n\
% Additional symbols:\n\
\\usepackage{textcomp}\n\
% Package for graphics inclusion:\n\
\\usepackage{graphicx}\n\
% Extended color support:\n\
\\usepackage{xcolor}\n\
% Program code listings:\n\
\\usepackage{listings}\n\
% Auto-aligned tables:\n\
\\usepackage{tabulary}\n\
% Hyperlinks and hyper references:\n\
\\usepackage{hyperref}\n\
% Boxes with frames and colors:\n\
\\usepackage{framed}\n", sTemplateData );

//       ReplacePlaceholder( "*escrido_latex_settings*", "", sTemplateData );

      ReplacePlaceholder( "*escrido_latex_commands*", "\
% Maxwidth definition for images:\n\
\\makeatletter\n\
\\def\\maxwidth#1{\\ifdim\\Gin@nat@width>#1 #1\\else\\Gin@nat@width\\fi}\n\
\\makeatother\n\
\n\
% Verbatim blocks (@example, @output etc.):\n\
\\newcommand{\\verbatimtitle}[1]{\\noindent\\textbf{\\footnotesize #1}}\n\
\\newcommand{\\code}[1]{\\lstinline[breaklines=false,columns=fixed,basewidth=0.50em]^#1^}\n\
\n\
% @remark and @note type blocks:\n\
\\definecolor{notecolorhead}{HTML}{BCE77C}\n\
\\definecolor{notecolorbody}{HTML}{EFF9E1}\n\
\\newenvironment{note}\n\
{ \\colorlet{shadecolor}{notecolorbody}\n\
  \\begin{shaded*}\n\
  \\noindent\\colorbox{notecolorhead}{\\begin{minipage}{\\textwidth}\\textbf{Note:}\\end{minipage}} }\n\
{ \\end{shaded*} }\n\
\n\
\\definecolor{remarkcolorhead}{HTML}{FFFFCC}\n\
\\definecolor{remarkcolorbody}{HTML}{FFFFF5}\n\
\\newenvironment{remark}\n\
{ \\colorlet{shadecolor}{remarkcolorbody}\n\
  \\begin{shaded*}\n\
  \\noindent\\colorbox{remarkcolorhead}{\\begin{minipage}{\\textwidth}\\textbf{Remark:}\\end{minipage}} }\n\
{ \\end{shaded*} }\n", sTemplateData );

      // Replace mainpage placeholders.
      if( pMainpage != NULL )
      {
        ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplateData );

        if( pMainContentUnit->HasTagBlock( tag_type::AUTHOR ) )
          ReplacePlaceholder( "*escrido-mainauthor*", pMainContentUnit->GetFirstTagBlock( tag_type::AUTHOR )->GetPlainText(), sTemplateData );

        if( pMainContentUnit->HasTagBlock( tag_type::DATE ) )
          ReplacePlaceholder( "*escrido-maindate*", pMainContentUnit->GetFirstTagBlock( tag_type::DATE )->GetPlainText(), sTemplateData );

        if( pMainContentUnit->HasTagBlock( tag_type::BRIEF ) )
          ReplacePlaceholder( "*escrido-mainbrief*", pMainContentUnit->GetFirstTagBlock( tag_type::BRIEF )->GetPlainText(), sTemplateData );
      }

      // Create all other pages. Loop over groups:
      for( size_t g = 0; g < this->oaGroupList.size(); g++ )
      {
        // Expand "*escrido-pages*":
        ReplacePlaceholder( "*escrido-pages*", "\\chapter{*escrido-grouptitle*}\n*escrido-pages*", sTemplateData );

        // Insert group name as chapter title:
        if( oaGroupList[g].sGroupName.empty() )
          ReplacePlaceholder( "*escrido-grouptitle*", "Introduction", sTemplateData );
        else
          ReplacePlaceholder( "*escrido-grouptitle*", oaGroupList[g].sGroupName, sTemplateData );

        // Write pages.
        for( size_t p = 0; p < oaGroupList[g].naDocPageIdxList.size(); p++ )
        {
          // Expand "*escrido-pages*":
          ReplacePlaceholder( "*escrido-pages*", "*escrido-page*\n*escrido-pages*", sTemplateData );

          // Write page.
          ReplacePlaceholder( "*escrido-page*", *paDocPageList[oaGroupList[g].naDocPageIdxList[p]], &CDocPage::WriteLaTeX, oWriteInfo, sTemplateData );
        }
      }

      // Delete final "*escrido-pages*":
      ReplacePlaceholder( "*escrido-pages*", "", sTemplateData );

      // Save data.
      WriteOutput( sOutputDir_i + "latex.tex", sTemplateData );
    }
    else
      std::cerr << "Unable to read LaTeX template file 'latex.tex'." << std::endl;
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

  WriteHTMLTag( "<div id=\"toc\">", oOutStrm_i, oWriteInfo_i );
  ++oWriteInfo_i;
  WriteHTMLTag( "<h2>Contents</h2>", oOutStrm_i, oWriteInfo_i );

  // Loop over groups:
  for( size_t g = 0; g < this->oaGroupList.size(); g++ )
  {
    // If groups are used: show group name.
    if( this->oaGroupList.size() > 1 )
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h3>" << oaGroupList[g].sGroupName << "</h3>" << std::endl;

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

  WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
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
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h4>" << GetCapPluralForm( sPageTypeLit ) << "</h4>" << std::endl;

    WriteHTMLTag( "<ul>", oOutStrm_i, oWriteInfo_i );
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

    WriteHTMLTag( "</ul>", oOutStrm_i, --oWriteInfo_i );
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
    std::cout << "cannot load template file '" << sTemplateDir_i + sFileName_i << "'" << std::endl;

    // Cannot open template file. Try default file instead.
    oInFile.open( sTemplateDir_i + sFallbackFileName_i, std::ifstream::in | std::ifstream::binary );
    if( !oInFile.is_open() )
    {
      std::cerr << "cannot load template files '" << sTemplateDir_i + sFileName_i << "' or '" << sTemplateDir_i + sFallbackFileName_i << "' - skipping page" << std::endl;
      return false;
    }
    else
      std::cout << "using template file '" << sTemplateDir_i + sFallbackFileName_i << "'" << std::endl;
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
/// \brief      Changes a given placeholder in the to-be-modified string by a
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
/// \brief      Changes a given placeholder in the to-be-modified string by the
///             output of a given function.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oPage_i
///             Reference to the \ref CDocPage documentation page class
///             containing the generation method.
/// \param[in]  WriteMethodHTML
///             Pointer to the generation method (member of oPage_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethodHTML().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the return value of
///             WriteMethodHTML().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocPage& oPage_i,
                                  void (CDocPage::*WriteMethodHTML)( std::ostream&, const SWriteInfo& ) const,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  if( sTemplateData_io.find( szPlaceholder_i ) != std::string::npos )
  {
    // Create replacement string.
    std::stringstream sReplacement;
    ( oPage_i.*WriteMethodHTML )( sReplacement, oWriteInfo_i );

    // Replace all occurances of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
  }
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Changes a given placeholder in the to-be-modified string by the
///             output of a given function.
///
/// \param[in]  szPlaceholder_i
///             Placeholder C string, e.g. "*escrido-toc*"
/// \param[in]  oDocumentation_i
///             Reference to the \ref CDocumentation class containing the
///             generation method.
/// \param[in]  WriteMethodHTML
///             Pointer to the generation method (member of oDocumentation_i)
///             whose output is used to replace the respective placeholder
///             string.
/// \param[in]  oWriteInfo_i
///             Argument to WriteMethodHTML().
/// \param      sTemplateData_io
///             String that may contain the placeholder string. The placeholder
///             string will be replaced by the return value of
///             WriteMethodHTML().
// *****************************************************************************

void escrido::ReplacePlaceholder( const char* szPlaceholder_i,
                                  const CDocumentation& oDocumentation_i,
                                  void (CDocumentation::*WriteMethodHTML)( std::ostream&, const SWriteInfo& ) const,
                                  const SWriteInfo& oWriteInfo_i,
                                  std::string& sTemplateData_io )
{
  // Only proceed if the placeholder can be found in the string.
  if( sTemplateData_io.find( szPlaceholder_i ) != std::string::npos )
  {
    // Create replacement string.
    std::stringstream sReplacement;
    ( oDocumentation_i.*WriteMethodHTML )( sReplacement, oWriteInfo_i );

    // Replace all occurances of the placeholder by the replacement string.
    ReplacePlaceholder( szPlaceholder_i, sReplacement.str(), sTemplateData_io );
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
