#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

#include <GL/glut.h>
using namespace std;

enum LightType {
	LightType_Spherical,
	LightType_Quad
};


struct Light {
	Vec3 material;
	bool isInCamSpace;
	LightType type;

	Vec3 pos;
	float radius;
	int precisionSphere = 10;

	Square quad;
	int precisionQuadX = 5;
	int precisionQuadY = 5;


	float powerCorrection;
	vector<Vec3> calculatePoints(Vec3 normal) {
		vector<Vec3> res;
		Vec3 u(-normal[1], normal[0], 0);
		Vec3 v = Vec3::cross(normal, u);
		if (type == LightType::LightType_Spherical) {
			res = vector<Vec3>(precisionSphere);
			for (int i = 0; i < precisionSphere; i++) {
				//Lumiere par Sphere
				double theta = (((double)rand()) / RAND_MAX) * M_PI;
				double phi = (((double)rand()) / RAND_MAX) * 2.0 * M_PI;
				res[i][0] = pos[0] + (radius * sin(theta) * cos(phi));
				res[i][1] = pos[1] + (radius * sin(theta) * sin(phi));
				res[i][2] = pos[2] + (radius * cos(theta));
			}
		}
		else if (type == LightType::LightType_Quad) {
			res = vector<Vec3>(precisionQuadX * precisionQuadY);
			Vec3 bottomLeft = quad.vertices[0].position;
			float pasX = quad.width / precisionQuadX;
			float pasY = quad.height / precisionQuadY;

			for (int x = 0; x < precisionQuadX; x++) {
				for (int y = 0; y < precisionQuadY; y++) {
					res[x * precisionQuadY + y] = bottomLeft + (x * pasX * u) + (y * pasY * v);
				}
			}
		}
		return res;
	}

	Light() : powerCorrection(1.0) {}


};
enum typeOfIntersection {
	mesh,
	sphere,
	square
};
struct RaySceneIntersection {
	bool intersectionExists;
	typeOfIntersection typeOfIntersectedObject;
	unsigned int objectIndex;
	float t;
	RayTriangleIntersection rayMeshIntersection;
	RaySphereIntersection raySphereIntersection;
	RaySquareIntersection raySquareIntersection;
	RaySceneIntersection() : intersectionExists(false), t(FLT_MAX) {}
};



class Scene {
	std::vector< Mesh > meshes;
	std::vector< Sphere > spheres;
	std::vector< Square > squares;
	std::vector< Light > lights;

public:


	Scene() {
	}


	void draw() {
		// iterer sur l'ensemble des objets, et faire leur rendu :
		for (unsigned int It = 0; It < meshes.size(); ++It) {
			Mesh const& mesh = meshes[It];
			mesh.draw();
		}
		for (unsigned int It = 0; It < spheres.size(); ++It) {
			Sphere const& sphere = spheres[It];
			sphere.draw();
		}
		for (unsigned int It = 0; It < squares.size(); ++It) {
			Square const& square = squares[It];
			square.draw();
		}
	}

	RaySceneIntersection computeIntersection(Ray const& ray) {
		RaySceneIntersection result;
		size_t size = meshes.size();
		float t_min = FLT_MAX;
		for (size_t i = 0; i < size; i++) {
			RayTriangleIntersection temp = meshes[i].intersect(ray);
			if (temp.intersectionExists && temp.t < t_min) {
				t_min = temp.t;
				result.typeOfIntersectedObject = typeOfIntersection::mesh;
				result.intersectionExists = true;
				result.objectIndex = i;
				result.rayMeshIntersection = temp;
				result.t = temp.t;
			}
		}
		size = spheres.size();
		for (size_t i = 0; i < size; i++) {
			RaySphereIntersection temp = spheres[i].intersect(ray);
			if (temp.intersectionExists && temp.t < t_min) {
				t_min = temp.t;
				result.typeOfIntersectedObject = typeOfIntersection::sphere;
				result.intersectionExists = true;
				result.objectIndex = i;
				result.raySphereIntersection = temp;
				result.t = temp.t;
			}
		}
		size = squares.size();
		for (size_t i = 0; i < size; i++) {
			RaySquareIntersection temp = squares[i].intersect(ray);
			if (temp.intersectionExists && temp.t < t_min) {
				t_min = temp.t;
				result.t = temp.t;
				result.typeOfIntersectedObject = typeOfIntersection::square;
				result.intersectionExists = true;
				result.objectIndex = i;
				result.raySquareIntersection = temp;
				result.t = temp.t;
			}
		}
		return result;
	}

