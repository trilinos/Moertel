/*
#@HEADER
# ************************************************************************
#
#                          Moertel FE Package
#                 Copyright (2006) Sandia Corporation
#
# Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the U.S. Government.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# Questions? Contact Michael Gee (mwgee@sandia.gov)
#
# ************************************************************************
#@HEADER
*/
/* ******************************************************************** */
/* See the file COPYRIGHT for a complete copyright notice, contact      */
/* person and disclaimer.                                               */
/* ******************************************************************** */
#include "mrtr_overlap.H"
#include "mrtr_projector.H"
#include "mrtr_node.H"
#include "mrtr_pnode.H"
#include "mrtr_segment.H"
#include "mrtr_segment_bilineartri.H"
#include "mrtr_interface.H"
#include "mrtr_utils.H"

/*----------------------------------------------------------------------*
 |  copy a polygon of points (private)                       mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::CopyPointPolygon(map<int,RefCountPtr<MOERTEL::Point> >& from, map<int,RefCountPtr<MOERTEL::Point> >& to)
{
  map<int,RefCountPtr<MOERTEL::Point> >::iterator pcurr;
  for (pcurr=from.begin(); pcurr != from.end(); ++pcurr)
    if (pcurr->second != null)
    {
      RefCountPtr<MOERTEL::Point> tmp = rcp(new MOERTEL::Point(pcurr->second->Id(),pcurr->second->Xi(),pcurr->second->OutLevel()));
      to.insert(pair<int,RefCountPtr<MOERTEL::Point> >(tmp->Id(),tmp));
    }
  return true;
}

/*----------------------------------------------------------------------*
 |  find intersection (private)                              mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::Clip_Intersect(const double* N,const double* PE,const double* P0,const double* P1,double* xi)
{
  double P1P0[2];
  P1P0[0] = P1[0] - P0[0];
  P1P0[1] = P1[1] - P0[1];

  // determine the denominator first
  double denom = -(N[0]*P1P0[0] + N[1]*P1P0[1]);
  
  // if the denom is zero, then lines are parallel, no intersection
  if (fabs(denom)<1.0e-10)
    return false;
    
  double alpha = (N[0]*(P0[0]-PE[0]) + N[1]*(P0[1]-PE[1]))/denom;
  
  // alpha is the line parameter of the line P0 - p1
  // if it's outside 0 <= alpha <= 1 there is no intersection
  // cout << "OVERLAP Clip_Intersect: alpha " << alpha << endl;
  
  // Compute the coord xi of the intersecting point
  xi[0] = P0[0] + alpha*P1P0[0];
  xi[1] = P0[1] + alpha*P1P0[1];

  if (alpha<0.0 || 1.0<alpha)
    return false;
    
  // cout << "OVERLAP Clip_Intersect: found intersection xi " << xi[0] << "/" << xi[1] << endl;
  return true;
}

/*----------------------------------------------------------------------*
 |  test point (private)                                     mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::Clip_TestPoint(const double* N, const double* PE, 
                                   const double* P, double eps)
{
  double PPE[2];
  PPE[0] = P[0] - PE[0];
  PPE[1] = P[1] - PE[1];

  double dotproduct = PPE[0]*N[0]+PPE[1]*N[1];
  // cout << "OVERLAP Clip_TestPoint: dotproduct " << dotproduct << endl;

  if (dotproduct>eps)
    return false;
  else
    return true;
}

/*----------------------------------------------------------------------*
 |  find parameterization alpha for point on line (private)  mwgee 10/05|
 *----------------------------------------------------------------------*/
double MOERTEL::Overlap::Clip_ParameterPointOnLine(const double* P0,const double* P1,const double* P)
{
  double dist1 = sqrt( (P[0]-P0[0])*(P[0]-P0[0])+(P[1]-P0[1])*(P[1]-P0[1]) );
  double dist2 = sqrt( (P1[0]-P0[0])*(P1[0]-P0[0])+(P1[1]-P0[1])*(P1[1]-P0[1]) );
  if (dist2<1.0e-10) 
  {
    cout << "***ERR*** MOERTEL::Overlap::Clip_ParameterPointOnLine:\n"
         << "***ERR*** edge length too small, division by near zero\n"
         << "***ERR*** file/line: " << __FILE__ << "/" << __LINE__ << "\n";
    exit(EXIT_FAILURE);
  }
  return (dist1/dist2);
}

