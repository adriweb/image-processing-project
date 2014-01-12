// Adrien Bertrand
// Mini-Projet TdI
// v4.5 - 11/01/2014

#include "watershed.h"

/////////////////
//!\ TO FIX ! /!\
/////////////////

uint** getNeighbors(uint** src, uint x, uint y)
{
	int j;

	uint** nb = (uint**)malloc(3 * sizeof(uint*));
	if (!nb) return NULL;
	for (j = 0; j < 3; j++) {
		nb[j] = (uint*)calloc(3, sizeof(uint));
		if (!nb[j]) return NULL;
	}

	//							nb[0][1] = src[y-1][x];
	//nb[1][0] = src[ y ][x-1];	nb[1][1] = src[ y ][x];		nb[1][2] = src[ y ][x+1];
	//							nb[2][1] = src[y+1][x];

	nb[0][0] = src[y-1][x-1];	nb[0][1] = src[y-1][x];		nb[0][2] = src[y-1][x+1];
	nb[1][0] = src[ y ][x-1];	nb[1][1] = src[ y ][x];		nb[1][2] = src[ y ][x+1];
	nb[2][0] = src[y+1][x-1];	nb[2][1] = src[y+1][x];		nb[2][2] = src[y+1][x+1];


	return nb;
}


bool aNeighborIsLower(uint** nb, u16 n)
{
	int i, j;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if (nb[j][i] < n) return true;

	return false;
}

bool aNeighborIsEqual(uint** nb, u16 n)
{
	int i, j;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if ((j!=1 && i!=1) && nb[j][i] == n) return true;

	return false;
}

bool aNeighborIsADifferentMarker(uint** nb, uint n)
{
	int i, j;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if ((j != 1 && i != 1) && nb[j][i] > 0 && nb[j][i] != label_collision && nb[j][i] != n) return true;

	return false;
}

u16** getMinima(u16** src)
{
	int i, j;
	uint** nb = NULL;

	// init loop
	u16** minMap = (u16**)malloc(img_h * sizeof(u16*));
	if (!minMap) return NULL;
	for (j = 0; j < img_h; j++) {
		minMap[j] = (u16*)calloc(img_w, sizeof(u16)); // will make everything mm_default (0)
		if (!minMap[j]) return NULL;
	}

	// analysis loop
	for (j = 1; j < img_h - 1; j++) { 			// TODO :
		for (i = 1; i < img_w - 1; i++) {		// handle edges.
			nb = getNeighbors((uint**)src, i, j);
			if (!nb) return NULL;
			if (aNeighborIsLower(nb, src[j][i])) {
				// on est actuellement sur une pente
				minMap[j][i] = mm_default;
			} else if (aNeighborIsEqual(nb, src[j][i])) {
				minMap[j][i] = mm_plateau; // plateau, mais pas forcément minimum
			} else {
				minMap[j][i] = mm_minimum;
			}
			
			secure_free(nb[0]); secure_free(nb[1]); secure_free(nb[2]);
			secure_free(nb);
		}
	}

	return minMap;
}

void image_flooding_overlay(DonneesImageRGB* image, floodmap watershed_mask) {
	int i, j, k = 0;
	for (j = 0; j < image->hauteurImage; j++) {
		for (i = 0; i < image->largeurImage; i++) {
			if (watershed_mask[j][i].label > 0 && watershed_mask[j][i].label < label_collision) {
				image->donneesRGB[k] = 0; image->donneesRGB[k + 1] = 0; image->donneesRGB[k + 2] = 255;
			}/* else if (watershed_mask[j][i].label == label_collision) {	
				//image->donneesRGB[k] = 255; 	image->donneesRGB[k + 1] = 0; 	image->donneesRGB[k + 2] = 0;
			} */
			k += 3;
		}
	}
}

uint getNumberOfMarkersInNeighboors(uint** nb)
{
	uint i, j, markers = 0;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if (nb[j][i]>0 && nb[j][i]!=label_collision) markers++; // -1  ==> uint_max - 1
	
	return markers;
}