	Vec3 rayTraceRecursive(Ray ray, int NRemainingBounces) {

		Vec3 color = Vec3(0.5, 0, 0);
		RaySceneIntersection raySceneIntersection = computeIntersection(ray);

		if (raySceneIntersection.intersectionExists) {
			Mesh mesh;
			Material material;
			Vec3 intersection;
			Vec3 normal;
			bool isset = false;
			//Recupération des informations d'intersection.
			switch (raySceneIntersection.typeOfIntersectedObject) {
			case typeOfIntersection::mesh:
				mesh = meshes[raySceneIntersection.objectIndex];

				intersection = raySceneIntersection.rayMeshIntersection.intersection;
				normal = raySceneIntersection.rayMeshIntersection.normal;

				isset = true;
				break;
			case typeOfIntersection::sphere:
				mesh = spheres[raySceneIntersection.objectIndex];
				intersection = raySceneIntersection.raySphereIntersection.intersection;
				normal = raySceneIntersection.raySphereIntersection.normal;

				isset = true;
				break;
			case typeOfIntersection::square:
				mesh = squares[raySceneIntersection.objectIndex];
				intersection = raySceneIntersection.raySquareIntersection.intersection;
				normal = raySceneIntersection.raySquareIntersection.normal;

				isset = true;
				break;
			}
			if (isset) {
				material = mesh.material;

				//Ix = Intensité de la lumiére x réfléchie // Isx = Intensité de la lumière x // Kx = Coeff de reflexion x du matérau 
				//Phi = angle entre source lumineuse et normal. // Phi = 0° = maximal // Phi = 90 = minimal (lumière tangeante) // Phi > 90 : pas visible par la lumière
				Vec3 ambiantColor = Vec3(0, 0, 0); //Ia = Isa * Ka 
				Vec3 diffuseColor = Vec3(0, 0, 0); //Id = Isd * Kd * cos(Phi) 
				Vec3 specularColor = Vec3(0, 0, 0); //Is = Iss * Ks * cos(alpha^n)
				Vec3 mixColor = Vec3(0, 0, 0);
				Vec3 v = ray.origin() - intersection;
				v.normalize();

				//lumière spherique et plane
				for (int i = 0, max = lights.size(); i < max; i++) {
					Light l = lights[i];
					Vec3 lNorm = l.pos - intersection;
					lNorm.normalize();
					vector<Vec3> lightPoints = l.calculatePoints(lNorm);
					int reussiteShadow = 0.;
					int counterRightAngle = 0;
					Vec3 tmpDiffuseColor = Vec3(0, 0, 0);
					Vec3 tmpSpecularColor = Vec3(0, 0, 0);
					for (int j = 0, max = lightPoints.size(); j < max; j++) {
						Vec3 lightVector = lightPoints[j] - intersection;
						lightVector.normalize();

						RaySceneIntersection tmprsi = computeIntersection(Ray(intersection + (0.0001 * lightVector), lightVector));
						/*float minimal_transparency = 1.0f;
						if (tmprsi.t < lightVector.length()) {
							do {
								Material material_tmp;
								Vec3 intersection_tmp;
								switch (tmprsi.typeOfIntersectedObject) {
								case typeOfIntersection::mesh:
									material_tmp = meshes[raySceneIntersection.objectIndex].material;
									intersection_tmp = tmprsi.rayMeshIntersection.intersection;
									break;
								case typeOfIntersection::sphere:
									material_tmp = spheres[raySceneIntersection.objectIndex].material;
									intersection_tmp = tmprsi.raySphereIntersection.intersection;
									break;
								case typeOfIntersection::square:
									material_tmp = squares[raySceneIntersection.objectIndex].material;
									intersection_tmp = tmprsi.raySquareIntersection.intersection;
									break;
								}
								if (material_tmp.type == MaterialType::Material_Glass) {
									minimal_transparency = (material.transparency < minimal_transparency ? material.transparency : minimal_transparency);
									tmprsi = computeIntersection(Ray(intersection_tmp + (0.0001 * lightVector), lightVector));
									test = true;
								}
								else {
									minimal_transparency = 0.0f;
								}
							} while (minimal_transparency > 0.001f && tmprsi.t < lightVector.length());
						}*/
						if (tmprsi.t >= lightVector.length()) {
							reussiteShadow++;
							double angle = Vec3::dot(lightVector, normal);
							Vec3 r = (2 * angle * normal) - lightVector;
							//pour chaque réussite visible, ajoute une valeur a la couleur
							tmpDiffuseColor += Vec3::compProduct(l.material, material.diffuse_material) * angle;
							tmpSpecularColor += (Vec3::compProduct(l.material, material.specular_material) * abs(pow(Vec3::dot(r, v), material.shininess)));
						}
					}

					if (reussiteShadow > 0) {
						//normalise les couleurs selon le nombre de points de lumiere utilisés.
						diffuseColor += tmpDiffuseColor / lightPoints.size();
						specularColor += tmpSpecularColor / lightPoints.size();
					}
					ambiantColor += Vec3::compProduct(l.material, material.ambient_material);
				}
				color = ambiantColor + diffuseColor + specularColor;

				//Rajoute les effets de transparence et de reflexion.
				if (NRemainingBounces > 0) {
					bool isSpecial = false;
					float coefSpecial = 0.0f;
					Vec3 specialColor = Vec3();

					if (material.type == MaterialType::Material_Mirror) {
						isSpecial = true;
						coefSpecial = material.metalness;
						Vec3 rayDir = (2 * normal * Vec3::dot(normal, (-1.0 * ray.direction()))) + ray.direction();
						specialColor = rayTraceRecursive(Ray(intersection + (0.001 * rayDir), rayDir), NRemainingBounces - 1);
					}
					else if (material.type == MaterialType::Material_Glass) {
						isSpecial = true;
						coefSpecial = material.transparency;
						float dotNR = Vec3::dot(normal, ray.direction());
						float k = (material.index_medium * material.index_medium * ((dotNR * dotNR)));
						if (k >= 0) {
							Vec3 rayDir = (material.index_medium * ray.direction()) + ((material.index_medium * (dotNR + sqrt(k))) * normal);
							rayDir.normalize();
							specialColor = rayTraceRecursive(Ray(intersection + (0.001 * rayDir), rayDir), NRemainingBounces - 1);
						}
					}
					if (isSpecial) {
						color = ((1.0f - coefSpecial) * color) + (coefSpecial * specialColor);
					}
				}
			}
		}
		return color;
	}

