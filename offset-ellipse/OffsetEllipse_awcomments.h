////////////////////////////////////////////////////////////////////////////////
// GroundSteel -- Low Level classes and functions used 
// by Computer Aided Manufacture Algorithms
// Written in 2004 by Julian Todd and Martin Dunschen.
//
// This file is Public Domain, no rights reserved, and 
// carries no warranty.  
////////////////////////////////////////////////////////////////////////////////

#ifndef GS_OFFSETELLIPSE__h
#define GS_OFFSETELLIPSE__h

namespace GroundSteel
{

// forward declarations 
struct EllipsOffsetPos;
struct TruncEllipseOffset;


//////////////////////////////////////////////////////////////////////
// we gradually poke the values in until the functions can be called.  
struct Ellipse
{
	P2 ecen;	// ellipse centre
	P2 j;		// major axis
	P2 n;		// minor axis
	double nlen;	// length of n
	double eccen;	// eccentricity (length of j / nlen)
	double eccensq; // Square(eccensq)

	// the coordinates of the place where the ellipse is parallel to the y axis.  
	double tang_s; 
	double tang_t; 

	// the coordinates of the place where the ellipse is perp to the y axis
	double perp_s; 
	double perp_t; 

	void SetEllipseVals(const P2& lecen, const P2& lj, const P2& ln, double lnlen, double leccen); 

	#ifdef MDEBUG
		P2 D_EEval(double s, double t) const; 
		P2 D_Norm(double s, double t) const; 
	#endif
};


//////////////////////////////////////////////////////////////////////
// we gradually poke the values in until the functions can be called.  
struct EllipseOffset : Ellipse
{
	double offrad;	// offset radius we will apply 
	double radrat;  // offrad / nlen; 

	void SetOffsetRadius(double loffrad); // tiny function to make the reloading in the test harness easier.  

	// interface used by the edge torus function
	void SetOffsetEllipseVals(const P2& lecen, const P2& veaxis, double veaxislen, double crad, double jlenfac, double loffrad); 

	#ifdef MDEBUG
		P2 D_Eval(double s, double t) const; 
	#endif

	// solves for points (positions) on the offset ellipse where eopm.p.u == 0
	double EllInters(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp); 
	double EllIntersMono(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp); 
	bool EllIntersMonoByHalf(EllipsOffsetPos& eopa, bool bst, EllipsOffsetPos& eopb, bool bjp, bool bnp); 
	bool EllIntersMonoByParap(EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp); 
	double EllIntersMonoByNR(EllipsOffsetPos& eopm, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb, bool bjp, bool bnp); 
}; 



//////////////////////////////////////////////////////////////////////
struct EllipsOffsetPos
{
	double s; 
	double t; 
	double k; // 1.0 / sqrt(eoff.eccensq * tsq + ssq); 
	P2 p; 

	#ifdef MDEBUG
		bool D_CheckVal(const EllipseOffset& eoff); 
	#endif

// doing these by constructors rather than create function presents to problem that 
// we can't give them distinctive names, so we hope we get to the right one.  

	// generating positions at the cardinal points of tangency and so forth.  
	void SetPosCardinal(const EllipseOffset& eoff, bool btangnorm, bool bgopos); 
	void SetPosGirth(const EllipseOffset& eoff, bool bnposgirth); 

	// generates the points are the truncated sides 
	void SetPosFromTruncatedCorner(const TruncEllipseOffset& teoff, bool boffup, bool bsidepn); 



	// generates position for s value here that's a midpoint between the two here.  
	void SetPos(const EllipseOffset& eoff, bool bst, bool bgopos, double lam, EllipsOffsetPos& eopa, EllipsOffsetPos& eopb); 

	// calculates the derivative by ds at this position
	double dpds(const EllipseOffset& eoff, bool bgopos); 
}; 



//////////////////////////////////////////////////////////////////////
struct TruncEllipseOffset : EllipseOffset
{
	// the truncated values 
	double s0; 
	double t0; 
	double s1; 
	double t1; 

	bool SetS01(double ls0, double ls1); 

	// corners of the truncations (or of the ends of the ellipse if the truncation is none).  
	double upoffrat; 
	P2 offupflatmid; // the flat part in the middle of the s1
	P2 offupflat0; 
	P2 offupflat1; 

	double downoffrat; 
	P2 offdownflatmid; // the flat part in the middle of the s0
	P2 offdownflat0; 
	P2 offdownflat1; 

	// signals for which way the flats face, and whether the extend is defined by a corner or a tangency to one side of the ellipse.  
	int npellup; // lower side (adding n positively)
	int nmellup; // upper side

	// corresponding points of the tangencies
	P2 npinfx;  // valid only if npellup == 0, coords are tang_s, tang_t
	P2 nminfx; 

	void SetTruncateCorners(const P3& a, const P3& b, const P3& v, double veaxissq, double tipcenz); 
	bool HitsExtentsZero(); // this is the hitting the x=0 line 

	// solving for the position eopm.p.u == 0, wherein it returns eopm.p.v
	double UpperIntersectZero(EllipsOffsetPos& eopm, int& ionfaceupper);  // this is the hitting the x=0 line 
	double LowerIntersectZero(EllipsOffsetPos& eopm, int& ionfacelower);  // this is the hitting the x=0 line 
}; 



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
struct AlignedEllipseOffset; // forward declaration

struct AlignedEllipsOffsetPos
{
	double s; 
	double ssq; 
	double t; 
	double tsq; 

	double l; 
	P2 p; 
	double dubds; 
	double dubdt; 

	void SetPosFN(const AlignedEllipseOffset& aeoff, double hxsq, double hysq); // set by normal being parallel to this pair of squares of a vector
	void SetPos(const AlignedEllipseOffset& aeoff, double lst, bool bIsS); 
	void SetPosHalfNR(const AlignedEllipseOffset& aeoff, AlignedEllipsOffsetPos& aeopa, AlignedEllipsOffsetPos& aeopb, double u); 
}; 


//////////////////////////////////////////////////////////////////////
struct AlignedEllipseOffset
{
	double j; // major radius (along u)
	double n; // minor radius (along v)
	double offrad; 
	double jsq; 
	double nsq; 
	double osq; 
	double uprecision; 
	int aeoiterations; 

	AlignedEllipseOffset(double lj, double ln, double loffrad); 
	void LowerIntersectU(AlignedEllipsOffsetPos& aeopm, double u); 
}; 

}; // end namespace

#endif
