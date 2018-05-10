#if !defined(ZIKEN_MATHLIB2D_INCLUDE_)
#define ZIKEN_MATHLIB2D_INCLUDE_
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <cmath>

namespace ziken
{


inline bool _valid(const void* const p)
{
    return p ? true : false;
};

inline bool _invalid(const void* const p)
{
    return p ? false : true;
};

//最大值
#if !defined(__max)
template <typename T>
inline const T& __max(const T& a, const T& b)
{
    return a < b ? b : a;
}
#endif

//最小值
#if !defined(__min)
template <typename T>
inline const T& __min(const T& a, const T& b)
{
    return a < b ? b : a;
}
#endif

//平方
#if !defined(__sqr)
template <typename T>
inline const T __sqr(const T& a)
{
    return a*a;
}
#endif

#if !defined(PI)
const double PI = 3.1415927;
#endif

#if !defined(PI_RAD)
const double PI_RAD = 0.01745329252;    //PI in radian
#endif

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

inline bool equal(double x, double y)
{
    unsigned __int64 *px = (unsigned __int64*)&x;
    unsigned __int64 *py = (unsigned __int64*)&y;

    unsigned __int64 d = (px[0] | py[0]);
    d = d>>56;
    if((d & 127) == 0) //x, y均为绝对值很小的数
    {
        return true;
    }
    unsigned __int64 X = px[0]>>20;
    unsigned __int64 Y = py[0]>>20;
    if(X == Y) return true;
    if(X < Y)
    {
        return X + 1 == Y;
    }
    else
    {
        return Y + 1 == X;
    }
    //可以加入判断低位的语句
//    return true;
}

inline bool dinky(const double a)
{
    return equal(a, 0.0f);
}

struct point_d
{
    double x;
    double y;

    point_d() {}
    point_d(double _x, double _y): x(_x), y(_y) {}

    const bool check_identical(const point_d& pnt)
    {
        if (equal(pnt.x, x) && equal(pnt.y, y))
            return true;
        else
            return false;
    }

    void output() const {std::cout << x << ", " << y;}

    //Returns the norm of this vector.
    double norm() const {return sqrt(x*x + y*y);}
     
    // Print point
    friend std::ostream& operator << ( std::ostream& s, const point_d& p )  {
      s << p.x << " " << p.y;
      return s;
    }
};

struct edge_d
{
    point_d v1;
    point_d v2;
};

struct rect_d
{
    double min_x;
    double max_x;
    double min_y;
    double max_y;

    double extent_x()
    {
        return max_x - min_x;
    }

    double extent_y()
    {
        return max_y - min_y;
    }
};

struct circle_d
{
    double r;
    point_d c;

    circle_d() {}
    circle_d(double _r, point_d& _c): r(_r), c(_c) {}
    circle_d(double _r, double _x, double _y): r(_r), c(_x, _y) {}

/*
 * Find the intersection point(s) of two circles,
 * when their centers and radiuses are given (2D).
 */
    size_t intersect(const circle_d& C2, point_d& i1, point_d& i2) {
        // distance between the centers
        double d = point_d(c.x - C2.c.x, 
                c.y - C2.c.y).norm();
         
        // find number of solutions
        if(d > r + C2.r) // circles are too far apart, no solution(s)
        {
            std::cout << "circle_ds are too far apart\n";
            return 0;
        }
        else if(d == 0 && r == C2.r) // circles coincide
        {
            std::cout << "circle_ds coincide\n";
            return 0;
        }
        // one circle contains the other
        else if(d + min(r, C2.r) < max(r, C2.r))
        {
            std::cout << "One circle contains the other\n";
            return 0;
        }
        else
        {
            double a = (r*r - C2.r*C2.r + d*d)/ (2.0*d);
            double h = sqrt(r*r - a*a);
             
            // find p2
            point_d p2( c.x + (a * (C2.c.x - c.x)) / d,
                    c.y + (a * (C2.c.y - c.y)) / d);
             
            // find intersection points p3
            i1.x = p2.x + (h * (C2.c.y - c.y)/ d);
            i1.y = p2.y - (h * (C2.c.x - c.x)/ d);

            i2.x = p2.x - (h * (C2.c.y - c.y)/ d);
            i2.y = p2.y + (h * (C2.c.x - c.x)/ d);
             
            if(d == r + C2.r)
            {
                return 1;
            }

            return 2;
        }
    }
     
