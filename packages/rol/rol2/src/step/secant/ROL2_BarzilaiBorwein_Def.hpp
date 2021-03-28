// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote product_s derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#pragma once 
#ifndef ROL2_BARZILAIBORWEIN_DEF_HPP
#define ROL2_BARZILAIBORWEIN_DEF_HPP


namespace ROL2 {

template<class Real>
void BarzilaiBorwein<Real>::applyH(       Vector<Real>& Hv,
                                    const Vector<Real>& v ) const {

  // Get Generic Secant State
  auto& state = Secant<Real>::getState();

  Hv.set(v.dual());

  if( state.iter_ != 0 && state.current_ != -1 ) {
    if ( type_ == 1 ) {
      Real yy = state.gradDiff_[state.current_]->dot(*(state.gradDiff_[state.current_]));
      Hv.scale(state.product_[state.current_]/yy);
    }
    else if( type_ == 2 ) {
      Real ss = state.iterDiff_[state.current_]->dot(*(state.iterDiff_[state.current_]));
      Hv.scale(ss/state.product_[state.current_]);
    }
  }
} // BarzilaiBorwein<Real>::applyH


template<class Real>
void BarzilaiBorwein<Real>::applyB(       Vector<Real>& Bv,
                                    const Vector<Real>& v ) const {
 // Get Generic Secant State
 auto& state = Secant<Real>::getState();

 Bv.set(v.dual());

 if ( state.iter_ != 0 && state.current_ != -1 ) {
   if ( type_ == 1 ) {
     Real yy = state.gradDiff_[state.current_]->dot(*(state.gradDiff_[state.current_]));
     Bv.scale(yy/state.product_[state.current_]);
   }
   else if ( type_ == 2 ) {
     Real ss = state.iterDiff_[state.current_]->dot(*(state.iterDiff_[state.current_]));
     Bv.scale(state.product_[state.current_]/ss);
   }
 }
} // BarzilaiBorwein<Real>::applyB

} // namespace ROL2

#endif // ROL2_BARZILAIBORWEIN_DEF_HPP
