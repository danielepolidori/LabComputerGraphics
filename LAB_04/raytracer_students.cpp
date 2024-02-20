#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"

#include <cmath>   /// Per togliere i puntini neri delle shadow

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

	// ==========================================
	// ASSIGNMENT:  ADD REFLECTIVE LOGIC
	// ==========================================
	
	// se (il punto sulla superficie e' riflettente & bounce_count>0)
	//     calcolare ReflectionRay  R=2<n,l>n -l
    //	   invocare TraceRay(ReflectionRay, hit,bounce_count-1)
	//     aggiungere ad answer il contributo riflesso
	if((reflectiveColor.r() > 0 ||
	    reflectiveColor.g() > 0 ||
	    reflectiveColor.b() > 0) &&   /// Se la superficie e' riflettente
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
		dirReflection.Normalize();   /// Garantisce che il vettore abbia una lunghezza unitaria

		Ray rReflection = Ray(point, dirReflection);
		Hit hReflection;

		Vec3f contributoRiflesso = TraceRay(rReflection, hReflection, bounce_count-1);
cout << "contrib_TraceRay: " << contributoRiflesso;


		/// Seleziono il contributo da applicare, in base alla sfera colpita (se rossa o bianca)
		if(reflectiveColor.r() > 0 &&
	  	   reflectiveColor.g() == 0 &&
	  	   reflectiveColor.b() == 0) {   	/// Se e' stata colpita la superficie riflettente della sfera ROSSA

			/// La media e' alta per colori chiari, bassa per colori scuri
			float mediaContributoRiflesso = (contributoRiflesso.r() + contributoRiflesso.g() + contributoRiflesso.b()) / 3;

			/// In questo modo, se la superficie riflessa e' chiara allora il rosso e' acceso, se e' scura allora il rosso e' scuro
			answer += mediaContributoRiflesso * reflectiveColor;
		}
		else if(reflectiveColor.r() > 0 &&
		 	reflectiveColor.g() > 0 &&
		 	reflectiveColor.b() > 0) {	/// Se e' stata colpita la superficie riflettente della sfera BIANCA

			/// Considero solamente il colore del riflesso (senza la componente bianca della sfera, i.e. reflectiveColor)
			answer += contributoRiflesso;
		}
	}	

	// ----------------------------------------------
	// add each light
	int num_lights = mesh->getLights().size();

	///if(! args->softShadow) {	/// Shadow (non soft)

		for (int i = 0; i < num_lights; i++)
		{

			if(args->softShadow) {	/// Soft shadow

				Face *f = mesh->getLights()[i];

				int numShadowRay = 64;		/// Numero di raggi shadow che partono dal punto colpito
				int numShadowRay2Light = 0;	/// Numero di raggi shadow che dal punto colpiscono la sorgente luminosa
				float gradoSoftShadow = 1.0;	/// Grado d'intensita' dell'ombra (in base al numero di raggi che colpiscono la luce da quel punto)

				/// Creazione shadow ray
				for(int j = 0; j < numShadowRay; j++) {

					Vec3f pointOnLight = f->RandomPoint();   	/// Punto casuale della (faccia della) sorgente luminosa
					Vec3f dirToLight = pointOnLight - point;	/// Direzione dal punto d'intersezione con l'oggetto al punto luce
					dirToLight.Normalize();

					Ray rShadow = Ray(point, dirToLight);   /// Il raggio viene costruito passando l'origine e la direzione
					Hit hShadow = Hit();			/// Cio' che viene colpito dal raggio shadow (dal punto iniziale alla luce)
					bool colpitoShadow = false;
					colpitoShadow = CastRay(rShadow, hShadow, false);
					RayTree::AddShadowSegment(rShadow, 0, hShadow.getT());   /// Il raggio shadow viene mostrato in verde con 't'

					Vec3f puntoColpitoShadow = rShadow.pointAtParameter(hShadow.getT());   /// Punto colpito dallo shadow ray
					float intorno = 1.0e-05;
					if (colpitoShadow &&
						puntoColpitoShadow.x() < pointOnLight.x() + intorno &&
						puntoColpitoShadow.x() > pointOnLight.x() - intorno &&
						puntoColpitoShadow.y() < pointOnLight.y() + intorno &&
						puntoColpitoShadow.y() > pointOnLight.y() - intorno &&
						puntoColpitoShadow.z() < pointOnLight.z() + intorno &&
						puntoColpitoShadow.z() > pointOnLight.z() - intorno) {   /// Se lo shadow ray colpisce la sorgente luminosa in considerazione

						if (normal.Dot3(dirToLight) > 0) {

							numShadowRay2Light++;
						}
					}
				}

				float incremento = 1.0 / numShadowRay;
				if(numShadowRay2Light < numShadowRay) {   /// Se siamo in ombra (o penombra)

					/// Piu' shadow ray arrivano alla luce, piu' quel punto sara' luminoso
					gradoSoftShadow = numShadowRay2Light * incremento;
				}


				/// Calcolo il contributo della luce

				Vec3f pointOnLight = f->computeCentroid();	/// Punto luce
				Vec3f dirToLight = pointOnLight - point;	/// Direzione dal punto d'intersezione con l'oggetto al punto luce
		  		dirToLight.Normalize();

				if(normal.Dot3(dirToLight) > 0) {

					Vec3f lightColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
					answer += gradoSoftShadow * m->Shade(ray, hit, dirToLight, lightColor, args);
				}
			}
			else {

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

			float intorno = 1.0e-05;
			if (colpitoShadow &&
				puntoColpitoShadow.x() < pointOnLight.x() + intorno &&
				puntoColpitoShadow.x() > pointOnLight.x() - intorno &&
				puntoColpitoShadow.y() < pointOnLight.y() + intorno &&
				puntoColpitoShadow.y() > pointOnLight.y() - intorno &&
				puntoColpitoShadow.z() < pointOnLight.z() + intorno &&
				puntoColpitoShadow.z() > pointOnLight.z() - intorno) {   /// Se lo shadow ray colpisce la sorgente luminosa in considerazione

				  if (normal.Dot3 (dirToLight) > 0)
				  {
					Vec3f lightColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
					answer += m->Shade(ray, hit, dirToLight, lightColor, args);
				  }
			}
		}} /// fine else non soft
	/*}
	else {				/// Soft shadow

		for(int i = 0; i < num_lights; i++) {

			Face *f = mesh->getLights()[i];
			///Vec3f pointOnLight = f->computeCentroid ();	/// Punto luce
			Vec3f dirToLight = pointOnLight - point;	/// Direzione dal punto d'intersezione con l'oggetto al punto luce
			dirToLight.Normalize();

			Ray rShadow = Ray(point, dirToLight);   /// Il raggio viene costruito passando l'origine e la direzione
			Hit hShadow = Hit();			/// Cio' che viene colpito dal raggio shadow (dal punto iniziale alla luce)
			bool colpitoShadow = false;
			colpitoShadow = CastRay(rShadow, hShadow, false);
			RayTree::AddShadowSegment(rShadow, 0, hShadow.getT());   /// Il raggio shadow viene mostrato in verde con 't'

			Vec3f puntoColpitoShadow = rShadow.pointAtParameter(hShadow.getT());   /// Punto colpito dallo shadow ray
			float intorno = 1.0e-05;
			if (colpitoShadow &&
				puntoColpitoShadow.x() < pointOnLight.x() + intorno &&
				puntoColpitoShadow.x() > pointOnLight.x() - intorno &&
				puntoColpitoShadow.y() < pointOnLight.y() + intorno &&
				puntoColpitoShadow.y() > pointOnLight.y() - intorno &&
				puntoColpitoShadow.z() < pointOnLight.z() + intorno &&
				puntoColpitoShadow.z() > pointOnLight.z() - intorno) {   /// Se lo shadow ray colpisce la sorgente luminosa in considerazione

				  if (normal.Dot3 (dirToLight) > 0)
				  {
					Vec3f lightColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
					answer += m->Shade(ray, hit, dirToLight, lightColor, args);
				  }
			}
		}
	}*/
  }

  return answer;
}
