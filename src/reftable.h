#ifndef REFTABLE_ALLREADY_READ_IN
#define REFTABLE_ALLREADY_READ_IN

// -----------------------------------------------------------------------------
/// \file       reftable.h
///
/// \brief      Module header for a reference table used in Escrido.
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

// -----------------------------------------------------------------------------

// CLASSES OVERVIEW

// -----------------------------------------------------------------------------

// *********************
// *                   *
// *        CRef       *
// *                   *
// *********************

// *********************
// *                   *
// *     CRefTable     *
// *                   *
// *********************

namespace escrido
{
  class CRef;
  class CRefTable;
}

// -----------------------------------------------------------------------------

// CLASS CRef

// -----------------------------------------------------------------------------

class escrido::CRef
{
  public:

    const std::string sIdent;   ///< Identifier label of the reference.
    const std::string sLink;    ///< URL to the referenced section.
    const std::string sText;    ///< Full text of the referenced element.

  public:

    CRef( const std::string& sIdent_i,
          const std::string& sLink_i,
          const std::string& sText_i );
};

// -----------------------------------------------------------------------------

// CLASS CRefTable

// -----------------------------------------------------------------------------

class escrido::CRefTable
{
  private:

    std::vector <CRef> oaRefList;

  public:

    // Constructor, destructor:
    CRefTable();

    // Access methods:
    void AppendRef( const std::string& sIdent_i,
                    const std::string& sLink_i );
    void AppendRef( const std::string& sIdent_i,
                    const std::string& sLink_i,
                    const std::string& sText_i );

    bool GetRefIdx( const std::string& sIdent_i, size_t& nRefIdx_o ) const;
    std::string GetLink( size_t nRefIdx_i ) const;
    const std::string& GetText( size_t nRefIdx_i ) const;
};

#endif /* REFTABLE_ALLREADY_READ_IN */