	Vec3 rayTrace(Ray const& rayStart) {
		Vec3 color = rayTraceRecursive(rayStart, 10);
		return color;
	}

	void setup_double_sphere() {
		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();

		{
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(-5, 5, 5);
			light.radius = 2.5f;
			light.powerCorrection = 2.f;
			light.type = LightType_Spherical;
			light.material = Vec3(1, 1, 1);
			light.isInCamSpace = false;
		}
		{
			spheres.resize(spheres.size() + 1);
			Sphere& s = spheres[spheres.size() - 1];
			s.m_center = Vec3(0., 0., 0.);
			s.m_radius = 0.1f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.diffuse_material = Vec3(0., 0.5, 0);
			s.material.specular_material = Vec3(0.2, 0.2, 0.2);
			s.material.shininess = 1;
			s.material.transparency = 0.4;

		}
		{
			spheres.resize(spheres.size() + 1);
			Sphere& s = spheres[spheres.size() - 1];
			s.m_center = Vec3(1., 1., -1.);
			s.m_radius = 1.f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.diffuse_material = Vec3(0., 0., 0.5);
			s.material.specular_material = Vec3(0.2, 0.2, 0.2);
			s.material.shininess = 1;
		}
	}

	void setup_single_sphere() {
		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();

		{
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(-5, 5, 5);
			light.radius = 2.5f;
			light.powerCorrection = 2.f;
			light.type = LightType_Spherical;
			light.material = Vec3(1, 1, 1);
			light.isInCamSpace = false;
		}
		{
			spheres.resize(spheres.size() + 1);
			Sphere& s = spheres[spheres.size() - 1];
			s.m_center = Vec3(0., 0., 0.);
			s.m_radius = 1.f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.diffuse_material = Vec3(1., 1., 1);
			s.material.specular_material = Vec3(0.2, 0.2, 0.2);
			s.material.shininess = 1;
		}
	}