uint get_the_nb_marker(uint** nb)
{
	uint i, j;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if (nb[j][i]>0 && nb[j][i]!=label_collision) return nb[j][i];

	return 0; // should never happen
}

bool areTheMarkersDifferent(uint** nb)
{
	uint i, j;
	uint a_marker = get_the_nb_marker(nb);
	for (j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			if (nb[j][i]>0 && nb[j][i] != label_collision)
				if (a_marker != nb[j][i]) return true;

	return false;
}

uint** old_ws_flooding(u16** src)
{
	int i, j;
	uint** nb = NULL;

	uint** flood_mask = (uint**)malloc(img_h * sizeof(uint*));
	if (!flood_mask) return NULL;
	for (j = 0; j < img_h; j++) {
		flood_mask[j] = (uint*)calloc(img_w, sizeof(uint)); // 0 
		if (!flood_mask[j]) return NULL;
	}

	uint current_marker = 0;
	int current_depth = 0;		// 0->deepest, 255->top

	// let the water flow...
	for (current_depth = 0; current_depth < 256; current_depth++) { // todo : get min and max of image to shrink the useful bounds of for loop
		for (j = 1; j < img_h-1; j++)
		for (i = 1; i < img_w-1; i++) {
			if (src[j][i] == current_depth) {
				nb = getNeighbors(flood_mask, i, j);
				if (!nb) return NULL;

				switch (getNumberOfMarkersInNeighboors(nb)) {
					case 0:
						flood_mask[j][i] = ++current_marker;
						break;
					case 1:
						flood_mask[j][i] = get_the_nb_marker(nb);
						break;
					default: // 2 or more
						flood_mask[j][i] = areTheMarkersDifferent(nb) ? label_collision : get_the_nb_marker(nb);
						break;
				}

				secure_free(nb[0]); secure_free(nb[1]); secure_free(nb[2]);
				secure_free(nb);
			}
		}
	}

	printf("current_marker = %d\n", current_marker);

	return flood_mask;
}


void ws_water_grow(u16** src, floodmap map, int x, int y, uint label)
{
	int i, j;

	map[y][x].isNew = false;
	map[y][x].label = label;

	for (j = y - 1; j < y + 1; j++) {
		for (i = x - 1; i < x + 1; i++) {
			if ((i != x && j != y) && src[j][i] >= src[y][x]) {
				if (map[j][i].label == 0) {
					map[j][i].isNew = true;
					map[y][x].label = label;
				} else if (map[j][i].label < label_collision) {
					map[j][i].isNew = false;
					map[j][i].label = label_collision;
				} else {
					map[j][i].isNew = false;
				}
			}
		}
	}
	/*for (j = y - 1; j < y + 1; j++) {
		for (i = x - 1; i < x + 1; i++) {
			if ((i != x && j != y) && map[j][i].isNew == true)
				;// ws_water_grow(src, map, i, j, label);
		}
	}*/
}

floodmap ws_flooding(u16** src)
{
	int i, j;

	floodmap map = (floodmap)malloc(img_h * sizeof(floodpoint*));
	if (!map) return NULL;
	for (j = 0; j < img_h; j++) {
		map[j] = (floodpoint*)calloc(img_w, sizeof(floodpoint)); // 0 
		if (!map[j]) return NULL;
	}

	int current_depth = 0;		// 0->deepest, 255->top
	uint current_label = 1;

	// let the water flow...
	for (current_depth = 0; current_depth < 256; current_depth++) // todo : get min and max of image to shrink the useful bounds of for loop
		for (j = 1; j < img_h - 1; j++)
		for (i = 1; i < img_w - 1; i++)
			if (src[j][i] == current_depth && map[j][i].label == 0)
				ws_water_grow(src, map, i, j, ++current_label);

	// printf("current_label = %d\n", current_label);

	return map;
}


void watershed(u16 ** src, DonneesImageRGB* img)
{
	floodmap map = ws_flooding(src);
	image_flooding_overlay(img, map);
	int i;
#pragma omp parallel for
	for (i = 0; i < img_h; i++)
		secure_free(map[i]);
	secure_free(map);
}