/*----------------------------------------------------------------------*
 |  add segment (private)                                    mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::AddSegment(int id, MOERTEL::Segment* seg)
{
  RefCountPtr<MOERTEL::Segment> tmp = rcp(seg);
  s_.insert(pair<int,RefCountPtr<MOERTEL::Segment> >(id,tmp));
  return true;
}

/*----------------------------------------------------------------------*
 |  add point (private)                                      mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::AddPointtoPolygon(const int id,const double* P)
{
  // check whether this point is already in there
  map<int,RefCountPtr<MOERTEL::Point> >::iterator curr = p_.find(id);
  // it's there
  if (curr != p_.end())
    curr->second->SetXi(P);
  else
  {
    //cout << "OVERLAP Clip_AddPointtoPolygon: added point " << id 
    //     << " xi=" << P[0] << "/" << P[1] << endl;
    RefCountPtr<MOERTEL::Point> p = rcp(new MOERTEL::Point(id,P,OutLevel()));
    p_.insert(pair<int,RefCountPtr<MOERTEL::Point> >(id,p));
  }
  return true;
}

/*----------------------------------------------------------------------*
 |  add point (private)                                      mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::AddPointtoPolygon(map<int,RefCountPtr<MOERTEL::Point> >& p,const int id,const double* P)
{
  RefCountPtr<MOERTEL::Point> point = rcp(new MOERTEL::Point(id,P,OutLevel()));
  p.insert(pair<int,RefCountPtr<MOERTEL::Point> >(id,point));
  return true;
}

/*----------------------------------------------------------------------*
 |  remove point (private)                                      mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::RemovePointfromPolygon(const int id,const double* P)
{
  // check whether this point is in there
  map<int,RefCountPtr<MOERTEL::Point> >::iterator curr = p_.find(id);
  if (curr != p_.end())
  {
    curr->second = null;
    p_.erase(id);
    // cout << "OVERLAP Clip_RemovePointfromPolygon: removed point " << id << endl;
    return true;
  }
  else
  {
    // cout << "OVERLAP Clip_RemovePointfromPolygon: do nothing\n";
    return false;
  }
}

/*----------------------------------------------------------------------*
 |  get point view (private)                                 mwgee 10/05|
 *----------------------------------------------------------------------*/
void MOERTEL::Overlap::PointView(vector<RefCountPtr<MOERTEL::Point> >& points)
{
  // allocate vector of ptrs
  points.resize(SizePointPolygon());
  
  // get the point views
  int count=0;
  map<int,RefCountPtr<MOERTEL::Point> >::iterator pcurr;
  for (pcurr=p_.begin(); pcurr != p_.end(); ++pcurr)
  {
    points[count] = pcurr->second;
    ++count;
  }
  if (count != SizePointPolygon())
  {
    cout << "***ERR*** MOERTEL::Overlap::PointView:\n"
         << "***ERR*** number of point wrong\n"
         << "***ERR*** file/line: " << __FILE__ << "/" << __LINE__ << "\n";
    exit(EXIT_FAILURE);
  }
  return;
}

/*----------------------------------------------------------------------*
 |  get segment view (protected)                             mwgee 11/05|
 *----------------------------------------------------------------------*/
void MOERTEL::Overlap::SegmentView(vector<RefCountPtr<MOERTEL::Segment> >& segs)
{
  // allocate vector of ptrs
  segs.resize(Nseg());
  
  // get the segment views
  int count=0;
  map<int,RefCountPtr<MOERTEL::Segment> >::iterator curr;
  for (curr=s_.begin(); curr != s_.end(); ++curr)
  {
    segs[count] = curr->second;
    ++count;
  }
  if (count != Nseg())
  {
    cout << "***ERR*** MOERTEL::Overlap::SegmentView:\n"
         << "***ERR*** number of segments wrong\n"
         << "***ERR*** file/line: " << __FILE__ << "/" << __LINE__ << "\n";
    exit(EXIT_FAILURE);
  }
  return;
}

