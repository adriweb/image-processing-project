#ifndef __WATERSHED_H__
#define __WATERSHED_H__

#include "common.h"

#ifndef UINT_MAX 
#define UINT_MAX	(uint)(-1)
#endif
#define label_collision	UINT_MAX

/**
* \brief	(obsolete) différentes valeurs pour la minmap
*/
typedef enum _minmap_props {
	mm_default = 0, // 0 (default by calloc)
	mm_minimum = 255,
	mm_plateau = 128
} minmap_props;

/**
* \brief	propriétés de chaque point pour la floodmap
*/
typedef struct _floodpoint {
	uint label;
	bool isNew;
} floodpoint;

typedef floodpoint** floodmap;

/**
* \brief	Retourne un tableau de "marqueurs" où se trouvent les minima et plateaux de l'image source (niveau-de-gris)
* \details	Marqueurs : 0 == default (ni minimum, ni plateau), 1 == minimum, 2 == plateau
* \param    src			Pointeur vers le tableau représentant l'image source (niveau-de-gris)
* \return   Pointeur vers le tableau de marqueurs.
*/
u16** getMinima(u16** src);

/**
* \brief	Crée un tableau des points voisins au point donné
* \param    src			Pointeur vers le tableau représentant l'image source (niveau-de-gris)
* \param    x			Coordonée en x du point voulu
* \param    y			Coordonée en y du point voulu
* \return   Pointeur vers le tableau des voisins.
*/
uint** getNeighbors(uint** src, uint x, uint y);

/**
* \brief	Dessine le "masque" du watershed par dessus l'image donnée
* \param    image			Pointeur vers l'image de destination
* \param    watershed_mask	la floodmap du watershed ("masque")
*/
void image_flooding_overlay(DonneesImageRGB* image, floodmap watershed_mask);

/**
* \brief	(obsolete) différentes valeurs pour la minmap
*/
uint** old_ws_flooding(u16** src);

/**
* \brief	montée de l'eau
*/
void ws_water_grow(u16** src, floodmap map, int x, int y, uint label);

/**
* \brief	essai d'algo de watershed
*/
floodmap ws_flooding(u16** src);

/**
* \brief	appelant de l'algo du watershed
*/
void watershed(u16 ** src, DonneesImageRGB* img);

#endif
