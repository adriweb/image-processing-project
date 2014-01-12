#ifndef __WATERSHED_H__
#define __WATERSHED_H__

#include "common.h"

#ifndef UINT_MAX 
#define UINT_MAX	(uint)(-1)
#endif
#define label_collision	UINT_MAX

typedef enum _minmap_props {
	mm_default = 0, // 0 (default by calloc)
	mm_minimum = 255,
	mm_plateau = 128
} minmap_props;

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

uint** getNeighbors(uint** src, uint x, uint y);

void image_flooding_overlay(DonneesImageRGB* image, floodmap watershed_mask);

uint** old_ws_flooding(u16** src);

void ws_water_grow(u16** src, floodmap map, int x, int y, uint label);

floodmap ws_flooding(u16** src);

void watershed(u16 ** src, DonneesImageRGB* img);

#endif
