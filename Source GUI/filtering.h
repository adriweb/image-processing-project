// Adrien Bertrand
// Mini-Projet TdI
// v4.5 - 11/01/2014

#ifndef __FILTERING_H__
#define __FILTERING_H__

#include "common.h"


#define NBR_FILTRES	15

// typedef void *(*void_func_ptr)(void);
// void_func_ptr filter_methods[NBR_FILTRES];

/**
* \brief	Liste des filtres possibles
*/
typedef enum _filters_types {
	flt_RobertsX,
	flt_RobertsY,
	flt_PrewittX,
	flt_PrewittY,
	flt_SobelX,
	flt_SobelY,
	flt_Laplacien_4,
	flt_Laplacien_8,
	flt_Median,
	flt_Average,
	flt_Gaussien3,
	flt_Gaussien5,
	flt_Dilatation,
	flt_Erosion,
	flt_Custom
} filters_types;

/**
* \brief	Méthodes disponibles pour l'implémentation des filtres
*/
typedef enum _filter_method_t {
	flt_m_Convolution,
	flt_m_Median,
	flt_m_Average,
	flt_m_Dilatation,
	flt_m_Erosion
} filter_method_t;

/**
* \brief	Propriétés d'un filtre.
*/
typedef struct _filter_t {
	filter_method_t method;
	int** mask;
	uint size;
	uint div;
	bool needNormalization;
	bool reBoundOnly;
} filter_t;

extern filter_t** filters;


/**
* \brief	Détermine la valeur médiane des éléments du tableau d'entiers.
* \details	A partir d'un tableau d'entiers (pas forcément triés), détermine la valeur médiane des éléments
* \param    arr			Pointeur vers le tableau d'entier
* \param    n			Taille du tableau
* \return   La valeur médiane des éléments du tableau.
* \note		Source : http://ndevilla.free.fr/median/median/src/quickselect.c
*/
int getMedian(int* arr, uint n);

/**
* \brief	Applique un filtre sur une image en niveau de gris (matricielle)
* \param    src			l'image (matricielle) en niveau de gris
* \param    filter		2ème couleur
* \return   Pointeur vers la nouvelle image (matricielle) en niveau de gris, traitée (avec le filtre)
*/
u16** apply_filter(u16** src, filter_t* filter);

/**
* \brief	Calcule le module à partir de 2 images (matricielles) niveau-de-gris sources.
* \details	Utilisée pour Sobel, par exemple.
* \param    src_x		l'image source 1
* \param    src_y		l'image source 2
* \return   Pointeur vers la nouvelle image (matricielle) en niveau de gris, résultante du calcul
*/
u16** gradientMagnitude(u16 ** src_x, u16 ** src_y);

void mask_copy_3(int** dest, int src[3][3]);
void mask_copy_5(int** dest, int src[5][5]);
void matrix_copy(int** dest, int** src, uint cols, uint rows);

/**
* \brief	Crée le tableau des filtres de base.
* \details	Crée le tableau de pointeur de filtres définis dans @ref _filters_types
* \return   Pointeur vers le tableau
*/
filter_t** createFilters();

/**
* \brief	Crée un filtre gaussien de taille n (paramètre 'size')
* \return   Pointeur vers le filtre
*/
filter_t* createGaussianFilterN(uint size);

#endif
