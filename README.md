# Raytracer
Raytracing project M1 Imagine

## Français

Compilation : 
- Linux : "make" ou "make linux".
- Windows x64 : "make w64".

Execution :
- Linux : ./main
- Windows x64 : ./main.exe

Commands : 
- R : rend la scene avec le Raytring dans le fichier render.ppm.
- + : change la scene parmis les 5 scenes actuellement présente (Sphere, Plan, Deux Spheres, Boite de Cornell avec deux sphères et une lampe, Boite de Cornelle avec un Mesh et une lampe)

Le projet implementes les 4 étapes du projets Raytracing.

Implementation :
- Rendu divisé en Thread. 
- Affichage de Sphere, Carré, Triangles et Mesh.
- Couleurs Ambiantes, Diffuses et Spéculaire.
- Ombre douces.
- Lumières Sphérique et Rectangulaire. (Avec résolution de la lumière)
- Matériaux Mirroir et Transparent avec gestion d'IOR.

## English

Compilation : 
- Linux : "make" or "make linux".
- Windows x64 : "make w64".

Execution :
- Linux : ./main
- Windows x64 : ./main.exe

Commands : 
- R : render the scene with raytracing in the render.ppm file.
- + : change the scene from 5 scene currently set up (Sphere, Plane, Two Sphere, Cornel Box with two Sphere and light, Cornell box With Mesh and light)

This implements all four of the steps of the Raytracing project.

Implementation :
- Threaded render.
- Show Spheres, Squares,Triangles and Global Mesh
- Ambiant, Diffuse and Specular Color.
- Soft Shadow.
- Spherical and Square Light. (With Light resolution)
- Mirror Material and Glass Material with IOR