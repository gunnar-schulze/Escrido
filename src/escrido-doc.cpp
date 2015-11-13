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
  fState    ( headline_parse_state::START )
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

const escrido::page_type escrido::CDocPage::GetPageType() const
{
  return page_type::PAGE;
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

void escrido::CDocPage::WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const
{
  // Title:
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h1 class=\"title\">" << sTitle << "</h1>"<< std::endl;

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


  // TODO HIER WEITERMACHEN
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
  std::cout << GetPageTypeStr( this->GetPageType() ) << ":" << std::endl;
  oContUnit.DebugOutput();
}

// -----------------------------------------------------------------------------

// CLASS CPageMainpage

// -----------------------------------------------------------------------------

escrido::CPageMainpage::CPageMainpage()
{
  // Set documentation page identifier to "mainpage".
  sIdent = "mainpage";
  fState = headline_parse_state::POST_IDENT;
}

// .............................................................................

const escrido::page_type escrido::CPageMainpage::GetPageType() const
{
  return page_type::MAINPAGE;
}

// .............................................................................

const std::string escrido::CPageMainpage::GetURL( const std::string& sOutputPostfix_i ) const
{
  return std::string( "index" + sOutputPostfix_i );
}

// .............................................................................

void escrido::CPageMainpage::WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const
{
  // Title:
  WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h1 class=\"title\">" << sTitle << "</h1>"<< std::endl;

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

// CLASS CPageFunc

// -----------------------------------------------------------------------------

escrido::CPageFunc::CPageFunc( const CParamList& oParamList_i ):
  oParamList  ( oParamList_i )
{}


// .............................................................................

const escrido::page_type escrido::CPageFunc::GetPageType() const
{
  return page_type::FUNCTION;
}

// .............................................................................

const std::string escrido::CPageFunc::GetURL( const std::string& sOutputPostfix_i ) const
{
  return std::string( "func_" + sIdent + sOutputPostfix_i );
}

// .............................................................................

void escrido::CPageFunc::WriteHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const
{
//   oOutStrm_i << "<div id=\"documentation\">" << std::endl
//              << "  <div id=\"type\">function</div>" << std::endl
//              << "  <h1><i>" << this->GetSignatureHTML() << "</i></h1>" << std::endl;
//   oContUnit.WriteHTML( oOutStrm_i, oWriteInfo_i );
//   oOutStrm_i << "</div>" << std::endl;
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
  SWriteHTMLInfo oWriteInfo;
  oWriteInfo.fShowInternal = fShowInternal_i;
  oWriteInfo.nIndent = 0;

  // Create a reference table.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    this->paDocPageList[p]->AddToRefTable( oWriteInfo.oRefTable, sOutputPostfix_i );

  // Search "mainpage".
  CPageMainpage* pMainpage;
  {
    pMainpage = NULL;
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
        ReplacePlaceholder( "*escrido-mainpage*", "", sTemplateData );
      }
      else
      {
        ReplacePlaceholder( "*escrido-maintitle*", pMainpage->GetTitle(), sTemplateData );
        ReplacePlaceholder( "*escrido-mainpage*", *pMainpage, &CDocPage::WriteHTML, oWriteInfo, sTemplateData );
      }
      ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteTableOfContentHTML, oWriteInfo, sTemplateData );

      // Save data.
      WriteOutput( sOutputDir_i + "index" + sOutputPostfix_i, sTemplateData );
    }
  }

  // Create all other pages.
  for( size_t p = 0; p < this->paDocPageList.size(); p++ )
    if( this->paDocPageList[p]->GetPageType() != page_type::MAINPAGE )
    {
      // Deduce template file name.
      std::string sTemplateFileName = GetPageTypeStr( this->paDocPageList[p]->GetPageType() ) + ".html";

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
        ReplacePlaceholder( "*escrido-title*", paDocPageList[p]->GetTitle(), sTemplateData );
        ReplacePlaceholder( "*escrido-page*", *paDocPageList[p], &CDocPage::WriteHTML, oWriteInfo, sTemplateData );
        ReplacePlaceholder( "*escrido-toc*", *this, &CDocumentation::WriteTableOfContentHTML, oWriteInfo, sTemplateData );

        // Save data.
        WriteOutput( sOutputDir_i + this->paDocPageList[p]->GetURL( sOutputPostfix_i ), sTemplateData );
      }
    }
}

