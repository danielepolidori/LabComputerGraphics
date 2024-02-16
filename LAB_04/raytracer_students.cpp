#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"

// casts a single ray through the scene geometry and finds the closest hit
bool
RayTracer::CastRay (Ray & ray, Hit & h, bool use_sphere_patches) const
{
  bool answer = false;
  Hit nearest;
  nearest = Hit();

  // intersect each of the quads
  for (int i = 0; i < mesh->numQuadFaces (); i++)
  {
	Face *f = mesh->getFace (i);
	if (f->intersect (ray, h, args->intersect_backfacing))
	{
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	}
  }

  // intersect each of the spheres (either the patches, or the original spheres)
  if (use_sphere_patches)
  {
	for (int i = mesh->numQuadFaces (); i < mesh->numFaces (); i++)
	{
	  Face *f = mesh->getFace (i);
	  if (f->intersect (ray, h, args->intersect_backfacing))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }
  else
  {
	const vector < Sphere * >&spheres = mesh->getSpheres ();
	for (unsigned int i = 0; i < spheres.size (); i++)
	{
	  if (spheres[i]->intersect (ray, h))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }

  h = nearest;
  return answer;
}

Vec3f
RayTracer::TraceRay (Ray & ray, Hit & hit, int bounce_count) const
{

printf("\nBC: %i\n", bounce_count);
  hit = Hit ();
  bool intersect = CastRay (ray, hit, false);

  if( bounce_count == args->num_bounces )
  	RayTree::SetMainSegment (ray, 0, hit.getT () );
  else
	RayTree::AddReflectedSegment(ray, 0, hit.getT() );

  Vec3f answer = args->background_color;

  Material *m = hit.getMaterial ();
printf("inters: %i\n", intersect);
  if (intersect == true)
  {

	assert (m != NULL);
	Vec3f normal = hit.getNormal ();
	Vec3f point = ray.pointAtParameter (hit.getT ());

	// ----------------------------------------------
	// ambient light
	answer = args->ambient_light * m->getDiffuseColor ();


	// ----------------------------------------------
	// if the surface is shiny...
	Vec3f reflectiveColor = m->getReflectiveColor ();

cout << "reflColor: " << reflectiveColor;
	if(reflectiveColor.r() > 0 ||
	   reflectiveColor.g() > 0 ||
	   reflectiveColor.b() > 0) {   /// Se la superficie e' riflettente

		answer += reflectiveColor;
		///answer = answer * reflectiveColor;
	}

	// ==========================================
	// ASSIGNMENT:  ADD REFLECTIVE LOGIC
	// ==========================================
	
	// se (il punto sulla superficie e' riflettente & bounce_count>0)
	//     calcolare ReflectionRay  R=2<n,l>n -l
    //	   invocare TraceRay(ReflectionRay, hit,bounce_count-1)
	//     aggiungere ad answer il contributo riflesso
	if((reflectiveColor.r() > 0 ||
	    reflectiveColor.g() > 0 ||
	    reflectiveColor.b() > 0) &&
	    bounce_count > 0) {

printf("is_refl\n");
		/***
		 *   v --> direzione di `ray`
		 *   n --> `normal`
		 * r_v --> `dirReflection`
		 ***/

		///RayTree::AddShadowSegment(Ray(point, normal), 0, 50); /// PER VISUALIZZARE LA NORMALE AL PUNTO

		Vec3f v = ray.getDirection();
		v.Normalize();

		float prodottoScalareNV = normal.Dot3(v);   /// Prodotto scalare n*v

		Vec3f dirReflection = v - 2 * prodottoScalareNV * normal;   /// v opposite
		///Vec3f dirReflection = 2 * prodottoScalareNV * normal - v;
		dirReflection.Normalize();   /// Garantisce che il vettore abbia una lunghezza unitaria

		Ray rReflection = Ray(point, dirReflection);
		Hit hReflection; /// OPPURE BASTA USARE 'hit' ?

		///answer += reflectiveColor;
		answer += TraceRay(rReflection, hReflection, bounce_count-1);
		///answer += TraceRay(rReflection, hit, bounce_count-1);
	}	

	// ----------------------------------------------
	// add each light
	int num_lights = mesh->getLights ().size ();
	for (int i = 0; i < num_lights; i++)
	{
	  // ==========================================
	  // ASSIGNMENT:  ADD SHADOW LOGIC
	  // ==========================================
	  Face *f = mesh->getLights ()[i];
	  Vec3f pointOnLight = f->computeCentroid ();	/// Punto luce
	  Vec3f dirToLight = pointOnLight - point;	/// Direzione dal punto d'intersezione con l'oggetto al punto luce
	  dirToLight.Normalize ();

      // creare shadow ray verso il punto luce
		Ray rShadow = Ray(point, dirToLight);   /// Il raggio viene costruito passando l'origine e la direzione
		Hit hShadow = Hit();			/// Cio' che viene colpito dal raggio shadow (dal punto iniziale alla luce)
		bool colpitoShadow = false;
		colpitoShadow = CastRay(rShadow, hShadow, false);
		RayTree::AddShadowSegment(rShadow, 0, hShadow.getT());   /// Il raggio shadow viene mostrato in verde con 't'

	  // controllare il primo oggetto colpito da tale raggio

	  // se e' la sorgente luminosa i-esima allora
	  //	calcolare e aggiungere ad answer il contributo luminoso
	  // altrimenti
	  //    la luce i non contribuisce alla luminosita' di point.
		Vec3f puntoColpitoShadow = rShadow.pointAtParameter(hShadow.getT());   /// Punto colpito dallo shadow ray
		if (colpitoShadow && puntoColpitoShadow==pointOnLight) {   /// Se lo shadow ray colpisce la sorgente luminosa in considerazione

			  if (normal.Dot3 (dirToLight) > 0)
			  {
				Vec3f lightColor = 0.2 * f->getMaterial ()->getEmittedColor () * f->getArea ();
				answer += m->Shade (ray, hit, dirToLight, lightColor, args);
			  }
		}
	}
    
  }

  return answer;
}
