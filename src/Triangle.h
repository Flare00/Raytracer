#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Vec3.h"
#include "Ray.h"
#include "Plane.h"

struct RayTriangleIntersection {
	bool intersectionExists;
	float t;
	float w0, w1, w2;
	unsigned int tIndex;
	Vec3 intersection;
	Vec3 normal;
};

class Triangle {
private:
	Vec3 m_c[3], m_normal;
	float area;
	Plane m_plane;
public:
	Triangle() {}
	Triangle(Vec3 const& c0, Vec3 const& c1, Vec3 const& c2) {
		m_c[0] = c0;
		m_c[1] = c1;
		m_c[2] = c2;
		updateAreaAndNormal();
		m_plane = Plane(m_c[0], m_normal);
	}
	void updateAreaAndNormal() {
		Vec3 nNotNormalized = Vec3::cross(m_c[1] - m_c[0], m_c[2] - m_c[0]);
		float norm = nNotNormalized.length();
		m_normal = nNotNormalized / norm;
		area = norm / 2.f;
	}
	void setC0(Vec3 const& c0) { m_c[0] = c0; } // remember to update the area and normal afterwards!
	void setC1(Vec3 const& c1) { m_c[1] = c1; } // remember to update the area and normal afterwards!
	void setC2(Vec3 const& c2) { m_c[2] = c2; } // remember to update the area and normal afterwards!
	Vec3 const& normal() const { return m_normal; }
	Vec3 projectOnSupportPlane(Vec3 const& p) const {
		/*Vec3 result;
		result = p - Vec3::dot(p - m_c[0], m_normal)*m_normal;
		return result;*/
		return m_plane.project(p);
	}
	float squareDistanceToSupportPlane(Vec3 const& p) const {
		return m_plane.squareDistance(p);
	}
	float distanceToSupportPlane(Vec3 const& p) const { return sqrt(squareDistanceToSupportPlane(p)); }
	/*float distanceToSupportPlane(Line const& l) const { 
		return - (Vec3::dot(m_normal, l.origin()) + Vec3::dot(m_normal, m_c[0])) / Vec3::dot(m_normal, l.direction()); 
	}*/

	bool isParallelTo(Line const& L) const {
		return m_plane.isParallelTo(L);
	}
	Vec3 getIntersectionPointWithSupportPlane(Line const& l) const {
		// you should check first that the line is not parallel to the plane!
		/*Vec3 result;
		result = l.origin() + distanceToSupportPlane(l.origin())* l.direction();
		return result;*/
		return m_plane.getIntersectionPoint(l);
	}

	void computeBarycentricCoordinates(Vec3 const& p, float& u0, float& u1, float& u2) const {
		//c2c0P = ||c0 - c2, P - c2||
		//c0c1P = ||c1 - c0, P - c0||
		//c1c2P = ||c2 - c1, P - c1||
		/*u0 = (Vec3::cross(m_c[0] - m_c[2], p - m_c[2]).length() / 2) / area;
		u1 = (Vec3::cross(m_c[1] - m_c[0], p - m_c[0]).length() / 2) / area;
		u2 = (Vec3::cross(m_c[2] - m_c[1], p - m_c[1]).length() / 2) / area;*/
		u0 = Vec3::dot(m_normal, Vec3::cross(m_c[1] - m_c[0], p - m_c[0]));
		u1 = Vec3::dot(m_normal, Vec3::cross(m_c[2] - m_c[1], p - m_c[1]));
		u2 = Vec3::dot(m_normal, Vec3::cross(m_c[0] - m_c[2], p - m_c[2]));
	}

	RayTriangleIntersection getIntersection(Ray const& ray) const {
		RayTriangleIntersection result;
		result.intersectionExists = false;
		// 1) check that the ray is not parallel to the triangle:
		if (!isParallelTo(ray)) {
			// 2) check that the triangle is "in front of" the ray:
			//float t =distanceToSupportPlane(ray.origin()) ;
			float t = Vec3::dot(m_c[0] - ray.origin(), m_normal) / Vec3::dot(ray.direction(), m_normal);
			if(t >= 0){
				Vec3 intersect = getIntersectionPointWithSupportPlane(ray);
				// 3) check that the intersection point is inside the triangle:
				// CONVENTION: compute u,v such that p = w0*c0 + w1*c1 + w2*c2, check that 0 <= w0,w1,w2 <= 1
				// w0 = p0/c0 
				float w0,w1,w2;
				computeBarycentricCoordinates(intersect, w0,w1,w2);
				//if(w0 >= 0 && w0 <= 1 && w1 >= 0 && w1 <= 1 && w2 >= 0 && w2 <= 1){
				if(w0 > 0 && w1 > 0 && w2 > 0){
					// 4) Finally, if all conditions were met, then there is an intersection! :
					result.intersectionExists =true;
					result.intersection = intersect;
					result.w0 = w0;
					result.w1 = w1;
					result.w2 = w2;
					result.t = t;
					result.normal = m_normal;
				}
			}
		}
		return result;
	}
};

#endif
