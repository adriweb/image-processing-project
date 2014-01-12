#ifndef __UTILS_H__
#define __UTILS_H__

#include "common.h"
#include "filtering.h"

char* getLatestSavedImageName();
int strEndsWith(const char *str, const char *suffix);
int array_min_idx(int* arr, int size);
int array_max_idx(int* arr, int size);
void freeFilter(filter_t* flt);
void saveBMPwithCurrentName(DonneesImageRGB * image, const char* name);

#endif