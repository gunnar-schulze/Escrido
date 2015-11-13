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

escrido::CRef::CRef( const std::string& sIdent_i, const std::string& sLink_i ) :
  sIdent ( sIdent_i ),
  sLink  ( sLink_i )
{}

// -----------------------------------------------------------------------------

// CLASS CRefTable

// -----------------------------------------------------------------------------

escrido::CRefTable::CRefTable()
{}

// .............................................................................

void escrido::CRefTable::AppendRef( const std::string& sIdent_i, const std::string& sLink_i )
{
  oaRefList.emplace_back( sIdent_i, sLink_i );
}

// .............................................................................

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