// .............................................................................

void escrido::CDocumentation::DebugOutput() const
{
  for( size_t f = 0; f < this->paDocPageList.size(); f++ )
    this->paDocPageList[f]->DebugOutput();
}

// .............................................................................

void escrido::CDocumentation::WriteTableOfContentHTML( std::ostream& oOutStrm_i, const SWriteHTMLInfo& oWriteInfo_i ) const
{
  if( !fGroupOrdered )
    this->OrderByGroups();

  WriteHTMLTag( "<div id=\"toc\">", oOutStrm_i, oWriteInfo_i );
  ++oWriteInfo_i;
  WriteHTMLTag( "<h2>Contents</h2>", oOutStrm_i, oWriteInfo_i );

  // Loop over groups:
  for( size_t g = 0; g < this->oaGroupList.size(); g++ )
  {
    // If groups are used: show group name.
    if( this->oaGroupList.size() > 1 )
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h3>" << oaGroupList[g].sGroupName << "</h3>" << std::endl;

    // Write different page types.
    WriteTOCPageType( this->oaGroupList[g], page_type::MAINPAGE, oOutStrm_i, oWriteInfo_i );
    WriteTOCPageType( this->oaGroupList[g], page_type::PAGE, oOutStrm_i, oWriteInfo_i );

  }

  WriteHTMLTag( "</div>", oOutStrm_i, --oWriteInfo_i );
}

// .............................................................................

void escrido::CDocumentation::WriteTOCPageType( const CGroup& oGroup_i,
                                                const page_type& fPageType_i,
                                                std::ostream& oOutStrm_i,
                                                const SWriteHTMLInfo& oWriteInfo_i ) const
{
  // Count all pages of the group of the specific type.
  size_t nPagesN = 0;
  for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); p++ )
    if( paDocPageList[oGroup_i.naDocPageIdxList[p]]->GetPageType() == fPageType_i )
      nPagesN++;

  // Show list of pages.
  if( nPagesN > 0 )
  {
    // For all but PAGE and MAINPAGE: write a headline.
    if( fPageType_i != page_type::PAGE &&
        fPageType_i != page_type::MAINPAGE )
      WriteHTMLIndents( oOutStrm_i, oWriteInfo_i ) << "<h2>" << GetPageTypeStr( fPageType_i ) << "</h2>" << std::endl;

    WriteHTMLTag( "<ul>", oOutStrm_i, oWriteInfo_i );
    ++oWriteInfo_i;

    // Loop over all pages of the group:
    for( size_t p = 0; p < oGroup_i.naDocPageIdxList.size(); p++ )
    {
      CDocPage* pPage = paDocPageList[oGroup_i.naDocPageIdxList[p]];
      if( pPage->GetPageType() == fPageType_i )
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
/// \brief      Sorts all pages by groups.
// *****************************************************************************

void escrido::CDocumentation::OrderByGroups() const
{
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
    nReplPos = sTemplateData_io.find( szPlaceholder_i );
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
                                  void (CDocPage::*WriteMethodHTML)( std::ostream&, const SWriteHTMLInfo& ) const,
                                  const SWriteHTMLInfo& oWriteInfo_i,
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
                                  void (CDocumentation::*WriteMethodHTML)( std::ostream&, const SWriteHTMLInfo& ) const,
                                  const SWriteHTMLInfo& oWriteInfo_i,
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

const std::string escrido::GetPageTypeStr( const page_type& fPageType_i )
{
  switch( fPageType_i )
  {
    case page_type::PAGE:
      return std::string( "page" );

    case page_type::MAINPAGE:
      return std::string( "mainpage" );

    case page_type::FUNCTION:
      return std::string( "function" );
  }
}