	void setup_single_square() {
		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();

		{
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(-5, 5, 5);
			light.radius = 2.5f;
			light.powerCorrection = 2.f;
			light.type = LightType_Spherical;
			light.material = Vec3(1, 1, 1);
			light.isInCamSpace = false;
		}

		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(0.8, 0.8, 0.8);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
	}
	void setup_cornellBox() {
		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();
		{
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(0, 0.7, 0);
			light.radius = 0.25f;
			light.powerCorrection = 2.0f;
			light.type = LightType_Spherical;
			light.precisionSphere = 10;
			light.material = Vec3(1, 1, 1);
			light.isInCamSpace = false;
		}
		/* {
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(0.4, 0.7, 0);
			light.radius = 0.25f;
			light.powerCorrection = 1.0f;
			light.type = LightType_Spherical;
			light.precisionSphere = 10;
			light.material = Vec3(1, 1, 0.5);
			light.isInCamSpace = false;
		}*/
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., -1.), Vec3(0., 1., 0.), Vec3(0., 0., 1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 0, 0);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(1., -1., 3.), Vec3(0., 1., 0.), Vec3(0., 0., -1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(0, 1, 0);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 3.), Vec3(1., 0., 0.), Vec3(0., 0., -1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., 1., 3.), Vec3(0., 0., -1.), Vec3(1., 0., 0.), 4., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., -1.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 3.), Vec3(0., 1., 0.), Vec3(1., 0., 0.), 2., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			spheres.resize(spheres.size() + 1);
			Sphere& s = spheres[spheres.size() - 1];
			s.m_center = Vec3(-0.2, -0.59, -0.2);
			s.m_radius = 0.4f;
			s.build_arrays();
			s.material.type = Material_Mirror;
			s.material.metalness = 1.0;
			s.material.ambient_material = Vec3(0.05, 0.05, 0.05);
			s.material.diffuse_material = Vec3(1, 1, 0);
			s.material.specular_material = Vec3(0.5, 0.5, 0.5);
			s.material.shininess = 10;
		}
		{
			spheres.resize(spheres.size() + 1);
			Sphere& s = spheres[spheres.size() - 1];
			s.m_center = Vec3(0.5, -0.69, 0.2);
			s.m_radius = 0.3f;
			s.build_arrays();
			s.material.type = Material_Glass;
			s.material.transparency = 1.0;
			s.material.index_medium = 1.3;
			s.material.ambient_material = Vec3(0.02, 0.04, 0.04);
			s.material.diffuse_material = Vec3(0.5, 0.5, 1);
			s.material.specular_material = Vec3(0.1, 0.1, 0.1);
			s.material.shininess = 10;

		}
	}

	void setup_cornellBoxMesh() {
		meshes.clear();
		spheres.clear();
		squares.clear();
		lights.clear();
		{
			lights.resize(lights.size() + 1);
			Light& light = lights[lights.size() - 1];
			light.pos = Vec3(0, 0.7, 0);
			light.radius = 0.25f;
			light.powerCorrection = 2.0f;
			light.type = LightType_Spherical;
			light.precisionSphere = 1;
			light.material = Vec3(1, 1, 1);
			light.isInCamSpace = false;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., -1.), Vec3(0., 1., 0.), Vec3(0., 0., 1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 0, 0);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(1., -1., 3.), Vec3(0., 1., 0.), Vec3(0., 0., -1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(0, 1, 0);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 3.), Vec3(1., 0., 0.), Vec3(0., 0., -1.), 2., 4.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., 1., 3.), Vec3(0., 0., -1.), Vec3(1., 0., 0.), 4., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., -1.), Vec3(1., 0., 0.), Vec3(0., 1., 0.), 2., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			squares.resize(squares.size() + 1);
			Square& s = squares[squares.size() - 1];
			s.setQuad(Vec3(-1., -1., 3.), Vec3(0., 1., 0.), Vec3(1., 0., 0.), 2., 2.);
			s.build_arrays();
			s.material.diffuse_material = Vec3(1, 1, 1);
			s.material.specular_material = Vec3(0.8, 0.8, 0.8);
			s.material.shininess = 1;
		}
		{
			meshes.resize(meshes.size() + 1);
			Mesh& m = meshes[meshes.size() - 1];
			m.loadOFF("maillages/avion_64.off");
			m.scale(Vec3(0.1, 0.1, 1.0));
			m.centerAndScaleToUnit();
			m.recomputeNormals();
			m.build_arrays();
			m.material.type = Material_Mirror;
			m.material.ambient_material = Vec3(0.02, 0.04, 0.04);
			m.material.diffuse_material = Vec3(0.5, 0.5, 1);
			m.material.specular_material = Vec3(0.1, 0.1, 0.1);
			m.material.shininess = 10;
			m.material.transparency = 0.4;
			m.material.index_medium = 1.3;
		}
	}
};

#endif
