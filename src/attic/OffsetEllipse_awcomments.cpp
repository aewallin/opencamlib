////////////////////////////////////////////////////////////////////////////////
// GroundSteel -- Low Level classes and functions used 
// by Computer Aided Manufacture Algorithms
// Written in 2004 by Julian Todd and Martin Dunschen.
//
// This file is Public Domain, no rights reserved, and 
// carries no warranty.  
////////////////////////////////////////////////////////////////////////////////

#include "groundsteel.h"
#include "OffsetEllipse.h"

// set this only to make a set of values that 
// we can optimize against.  
//#define TRACE_ELLIPSE_CALLS 10

#ifdef TRACE_ELLIPSE_CALLS
#include <stdio.h>
namespace GroundSteel
{
const char* strfindzeros = " FINDZEROS a= %.12lg %.12lg %.12lg %.12lg  b= %.12lg %.12lg %.12lg %.12lg  side= %d %d\n"; 
const char* strsetellipse = "SETELLIPSE ecen= %.12lg %.12lg j= %.12lg %.12lg n= %.12lg %.12lg nlen= %.12lg eccen= %.12lg"; 
int TECcallcount = 0; // used to pick out every tenth call 
};
#endif

namespace GroundSteel
{

#ifdef MDEBUG
//////////////////////////////////////////////////////////////////////
P2 Ellipse::D_EEval(double s, double t) const {
    TOL_ZERO(Square(s) + Square(t) - 1.0);  // check that s,t parameters are valid
    // ellipse-definition: ( j*cos(theta) , n*sin(theta) )
    // s=cos(theta)   t=sin(theta)
    return ecen + j * s + n * t;            // return a point on the ellipse
}

P2 Ellipse::D_Norm(double s, double t) const  {
    TOL_ZERO(Square(s) + Square(t) - 1.0);  // check that s,t parameters are valid
    // return normal to ellipse
    // ellipse: ( norm(j)*cos(theta) ,  norm(n)*sin(theta) )
    // normal:  ( norm(n)*sin(theta) , -norm(j)*cos(theta) )            exchange and flip one sign
    //  s=cos(theta)   t=sin(theta)
    // normal is  j*s/eccen + n*(t*eccen)
	return ZNorm(j * (s / eccen) + n * (t * eccen));   // what does Znorm() do? normalize to length 1?
}

P2 EllipseOffset::D_Eval(double s, double t) const {
    // return point on offset-ellipse = ellipse + normal*offrad
    return D_EEval(s, t) + D_Norm(s, t) * offrad; 
}
#endif


// for (s, t) where:          s^2 + t^2 = 1, 
// point of ellipse is:       ecen + j s + n t
// tangent at point is:       -j t + n s
// normal at point is:        j (s / eccen) + n (t * eccen)
// point on offset-ellipse:   point on ellipse + offrad*normal

// The code has been stabilized for very large values of eccen (long, thin, ellipses)
//////////////////////////////////////////////////////////////////////
void Ellipse::SetEllipseVals(const P2& lecen, const P2& lj, const P2& ln, double lnlen, double leccen)  
{
#ifdef TRACE_ELLIPSE_CALLS
    TECcallcount++; 
    if ((TECcallcount % TRACE_ELLIPSE_CALLS) == 0)
        fprintf(stderr, strsetellipse, lecen.u, lecen.v, lj.u, lj.v, ln.u, ln.v, lnlen, leccen); 
#endif
    // ellipse centre point 
    ecen = lecen; 
	// the major axis
    j = lj; 
    ASSERT(j.v >= 0.0); // axis is tilted upwards (wlog)
    // the minor axis 
    n = ln; 
    nlen = lnlen; 
    TOL_ZERO(n.Len() - nlen);  // check that nlen is valid
    ASSERT(n.u <= 0.0);        // ellipse always tilted so that n.u <= 0
    // the eccentricity (ratio of the two axes)
    ASSERT(leccen >= 1.0 - MDTOL_SMALL); 
    eccen = max(1.0, leccen); // eccentricity must be >= 1.0
    eccensq = Square(eccen);  
    IF_TOL_ZERO( (eccen < 1e10), eccen - j.Len() / nlen ); // more checking...
	// find parameters of points where the tangent is parallel to the v axis; 
	// these points also define the extents in u for the offset ellipse.  
	// solve for (j s / eccen + n t eccen).v = 0
	TOL_ZERO( fabs(j.u / eccen) - fabs(n.v) ); // j.u/eccen should be == j.v
	P2 stangv = ZNorm( P2(j.u, -n.u) );        // (s, t) of the position of the tangent 
	tang_s = -stangv.u;
	tang_t = stangv.v;
	TOL_ZERO(j.v * tang_s / eccen + n.v * tang_t * eccen); 
	TOL_ZERO(D_Norm(tang_s, tang_t).v); // check that tang-point has no v-component
	// other side is by negating stangv
	// solve for (j s / eccen + n t eccen).v = 0
	TOL_ZERO(j.v / eccen + n.u); 
	P2 sperpv = ZNorm(P2(j.v, n.v)); // (s, t) of the position of the tangent 
	perp_s = sperpv.u;
	perp_t = sperpv.v;
	TOL_ZERO(D_Norm(perp_s, perp_t).u); // check that perp-point has no u-component
}


//////////////////////////////////////////////////////////////////////
void EllipseOffset::SetOffsetRadius(double loffrad) {
// appends onto the end of the line 
#ifdef TRACE_ELLIPSE_CALLS
	if ((TECcallcount % TRACE_ELLIPSE_CALLS) == 0)
		fprintf(stderr, " offrad= %.12lg\n", loffrad); 
#endif
    // the offset distance 
    offrad = loffrad; 
    radrat = offrad / nlen;  // (radius ratio)
}


//////////////////////////////////////////////////////////////////////
// the ellipse has major axis (crad * jlenfac * veaxislen), and minor axis (crad)
// major axis is parallel to veaxis, which is used to generate the two axes
// the eccentricity (ration of axis lengths) is (jlenfac * veaxislen)
void EllipseOffset::SetOffsetEllipseVals(const P2& lecen,      // center of ellipse
                                         const P2& veaxis, 
                                         double veaxislen, 
                                         double crad,          // cutter radius
                                         double jlenfac,       
                                         double loffrad)       // offset radius
{
    ASSERT((veaxis.v > 0.0) || ((veaxis.v == 0.0) && (veaxis.u > 0.0))); 
    TOL_ZERO(veaxis.Len() - veaxislen); 
	SetEllipseVals( lecen,                                  // ellipse center
                    (veaxis * (crad * jlenfac)),            // major axis
                    (APerp(veaxis) * (crad / veaxislen)),   // minor axis
                    crad,                                   // length of minor axis == cutter radius
                    (jlenfac * veaxislen));                 // eccentricity
    SetOffsetRadius(loffrad); 
}





////////////////////////////////////////////////////////////////////////
#ifdef MDEBUG
bool EllipsOffsetPos::D_CheckVal(const EllipseOffset& eoff) {
    IF_TOL_ZERO((eoff.eccen < 1e6), (eoff.D_Eval(s, t) - p).Len() );  // p should be a point on the offset-ellipse at (s,t) (?)
    return true; 
}
#endif


////////////////////////////////////////////////////////////////////////
void EllipsOffsetPos::SetPosCardinal(const EllipseOffset& eoff, bool btangnorm, bool bgopos) 
{ // set ellipse-position (s,t) to one of the "cardinal" positions
	if (btangnorm) {
		P2 tdis = eoff.j * eoff.tang_s + eoff.n * eoff.tang_t - P2(eoff.offrad, 0.0); 
		if (bgopos) {
			s = eoff.tang_s; 
			t = eoff.tang_t; 
			p = eoff.ecen + tdis; 
		} else {
			s = -eoff.tang_s; 
			t = -eoff.tang_t; 
			p = eoff.ecen - tdis; 
		}
	} else {
		P2 pdis = eoff.j * eoff.perp_s + eoff.n * eoff.perp_t + P2(0.0, eoff.offrad); 
		if (bgopos) {
			s = eoff.perp_s; 
			t = eoff.perp_t; 
			p = eoff.ecen + pdis; 
		} else {
			s = -eoff.perp_s; 
			t = -eoff.perp_t; 
			p = eoff.ecen - pdis; 
		}
	}
	ASSERT(D_CheckVal(eoff));  
}


//////////////////////////////////////////////////////////////////////
void EllipsOffsetPos::SetPosGirth(const EllipseOffset& eoff, bool bnposgirth)  {
	s = 0.0; 
	t = (bnposgirth ? 1.0 : -1.0); 
    // start at center, move along minor axis to ellipse-edge and then to offset-ellipse
	p = eoff.ecen + eoff.n * (t * (1.0 + eoff.offrad / eoff.nlen)); 
	ASSERT(D_CheckVal(eoff));  // check that values are ok
}

//////////////////////////////////////////////////////////////////////
void EllipsOffsetPos::SetPos(const EllipseOffset& eoff, 
                             bool bst, 
                             bool bgopos, 
                             double lam, 
                             EllipsOffsetPos& eopa,    // position (a.s , a.t)
                             EllipsOffsetPos& eopb)    // position (b.s , b.t)
{
	// linear approximation 
	// we should get the evaluation to this type of thing by a controlled manner
	if ((eoff.eccen > 1e8) && ( (fabs(eopa.s - eopb.s) < 1e-8) || (fabs(eopa.t - eopb.t) < 1e-8) ) ) {
        // a special case with extreme eccentricity, 
        // and when eopa or eopb are close to eachother.
		s = AlongAcc(lam, eopa.s, eopb.s); 
		t = AlongAcc(lam, eopa.t, eopb.t); 
		k = 1.0 / sqrt(eoff.eccensq * Square(t) + Square(s)); 
		p = Along(lam, eopa.p, eopb.p); 
		ASSERT(D_CheckVal(eoff));  
		return; 
	}
	double ssq; 
	double tsq; 
	if (bst) { // a switch to select variable s or t
		s = AlongPushIn(lam, eopa.s, eopb.s); // calculate a new s-value somehow (?how)
		ssq = Square(s); 
		tsq = 1.0 - ssq; 
		t = sqrt(tsq) * (bgopos ? 1 : -1);     // the corresponding t-value
		ASSERT(I1::SCombine(eopa.t, eopb.t).Contains(t));  // ?check that the new t-value value is contained within [eopa.t , eopb.t]  ?
	} else {   // modify the t-value
		t = AlongPushIn(lam, eopa.t, eopb.t); 
		tsq = Square(t); 
		ssq = 1.0 - tsq;  
		s = sqrt(ssq) * (bgopos ? 1 : -1); // corresponding s-value 
		ASSERT(I1::SCombine(eopa.s, eopb.s).Contains(s)); // check
	}
	k = 1.0 / sqrt(eoff.eccensq * tsq + ssq); // ?k-value?
	double kf = k * eoff.radrat; 
    // calculation of point on the offset-ellipse?
	p = eoff.ecen + eoff.j * (s * (1.0 + kf / eoff.eccen)) + eoff.n * (t * (1.0 + kf * eoff.eccen)); 
	ASSERT(D_CheckVal(eoff));  
}


//////////////////////////////////////////////////////////////////////
double EllipsOffsetPos::dpds(const EllipseOffset& eoff, bool bgopos) {
	// k = (eccensq - (eccensq - 1) * ssq)^(-1/2)
	double dkds = k * k * k * s * (eoff.eccensq - 1.0); 
	double jfac = 1.0 + (eoff.radrat / eoff.eccen) * (k + s * dkds); 
	// dtds = -s/t
	double sbt = s / t; 
	double nfac = -sbt + (eoff.radrat * eoff.eccen) * (-sbt * k + t * dkds); 
	return eoff.j.u * jfac + eoff.n.u * nfac; 
}

//////////////////////////////////////////////////////////////////////
// Move in by alongs on the parameters 
bool EllipseOffset::EllIntersMonoByParap(EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp)  {
	double lam = -eopa.p.u / (eopb.p.u - eopa.p.u); 
	EllipsOffsetPos eops; 
	eops.SetPos(*this, true, bnp, lam, eopa, eopb); 
	bool bspos = ((eopa.s > 0.0) || ((eopa.s == 0.0) && (eopb.s > 0.0))); 
	EllipsOffsetPos eopt; 
	eopt.SetPos(*this, false, bspos, lam, eopa, eopb); 
	if (EqualOr(eops.s, eopa.s, eopb.s) && EqualOr(eopt.s, eopa.s, eopb.s))  {
		ASSERT(0); // limit of subdividing... 
		return false; 
	}
	// try and fit this pair in (don't know which order it should be)  
	bool bslt = (eops.p.u < eopt.p.u); 
	if (((bslt ? eops.p.u : eopt.p.u) < eopa.p.u) || ((bslt ? eopt.p.u : eops.p.u) > eopb.p.u)) {
		ASSERT(0); // outside the u-searching range 
		return false; 
	}
	if (((bslt ? eops.p.u : eopt.p.u) > 0.0) || ((bslt ? eopt.p.u : eops.p.u) < 0.0)) {
		//ASSERT(0); // outside the u-searching range 
		return false; 
	}
	if (bslt) {
		eopa = eops; 
		eopb = eopt; 
	} else {
		eopa = eopt; 
		eopb = eops; 
	}
	return true; 
}


//////////////////////////////////////////////////////////////////////
// Move in by half
bool EllipseOffset::EllIntersMonoByHalf(EllipsOffsetPos& eopa, bool bst, EllipsOffsetPos& eopb, bool bjp, bool bnp)  {
	EllipsOffsetPos eopd; 
	if (bst) {
		eopd.SetPos(*this, true, bnp, 0.5, eopa, eopb); 
		ASSERT(bnp ? ((eopa.t >= 0.0) && (eopb.t >= 0.0)) : ((eopa.t <= 0.0) && (eopb.t <= 0.0))); 
		if (EqualOr(eopd.s, eopa.s, eopb.s)) {
			ASSERT(0); // limit of subdividing... 
			return false; 
		}
	} else {
		bool bgopos = ((eopa.s >= 0.0) && (eopb.s >= 0.0)); 
		ASSERT(bgopos ? ((eopa.s >= 0.0) && (eopb.s >= 0.0)) : ((eopa.s <= 0.0) && (eopb.s <= 0.0))); 
		eopd.SetPos(*this, false, bgopos, 0.5, eopa, eopb); 
		if (EqualOr(eopd.t, eopa.t, eopb.t)) {
			ASSERT(0); // limit of subdividing... 
			return false; 
		}
	}
	if ((eopd.p.u < eopa.p.u) || (eopd.p.u > eopb.p.u)) {
		ASSERT(0); // outside the u-searching range 
		return false; 
	}
	if (eopd.p.u < 0.0) 
		eopa = eopd; 
	else
		eopb = eopd; 
	return true; 
}



//////////////////////////////////////////////////////////////////////
// these asserts are brittle, as I try to find how this geometry works.  

// Newton-Rhapson solver (?)
double EllipseOffset::EllIntersMonoByNR(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp)  {
// if there's any problem, phase in the function, use the slow version which does by binary subdivision.  
//return EllIntersMono(eopa, eopb, bjp, bnp);  
	double lam = -eopa.p.u / (eopb.p.u - eopa.p.u); 
	//EllipsOffsetPos eopm; 
	bool bspos = ((eopa.s > 0.0) || ((eopa.s == 0.0) && (eopb.s > 0.0))); 
	if (fabs(eopa.s - eopb.s) > fabs(eopa.t - eopb.t)) {
		eopm.SetPos(*this, true, bnp, lam, eopa, eopb); 
		ASSERT(I1::SCombine(eopa.t, eopb.t).Contains(eopm.t)); 
	} else {
		eopm.SetPos(*this, false, bspos, lam, eopa, eopb); 
		ASSERT(I1::SCombine(eopa.s, eopb.s).Contains(eopm.s)); 
	}
	// the loop searching for the best value 
	DEBUG_ONLY(int Diters = 0); 
	while (fabs(eopm.p.u) > 0.0000001) { // fixed value included (shouldn't be)
		ASSERT(I1(eopa.p.u, eopb.p.u).Contains(eopm.p.u)); 
		if (!I1::SCombine(eopa.s, eopb.s).ContainsStrict(eopm.s))  // bail out 
			return EllIntersMono(eopm, eopa, eopb, bjp, bnp);  
		if (eopm.p.u < 0.0) 
			eopa = eopm; 
		else
			eopb = eopm; 
		ASSERT(bspos == ((eopa.s > 0.0) || ((eopa.s == 0.0) && (eopb.s > 0.0)))); 
		// work in s 
		// calculates the derivative by ds at this position
		double vdpds = eopm.dpds(*this, bnp); 
		double dels = -eopm.p.u / vdpds; 
		double ns = eopm.s + dels; 
		// bail out 
		if (!I1::SCombine(eopa.s, eopb.s).ContainsStrict(ns)) 
			return EllIntersMono(eopm, eopa, eopb, bjp, bnp);  
		double lam = (ns - eopa.s) / (eopb.s - eopa.s); 
		TOL_ZERO(Along(lam, eopa.s, eopb.s) - ns); 
		double pushlam = I1(0.00001, 0.99999).PushInto(lam); 
		eopm.SetPos(*this, true, bnp, pushlam, eopa, eopb); 
		ASSERT(I1::SCombine(eopa.t, eopb.t).Contains(eopm.t)); 
		DEBUG_ONLY(Diters++); 
		ASSERT(Diters < 10000); 
	}
	return eopm.p.v;
}




//////////////////////////////////////////////////////////////////////
// do by subdivision for now 
double EllipseOffset::EllIntersMono(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp)  {
	ASSERT((eopa.p.u <= 0.0) && (eopb.p.u >= 0.0)); 
	bool bst = I1::SCombine(eopa.s, eopb.s).Overlaps(I1(-0.5, 0.5)); 
	// precision 
	DEBUG_ONLY(int niters = 0); 
	while (fabs(eopa.p.v - eopb.p.v) > 0.000001) {
//		if (!EllIntersMonoByParap(eopa, eopb, bjp, bnp)) 
		{
			if (!EllIntersMonoByHalf(eopa, bst, eopb, bjp, bnp)) 
				break; 
		}
		DEBUG_ONLY(niters++); 
	}
	eopm = eopa;
	return Half(eopa.p.v, eopb.p.v); 
}

//////////////////////////////////////////////////////////////////////
double EllipseOffset::EllInters(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp) 
{
#ifdef TRACE_ELLIPSE_CALLS
	if ((TECcallcount % TRACE_ELLIPSE_CALLS) == 0)
		printf(strfindzeros, eopa.s, eopa.t, eopa.p.u, eopa.p.v, eopb.s, eopb.t, eopb.p.u, eopb.p.v, bjp, bnp); 
#endif

	ASSERT((bjp == bnp) ? (eopa.s <= eopb.s) : (eopa.s >= eopb.s)); 
	ASSERT((eopa.p.u <= 0.0) && (eopb.p.u >= 0.0)); 

	// reduce interval at a point on the girth
	if (((eopa.s < 0.0) != (eopb.s < 0.0)) && (eopa.s != 0.0) && (eopb.s != 0.0)) 
	{
		EllipsOffsetPos eopg; 
		eopg.SetPosGirth(*this, bnp); 
		if (eopg.p.u < 0.0) 
			eopa = eopg; 
		else
			eopb = eopg; 
		ASSERT((bjp == bnp) ? (eopa.s <= eopb.s) : (eopa.s >= eopb.s)); 
	}
	ASSERT(((eopa.s < 0.0) == (eopb.s < 0.0)) || (eopa.s == 0.0) || (eopb.s == 0.0)); 
	ASSERT(((eopa.t < 0.0) == (eopb.t < 0.0)) || (eopa.t == 0.0) || (eopb.t == 0.0)); 

	// reduce interval at a point on the max or min to make the curve monotonic
	bool bmxmnup = ((perp_t > 0.0) == bnp); 
	double mxmns = (bmxmnup ? perp_s : -perp_s); 
	if (((eopa.s < mxmns) != (eopb.s < mxmns)) && (eopa.s != mxmns) && (eopb.s != mxmns)) 
	{
		EllipsOffsetPos eopm1; 
		eopm1.SetPosCardinal(*this, false, bmxmnup); 
		if (eopm1.p.u < 0.0) 
			eopa = eopm1; 
		else
			eopb = eopm1; 
		ASSERT((bjp == bnp) ? (eopa.s <= eopb.s) : (eopa.s >= eopb.s)); 

		ASSERT(((eopa.s < 0.0) == (eopb.s < 0.0)) || (eopa.s == 0.0) || (eopb.s == 0.0)); 
		ASSERT(((eopa.t < 0.0) == (eopb.t < 0.0)) || (eopa.t == 0.0) || (eopb.t == 0.0)); 
	}

	//ASSERT(bjp ? ((eopa.s >= 0.0) && (eopb.s >= 0.0)) : ((eopa.s <= 0.0) && (eopb.s <= 0.0))); 
	ASSERT(bnp ? ((eopa.t >= 0.0) && (eopb.t >= 0.0)) : ((eopa.t <= 0.0) && (eopb.t <= 0.0))); 

	// the non-offset ellipse case (for testing for now)  
	if (offrad == 0.0)
	{
		EllipsOffsetPos res; 
		ASSERT(0); 
//		res.SetPos(*this, 0.0, bjp, bnp);   // don't know what this call is all about; looks like it's setting s to 0
		ASSERT(I1::SCombine(eopa.s, eopb.s).Contains(res.s)); 
		ASSERT(I1::SCombine(eopa.p.v, eopb.p.v).Contains(res.p.v)); // proves it's monotonic
		return res.p.v; 
	}

	// we now know the result will be between eopa.p.v and eopb.p.v
	return EllIntersMonoByNR(eopm, eopa, eopb, bjp, bnp);   
	//return EllIntersMono(eopa, eopb, bjp, bnp); 
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*P2 AlignedEllipseOffset::D_eval(double s, double t)
{
	TOL_ZERO(Square(s) + Square(t) - 1.0); 
	P2 ep(j * s, n * t); 
	P2 nep(n * s, j * t); 
	return ep + ZNorm(nep) * offrad; 
}*/

//////////////////////////////////////////////////////////////////////
// an aligned ellipse problem where the ellipse is centred on 0, and we intersect it a u-line
// res.v = u * t / s 
// the geometry of the calculation is preserved throughout so we can see where the 
// quartic becomes ill-conditioned
// analytic solutions can be carried out in the original geometric setting.  
//////////////////////////////////////////////////////////////////////
AlignedEllipseOffset::AlignedEllipseOffset(double lj, double ln, double loffrad) :
	j(lj), n(ln), offrad(loffrad) {
	ASSERT(n <= j); 
	ASSERT(offrad >= 0.0); 
	jsq = Square(j); 
	nsq = Square(n); 
	osq = Square(offrad); 
	uprecision = 0.00000001; 
	aeoiterations = 0; 
}



//////////////////////////////////////////////////////////////////////
void AlignedEllipsOffsetPos::SetPos(const AlignedEllipseOffset& aeoff, double lst, bool bIsS)
{
	ASSERT((lst >= 0.0) && (lst < 1.0)); 
	if (bIsS)
	{
		s = lst; 
		ssq = Square(s); 
		tsq = 1 - ssq; 
		t = sqrt(tsq); 
	}
	else
	{
		t = lst; 
		tsq = Square(t); 
		ssq = 1.0 - tsq; 
		s = sqrt(ssq); 
	}
	P2 ep(aeoff.j * s, aeoff.n * t); 
	P2 nep(aeoff.n * s, aeoff.j * t); 

	double nepsq = (s < 0.5 ? aeoff.jsq + ssq * (aeoff.nsq - aeoff.jsq) : aeoff.nsq - tsq * (aeoff.nsq - aeoff.jsq)); 
	IF_TOL_ZERO(((s > 0.001) && (s < 0.999)), nepsq - (s >= 0.5 ? aeoff.jsq + ssq * (aeoff.nsq - aeoff.jsq) : aeoff.nsq - tsq * (aeoff.nsq - aeoff.jsq))); 
	double neplen = sqrt(nepsq); 
	TOL_ZERO(nep.Len() - neplen); 

	double pubs = aeoff.j + aeoff.n * aeoff.offrad / neplen; 
	double pvbt = aeoff.n + aeoff.j * aeoff.offrad / neplen; 
	p = P2(pubs * s, pvbt * t); 
	TOL_ZERO((ep + nep * (aeoff.offrad / neplen) - p).Len()); 
	l = pubs * t; 
	TOL_ZERO(l - (s == 0.0 ? (aeoff.j + aeoff.n * aeoff.offrad / aeoff.j) : p.u * t / s)); 

	// the derivative of u wrt ds

	// aeopm.p.u = s pubs = (1 - tsq)^0.5 pubs
	// pubs = j + n offrad nepsq^-0.5
	// nepsq = jsq + ssq (nsq - jsq)
	//		 = nsq - tsq (nsq - jsq)

	// d(nepsq)/ds = 2 s (nsq - jsq)
	// d(pubs)/ds = (-0.5) n offrad nepsq^-1.5 d(nepsq)/ds = -n s offrad (nsq - jsq) nepsq^-1.5
	// du/ds = pubs + s d(pubs)/ds = pubs - n ssq offrad (nsq - jsq) nepsq^-1.5
	double ssqfac = aeoff.n * aeoff.offrad * (aeoff.nsq - aeoff.jsq) / (nepsq * neplen); 
	dubds = pubs - ssq * ssqfac; 
	ASSERT(dubds >= 0.0); 

	// d(nepsq)/dt = -2 t (nsq - jsq)
	// d(pubs)/dt = (-0.5) n offrad nepsq^-1.5 d(nepsq)/ds = n t offrad (nsq - jsq) nepsq^-1.5
	// du/dt = -pubs t / s + s t n offrad (nsq - jsq) nepsq^-1.5
	dubdt = (s != 0.0 ? -pubs * t / s + s * t * ssqfac : 0.0); 
	ASSERT(dubdt <= 0.0); 
}

//////////////////////////////////////////////////////////////////////
void AlignedEllipsOffsetPos::SetPosFN(const AlignedEllipseOffset& aeoff, double hxsq, double hysq) 
{
	// APerp(ns, jt).(hx, hy) = 0
	// nsq * ssq * hysq = jsq * tsq * hxsq
	double hynsq = hysq * aeoff.nsq; 
	double hxjsq = hxsq * aeoff.jsq; 
	double htsq = hynsq / (hxjsq + hynsq); 
	if (hynsq < hxjsq)
	{
		double htsq = hynsq / (hxjsq + hynsq); 
		double ht = sqrt(htsq); 
		SetPos(aeoff, ht, false); 
	}
	else
	{
		double hssq = hxjsq / (hxjsq + hynsq); 
		double hs = sqrt(hssq); 
		SetPos(aeoff, hs, true); 
	}
	DEBUG_ONLY(P2 Dnep(aeoff.n * s, aeoff.j * t)); 
	DEBUG_ONLY(P2 Dh(sqrt(hxsq), sqrt(hysq))); 
	TOL_ZERO(Dot(APerp(ZNorm(Dnep)), ZNorm(Dh))); 
}



//////////////////////////////////////////////////////////////////////
void AlignedEllipsOffsetPos::SetPosHalfNR(const AlignedEllipseOffset& aeoff, AlignedEllipsOffsetPos& aeopa, AlignedEllipsOffsetPos& aeopb, double u) 
{
	bool bNRworked = false; 
	double umpu = u - p.u; 
	double ns = (dubds != 0.0 ? s + umpu / dubds : -1.0);  // the newton rhapson equation
	double nt = (dubdt != 0.0 ? t + umpu / dubdt : -1.0); 
	if ((dubds != 0.0) && (s < 0.6) && I1(aeopa.s, aeopb.s).Contains(ns))
	{
		SetPos(aeoff, ns, true); // re-sets the value of *this in its place
		bNRworked = (fabs(u - p.u) < fabs(umpu)); 
	}

	// t would be invalidated if the SetPos applied up there
	if (!bNRworked && (dubdt != 0.0) && I1(aeopb.t, aeopa.t).Contains(nt))
	{
		SetPos(aeoff, nt, false); // re-sets the value of *this in its place
		bNRworked = (fabs(u - p.u) < fabs(umpu)); 
	}

	// when the NR isn't getting there, help it along with some other subdivisions, and let it get back to it again.  
	if (!bNRworked || ((aeoff.aeoiterations >= 10) && ((aeoff.aeoiterations & 2) != 0)))
	{
		double lamr = I1(aeopa.p.u, aeopb.p.u).InvAlong(u); 
		if ((aeoff.aeoiterations % 1) != 0)
		{
			// set according to interpolated normal direction 
			P2 iv = P2(u, AlongAcc(lamr, aeopa.p.v, aeopb.p.v)) - P2(aeoff.j * AlongAcc(lamr, aeopa.s, aeopb.s), aeoff.n * Along(lamr, aeopa.t, aeopb.t)); 
			double hx = iv.u; 
			double hy = iv.v; 
			double hxsq = Square(hx); 
			double hysq = Square(hy); 
			SetPosFN(aeoff, hxsq, hysq);  
		}
		// set according to parameter interpolation
		else
		{
			double lam = (lamr * 7 + 0.5) / 8; // pull to centre
			if ((aeopa.s < 0.5) && (aeopb.s < 0.5)) 
				SetPos(aeoff, Along(lam, aeopa.s, aeopb.s), true); 
			else
				SetPos(aeoff, Along(lam, aeopa.t, aeopb.t), false); 
		}
	}

	// copy to the correct boundary
	if (p.u < u)
		aeopa = *this; 
	else
		aeopb = *this; 
}



//////////////////////////////////////////////////////////////////////
void AlignedEllipseOffset::LowerIntersectU(AlignedEllipsOffsetPos& aeopm, double u)  
{
	ASSERT((u >= 0.0) && (u <= j + offrad)); 
	if (u == 0.0)
		return aeopm.SetPos(*this, 0.0, true); // hitting square on
	double ohx = u - j; 
	if (ohx >= offrad)
		return aeopm.SetPos(*this, 0.0, false); // hitting tangential

	// calculate a lower bound if there is one
	AlignedEllipsOffsetPos aeop0, aeop1; 
	bool b0set = false; 
	bool b1set = false; 
	if (ohx > 0.0)
	{
		double ohxsq = Square(ohx); 
		double ohysq = osq - ohxsq; 
		ASSERT(ohysq >= 0.0); 
		aeopm.SetPosFN(*this, ohxsq, ohysq);  
		ASSERT(aeopm.p.u <= u + MDTOL_SMALL); 
		if (fabs(aeopm.p.u - u) < uprecision)  
			return; 
		aeop0 = aeopm; 
		b0set = true; 
	}

	// treat the whole offset ellipse as an ellipse
	double whe = (offrad + j) / (offrad + n); 
	double uhx = u / (offrad + j); 
	double uhxsq = Square(uhx); 
	double uhysq = 1.0 - uhxsq; 
	aeopm.SetPosFN(*this, uhxsq * Square(whe), uhysq);  
	if (fabs(aeopm.p.u - u) < uprecision)
		return; 
	if (aeopm.p.u > u)
	{
		aeop1 = aeopm; 
		b1set = true; 
	}
	else if (!b0set)
	{
		aeop0 = aeopm; 
		b0set = true; 
	}
	else if (aeopm.p.u > aeop0.p.u)
		aeop0 = aeopm; 

	// set a clear upper bound, by intersecting with unoffset ellipse
	if ((ohx < 0.0) && !b1set)
	{
		double nhe = j / n; 
		double nhx = u / j; 
		double nhxsq = Square(nhx); 
		double nhysq = 1.0 - nhxsq; 
		aeop1.SetPosFN(*this, nhxsq * Square(nhe), nhysq);  
		ASSERT(aeop1.p.u >= u); 
		b1set = true; 
	}

	// at least something is set; set the other end
	ASSERT(b0set || b1set); 
	if (!b0set)
	{
		aeop0.SetPos(*this, 0.0, true); 
		b0set = true; 
	}
	if (!b1set)
	{
		aeop1.SetPos(*this, 0.0, false); 
		b1set = true; 
	}
	ASSERT(b0set && b1set); 
	ASSERT(I1(aeop0.p.u, aeop1.p.u).Contains(u)); 
	ASSERT(EqualOr(aeopm.p.u, aeop0.p.u, aeop1.p.u)); // from at least one side
	if (aeopm.p.u == aeop0.p.u)
	{
		if ((aeop1.p.u != 1.0) && (fabs(aeop1.p.u - u) < fabs(aeop0.p.u - u)))
			aeopm = aeop1; 
	}
	else
	{
		ASSERT(aeopm.p.u == aeop1.p.u); 
		if ((aeop1.p.u == 1.0) || (fabs(aeop0.p.u - u) < fabs(aeop1.p.u - u)))
			aeopm = aeop0; 
	}
	ASSERT((aeop0.p.u <= u) && (aeop1.p.u >= u)); 

	// need to solve for s^2 + t^2 == 1	
	// P2 ep(j * s, n * t); 
	// P2 nep(n * s, j * t); 
	// u = ep.u + nep.u * offrad / nep.Len()

	// we can resolve this into solving for s or t 
	// depending on how close u is to 0
	// nep.Lensq() = j^2 + s^2 (n^2 - j^2)
	// u = j s + n s offrad / sqrt(j^2 + s^2 (n^2 - j^2))

	// do simple binary search for now
	// this has very bad convergence when n is small, and u > j
	while (fabs(aeopm.p.u - u) > uprecision)
	{
		aeopm.SetPosHalfNR(*this, aeop0, aeop1, u); 
//		if ((aeop1.s - aeop0.s < 1e-12) && (aeop0.t - aeop1.t < 1e-12))
//			break; 
		aeoiterations++; 
        if (aeoiterations > 1000)
        { EASSERT(0); return; }
	}

}


// supposing we were to make a quartic
	// pre-calculation of terms

	//double s = aeoff0.s; 
	//double nepsq = jsq + Square(s) * (nsq - jsq); 
	//double lhs = (u - j * s) * sqrt(nepsq); 
	//double rhs = n * s * offrad; 

	//double lhs = (usq - 2 * j * s * u + jsq * Square(s)) * (jsq + Square(s) * (nsq - jsq)); 
	//double rhs = nsq * Square(s) * osq; 
	//TOL_ZERO(lhs - rhs); 

	// cubic term is -2 * j * u * (nsq - jsq)
	// solving: 
	//   (usq - s 2 j u + s^2 jsq)) * (jsq + s^2 (nsq - jsq)) - s^2 nsq osq == 0
	
	// factor u out of the equation by setting s = q u
	//   (usq - q 2 j usq + q^2 usq jsq) (jsq + q^2 usq (nsq - jsq)) - q^2 usq nsq osq == 0
	//   (1 - q 2 j  + q^2 jsq) (jsq + q^2 usq (nsq - jsq)) - q^2 nsq osq == 0


	// this is the quartic that would have to be solved to do it not with NR
	//double usq = Square(u); 
	//double q = aeopm.s / u; 
	//double qsq = Square(q); 
	//TOL_ZERO((1.0 - q * 2 * j  + qsq * jsq) * (jsq + qsq * usq * (nsq - jsq)) - qsq * nsq * osq); 

	// divide out by the q^4 term, jsq usq (nsq - jsq)
	// this term becomes zero if we hit on the centre, or the toolpath is vertical
	//  (1 - q 2 j  + q^2 jsq) (jsq / (usq (nsq - jsq)) + q^2) - q^2 nsq osq / (usq (nsq - jsq))== 0

	// the q^3 term is -2 j


	// l = u t / s = u sqrt(1 - s^2)/s = sqrt((u/s)^2 - u^2)

	// need to calculate 
	// hk = P2(u, l).Len() - stockmodelcutflatrad; 

};