/*----------------------------------------------------------------------*
 |  get point view (private)                                 mwgee 10/05|
 *----------------------------------------------------------------------*/
void MOERTEL::Overlap::PointView(map<int,RefCountPtr<MOERTEL::Point> >& p,
                                       vector<RefCountPtr<MOERTEL::Point> >& points)
{
  // allocate vector of ptrs
  int np = p.size();
  points.resize(np);
  
  // get the point views
  int count=0;
  map<int,RefCountPtr<MOERTEL::Point> >::iterator pcurr;
  for (pcurr=p.begin(); pcurr != p.end(); ++pcurr)
  {
    points[count] = pcurr->second;
    ++count;
  }
  if (count != np)
  {
    cout << "***ERR*** MOERTEL::Overlap::PointView:\n"
         << "***ERR*** number of point wrong\n"
         << "***ERR*** file/line: " << __FILE__ << "/" << __LINE__ << "\n";
    exit(EXIT_FAILURE);
  }
  return;
}

/*----------------------------------------------------------------------*
 |  get point view (private)                                 mwgee 11/05|
 *----------------------------------------------------------------------*/
void MOERTEL::Overlap::PointView(vector<MOERTEL::Point*>& p,const int* nodeids,const int np)
{
  p.resize(np);
  
  for (int i=0; i<np; ++i)
  {
    map<int,RefCountPtr<MOERTEL::Point> >::iterator pcurr = p_.find(nodeids[i]);
    if (pcurr==p_.end())
    {
      cout << "***ERR*** MOERTEL::Overlap::PointView:\n"
           << "***ERR*** cannot find point " << nodeids[i] << "\n"
           << "***ERR*** file/line: " << __FILE__ << "/" << __LINE__ << "\n";
      exit(EXIT_FAILURE);
    }
    p[i] = pcurr->second.get();
  }
  return;
}

/*----------------------------------------------------------------------*
 |  perform a quick search (private)                         mwgee 10/05|
 *----------------------------------------------------------------------*/
bool MOERTEL::Overlap::QuickOverlapTest()
{
  // we need the projection of the master element on the slave element 
  // to do this test, otherwise we assume we passed it
  if (!havemxi_) 
    return true;
  
  // find the max and min coords of master points
  double mmax[2]; 
  mmax[0] = mxi_[0][0];
  mmax[1] = mxi_[0][1];
  double mmin[2]; 
  mmin[0] = mxi_[0][0];
  mmin[1] = mxi_[0][1];
  for (int point=1; point<3; ++point)
    for (int dim=0; dim<2; ++dim)
    {
      if (mmax[dim] < mxi_[point][dim]) 
        mmax[dim] = mxi_[point][dim];
      if (mmin[dim] > mxi_[point][dim]) 
        mmin[dim] = mxi_[point][dim];
    }


  bool isin = false;
  // check xi0 direction
  if (  (0.0 <= mmin[0] && mmin[0] <= 1.0) ||
        (0.0 <= mmax[0] && mmax[0] <= 1.0) ||
        (mmin[0] <= 0.0 && 1.0 <= mmax[0])  )
  isin = true;
  
  if (!isin) 
  {
    //cout << "OVERLAP: " << sseg_.Id() << " and " << mseg_.Id() << " do NOT overlap in QuickOverlapTest\n";
    return false;
  }

  // check xi1 direction
  if (  (0.0 <= mmin[1] && mmin[1] <= 1.0) ||
        (0.0 <= mmax[1] && mmax[1] <= 1.0) ||
        (mmin[1] <= 0.0 && 1.0 <= mmax[1])  )
  isin = true;
  
  if (!isin)
  {
    //cout << "OVERLAP: " << sseg_.Id() << " and " << mseg_.Id() << " do NOT overlap in QuickOverlapTest\n";
    return false;
  }

  return true;
}
