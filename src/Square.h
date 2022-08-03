#ifndef SQUARE_H
#define SQUARE_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include "Triangle.h"

#include <cmath>

struct RaySquareIntersection {
	bool intersectionExists;
	float t;
	float u, v;
	unsigned int sIndex;
	Vec3 intersection;
	Vec3 normal;
};


class Square : public Mesh {
public:
	float width;
	float height;
	Square() : Mesh() {}

	void setQuad(Vec3 const& bottomLeft, Vec3 rightVector, Vec3 upVector, float width = 1., float height = 1.,
		float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f) {
		this->width = width;
		this->height = height;
		Vec3 normal = Vec3::cross(rightVector, upVector);
		normal.normalize();
		rightVector.normalize();
		rightVector.normalize();
		vertices.clear();
		vertices.resize(4);
		vertices[0].position = bottomLeft;                                           vertices[0].u = uMin; vertices[0].v = vMin;
		vertices[1].position = bottomLeft + width * rightVector;                     vertices[1].u = uMax; vertices[1].v = vMin;
		vertices[2].position = bottomLeft + width * rightVector + height * upVector; vertices[2].u = uMax; vertices[2].v = vMax;
		vertices[3].position = bottomLeft + height * upVector;                       vertices[3].u = uMin; vertices[3].v = vMax;
		vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = normal;
		triangles.clear();
		triangles.resize(2);
		triangles[0][0] = 0;
		triangles[0][1] = 1;
		triangles[0][2] = 2;
		triangles[1][0] = 0;
		triangles[1][1] = 2;
		triangles[1][2] = 3;
	}

	RaySquareIntersection intersect(const Ray& ray) const {
		RaySquareIntersection intersection;
		intersection.intersectionExists = false;
		RayTriangleIntersection temp = Triangle(vertices[0].position, vertices[1].position, vertices[2].position).getIntersection(ray);
		if(temp.intersectionExists){
			intersection.intersectionExists = true;
		} else {
			temp = Triangle(vertices[0].position, vertices[2].position, vertices[3].position).getIntersection(ray);
			if(temp.intersectionExists){
				intersection.intersectionExists = true;
			}
		}
		if(intersection.intersectionExists){
			intersection.intersection = temp.intersection;
			intersection.normal = temp.normal;
			intersection.t = temp.t;
			intersection.u = temp.w0;
			intersection.v = temp.w1;
		}
		return intersection;
	}
};
#endif // SQUARE_H
