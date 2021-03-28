// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL2) Package
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
#ifndef ROL2_LBFGS_DEF_H
#define ROL2_LBFGS_DEF_H

/** \class ROL2::Secant
    \brief Provides interface for and implements limited-memory secant operators.
*/

namespace ROL2 {

// Apply lBFGS Approximate Inverse Hessian
template<class Real>
void lBFGS<Real>::applyH(       Vector<Real>& Hv,
                          const Vector<Real>& v ) const {

  auto& state = Secant<Real>::getState();
  const Real zero(0);

  Hv.set(v.dual());
  std::vector<Real> alpha(state.current_+1,zero);

  for (int i = state.current_; i>=0; i--) {
    alpha[i]  = state.iterDiff_[i]->dot(Hv);
    alpha[i] /= state.product_[i];
    Hv.axpy(-alpha[i],(state.gradDiff_[i])->dual());
  }

  // Apply initial inverse Hessian approximation to v
  auto tmp = Hv.clone();
  Secant<Real>::applyH0(*tmp,Hv.dual());
  Hv.set(*tmp);

  Real beta(0);
  for (int i = 0; i <= state.current_; i++) {
    //beta  = Hv.dot((state.gradDiff_[i])->dual());
    beta  = Hv.apply(*state.gradDiff_[i]);
    beta /= state.product_[i];
    Hv.axpy((alpha[i]-beta),*(state.iterDiff_[i]));
  }
} // applyH

// Apply lBFGS Approximate Hessian
template<class Real>
void lBFGS<Real>::applyB(       Vector<Real>& Bv,
                          const Vector<Real>& v ) const {

  auto& state = Secant<Real>::getState();
  const Real one(1);

  // Apply initial Hessian approximation to v
  Secant<Real>::applyB0(Bv,v);

  std::vector<Ptr<Vector<Real>>> a(state.current_+1), b(state.current_+1);

  Real bv(0), av(0), bs(0), as(0);

  for (int i = 0; i <= state.current_; i++) {
    b[i] = Bv.clone();
    b[i]->set(*(state.gradDiff_[i]));
    b[i]->scale(one/sqrt(state.product_[i]));
    bv = v.apply(*b[i]);
    Bv.axpy(bv,*b[i]);

    a[i] = Bv.clone();
    Secant<Real>::applyB0(*a[i],*(state.iterDiff_[i]));

    for (int j = 0; j < i; j++) {
      bs = (state.iterDiff_[i])->apply(*b[j]);
      a[i]->axpy(bs,*b[j]);
      as = (state.iterDiff_[i])->apply(*a[j]);
      a[i]->axpy(-as,*a[j]);
    }
    as = (state.iterDiff_[i])->apply(*a[i]);
    a[i]->scale(one/sqrt(as));
    av = v.apply(*a[i]);
    Bv.axpy(-av,*a[i]);
  }
} // applyB0

} // namespace ROL2

#endif // ROL2_LBFGS_DEF_H