    // Print circle
    friend std::ostream& operator << ( std::ostream& s, const circle_d& C )
    {
      s << "Center: " << C.c << ", r = " << C.r;
      return s;
    }
};

int intersect(const circle_d &circle0, const circle_d &circle1,
    point_d &intersection0, point_d &intersection1)
{
    circle_d c0(circle0.r, circle0.c.x, circle0.c.y);
    circle_d c1(circle1.r, circle1.c.x, circle1.c.y);
    point_d i0, i1;
    int num = c0.intersect(c1, i0, i1);
    intersection0.x = i0.x;
    intersection0.y = i0.y;
    intersection1.x = i1.x;
    intersection1.y = i1.y;
    return num;
}

// return positive  -   the three points are in ccw order
double integral_area(point_d &a, point_d &b, point_d &c)
{
	return ((b.x - a.x)*(c.y - b.y) - (c.x - b.x)*(b.y - a.y));
}

// return < 0   -   pnt is inside the circle abc
// return = 0   -   pnt is on the circle abc
// return > 0   -   pnt is outside the circle abc
double within_circle1(point_d &pnt, point_d &a, point_d &b, point_d &c)
{
	double apx = a.x - pnt.x;
	double apy = a.y - pnt.y;
	double bpx = b.x - pnt.x;
	double bpy = b.y - pnt.y;
	double cpx = c.x - pnt.x;
	double cpy = c.y - pnt.y;

	double bpxcpy = bpx * cpy;
	double cpxbpy = cpx * bpy;
	double alift = apx * apx + apy * apy;

	double cpxapy = cpx * apy;
	double apxcpy = apx * cpy;
	double blift = bpx * bpx + bpy * bpy;

	double apxbpy = apx * bpy;
	double bpxapy = bpx * apy;
	double clift = cpx * cpx + cpy * cpy;

	double det = alift * (bpxcpy - cpxbpy)
		+ blift * (cpxapy - apxcpy)
		+ clift * (apxbpy - bpxapy);

	return -det;
}

void calc_circle(const point_d &pnt_a, const point_d &pnt_b,
    const point_d &pnt_c, point_d &center, double &r)
{
    double x1 = pnt_a.x;
    double y1 = pnt_a.y;
    double x2 = pnt_b.x;
    double y2 = pnt_b.y;
    double x3 = pnt_c.x;
    double y3 = pnt_c.y;

    double a=2*(x2-x1);
    double b=2*(y2-y1);
    double c=x2*x2+y2*y2-x1*x1-y1*y1;
    double d=2*(x3-x2);
    double e=2*(y3-y2);
    double f=x3*x3+y3*y3-x2*x2-y2*y2;

    center.x = (b*f-e*c)/(b*d-e*a);
    center.y = (d*c-a*f)/(b*d-e*a);
    r = sqrt((center.x-x1)*(center.x-x1)
        +(center.y-y1)*(center.y-y1));
}

bool within_circle(const point_d &pnt, const point_d &pnt_a,
    const point_d &pnt_b, const point_d &pnt_c)
{
    point_d center;
    double r;
    calc_circle(pnt_a, pnt_b, pnt_c, center, r);
    double xx = (pnt.x - center.x)*(pnt.x - center.x);
    double yy = (pnt.y - center.y)*(pnt.y - center.y);
    double dist = sqrt(xx + yy);
    if (dist < r)
        return true;

    return false;
}

bool in_range(point_d &pnt, point_d &a, point_d &b)
{
    double min_x = min(a.x, b.x);
    double max_x = max(a.x, b.x);
    double min_y = min(a.y, b.y);
    double max_y = max(a.y, b.y);

    if (pnt.x >= min_x && pnt.x <= max_x)
    {
        if (pnt.y >= min_y && pnt.y <= max_y)
        {
            return true;
        }
    }

    return false;
}

// see if the vertex is inside or outside the triangle,
// or lies on a edge of the triangle
// be sure that v1 v2 v3 in t are in ccw direction
//
// return:
//  1   -   v is inside of t
//  0   -   v lies on one of the edges
//  -1  -   v is one of the vertices of t
//  -2  -   v is outside of t
int within_triangle(point_d &v, point_d &v1, point_d &v2,
    point_d &v3)
{
    if (v.check_identical(v1)
        || v.check_identical(v3)
        || v.check_identical(v3))
    {
        // v is just one of the vertices of t
        return -1;
    }

    // compute the three integral areas formed by v and each edge
    // following ccw direction
	double area1 = integral_area(v1, v2, v);
	double area2 = integral_area(v2, v3, v);
	double area3 = integral_area(v3, v1, v);

	if (area1>0 && area2>0 && area3>0)
	{
        // v is absolutely inside of t
		return 1;
	}
	else if (dinky(area1*area2*area3))
    {
        // v might lie on one edge
        if (dinky(area1))
        {
            if (in_range(v, v1, v2))
            {
                return 0;
            }
        }
        else if (dinky(area2))
        {
            if (in_range(v, v2, v3))
            {
                return 0;
            }
        }
        else if (dinky(area3))
        {
            if (in_range(v, v3, v1))
            {
                return 0;
            }
        }
    }

    // v is outside of t
	return -2;
}

}
#endif