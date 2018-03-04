// -----------------------------------------------------------------------------
/// \file       reftable.cpp
///
/// \brief      Module implementation file for a reference table used in
///             Escrido.
///
/// \author     Gunnar Schulze
/// \date       2015-10-13
/// \copyright  2015 trinckle 3D GmbH
// -----------------------------------------------------------------------------

#include "reftable.h"

#include <iostream>     // std::cout, std::cin, std::cerr, std::endl

// -----------------------------------------------------------------------------

// CLASS CRef

// -----------------------------------------------------------------------------

escrido::CRef::CRef( const std::string& sIdent_i,
                     const std::string& sLink_i,
                     const std::string& sName_i ) :
  sIdent ( sIdent_i ),
  sLink  ( sLink_i ),
  sName  ( sName_i )
{}

// -----------------------------------------------------------------------------

// CLASS CRefTable

// -----------------------------------------------------------------------------

escrido::CRefTable::CRefTable()
{}

// .............................................................................

void escrido::CRefTable::AppendRef( const std::string& sIdent_i,
                                    const std::string& sLink_i )
{
  oaRefList.emplace_back( sIdent_i, sLink_i, sIdent_i );
}

// .............................................................................

void escrido::CRefTable::AppendRef( const std::string& sIdent_i,
                                    const std::string& sLink_i,
                                    const std::string& sName_i )
{
  oaRefList.emplace_back( sIdent_i, sLink_i, sName_i );
}

// .............................................................................

// *****************************************************************************
/// @brief      Checks whether a reference with a given identifier exists and
///             retrieves its index.
///
/// @param[in]  sIdent_i
///             The reference label identifier that is searched for.
/// @param[out] nRefIdx_o
///             Returns the index of the reference, if it exists.
///
/// @return     'true' if the reference exists, 'false' otherwise.
// *****************************************************************************

bool escrido::CRefTable::GetRefIdx( const std::string& sIdent_i, size_t& nRefIdx_o ) const
{
  for( size_t r = 0; r < oaRefList.size(); r++ )
  {
    if( oaRefList[r].sIdent == sIdent_i )
    {
      nRefIdx_o = r;
      return true;
    }
  }

  return false;
}

// .............................................................................

std::string escrido::CRefTable::GetLink( size_t nRefIdx_i ) const
{
  if( nRefIdx_i < oaRefList.size() )
    return oaRefList[nRefIdx_i].sLink;
  else
    return std::string();
}

// .............................................................................

const std::string& escrido::CRefTable::GetName( size_t nRefIdx_i ) const
{
  if( nRefIdx_i < oaRefList.size() )
    return oaRefList[nRefIdx_i].sName;
  else
    return std::string();
}