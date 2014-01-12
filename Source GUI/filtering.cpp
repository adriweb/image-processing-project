// Adrien Bertrand
// Mini-Projet TdI
// v4.5 - 11/01/2014


#include "filtering.h"

filter_t** filters;

#define INT_SWAP(a,b) { register int t=(a);(a)=(b);(b)=t; }

int getMedian(int* arr, uint n)
{
    int low, high;
    int median;
    int middle, ll, hh;

    low = 0; high = n - 1; median = (low + high) >> 1;
    for (;;) {

        if (high <= low) // One element only
            return arr[median];

        if (high == low + 1) {  // Two elements only
            if (arr[low] > arr[high])
                INT_SWAP(arr[low], arr[high]);
            return arr[median];
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) >> 1;
        if (arr[middle] > arr[high])    INT_SWAP(arr[middle], arr[high]);
        if (arr[low] > arr[high])       INT_SWAP(arr[low], arr[high]);
        if (arr[middle] > arr[low])     INT_SWAP(arr[middle], arr[low]);

        /* Swap low item (now in position middle) into position (low+1) */
        INT_SWAP(arr[middle], arr[low + 1]);

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;) {
            do ll++; while (arr[low] > arr[ll]);
            do hh--; while (arr[hh] > arr[low]);

            if (hh < ll)
                break;

            INT_SWAP(arr[ll], arr[hh]);
        }

        /* Swap middle item (in position low) back into correct position */
        INT_SWAP(arr[low], arr[hh]);

        /* Re-set active partition */
        if (hh <= median)
            low = ll;
        if (hh >= median)
            high = hh - 1;
    }
}
#undef ELEM_SWAP

void reNormalize(int** imageNG)
{
    int min = INT_MAX, max = INT_MIN, val;
    double ratio = 0.0;
    int i, j;
    for (j = 0; j < img_h; j++) {
        for (i = 0; i < img_w; i++) {
            val = imageNG[j][i];
            if (val < min) min = val;
            if (val > max) max = val;
        }
    }

    ratio = ((double)255) / (max - min);

#pragma omp parallel for
    for (j = 0; j < img_h; j++)
        for (i = 0; i < img_w; i++)
            imageNG[j][i] = (int)((imageNG[j][i]-min) * ratio);

}

void reBound(int** imageNG)
{
    int i, j;
    for (j = 0; j < img_h; j++)
    for (i = 0; i < img_w; i++)
        imageNG[j][i] = (imageNG[j][i] < 0) ? 0 : ((imageNG[j][i]) > 255 ? 255 : imageNG[j][i]);
}

u16** apply_filter(u16** src, filter_t* filter)
{
    int filter_size = filter->size;

    bool needNormalization = filter->needNormalization;

    if (filter_size % 2 == 0) {
        error("Erreur : le filtre n'est pas de taille impaire !\n");
        return NULL;
    }
    if (filter_size > img_w) {
        error("Erreur : le filtre est plus grand que l'image source !\n");
        return NULL;
    }

    int i, j;
    int x, y;
    int tmp;

    int* median_array = NULL;
    if (filter->method == flt_m_Median) {
        median_array = (int*)calloc(filter_size * filter_size, sizeof(int));
        if (!median_array) return NULL;
    }

    uint median_array_idx;

    int weightSum = 1;

    if (filter->needNormalization && filter->mask && filter->div == 0) {
        for (j = 0; j < filter_size; j++)
        for (i = 0; i < filter_size; i++)
            weightSum += filter->mask[j][i];
    }

    // Overrides :
    if (filter->div > 0)
        weightSum = filter->div;


    int** imageNG = (int**)malloc(img_w*img_h * sizeof(int*));
    if (!imageNG) return NULL;
    for (j = 0; j < img_h; j++) {
        imageNG[j] = (int*)calloc(img_w, sizeof(int));
        if (!imageNG[j]) return NULL;
    }

    int offset = filter_size >> 1;

    switch (filter->method) {
    case flt_m_Convolution:
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, tmp = 0; i < img_w - offset; i++, tmp = 0) {
            for (y = -offset; y <= offset; y++)
            for (x = -offset; x <= offset; x++)
                tmp += (int)(src[j + y][x + i]) * (int)(filter->mask[y + offset][x + offset]);
            imageNG[j][i] = needNormalization ? tmp / weightSum : tmp;
        }
        break;
    case flt_m_Average:
#pragma omp parallel for
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, tmp = 0; i < img_w - offset; i++, tmp = 0) {
            for (y = -offset; y <= offset; y++)
            for (x = -offset; x <= offset; x++)
                tmp += (int)(src[j + y][x + i]);
            imageNG[j][i] = tmp / weightSum;
        }
        break;
    case flt_m_Median:
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, median_array_idx = 0; i < img_w - offset; i++, median_array_idx = 0) {
            for (y = -offset; y <= offset; y++)
            for (x = -offset; x <= offset; x++)
                median_array[median_array_idx++] = (int)(src[j + y][x + i]);
            imageNG[j][i] = getMedian(median_array, filter_size*filter_size);
        }
        secure_free(median_array);
        break;
    case flt_m_Dilatation:
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, tmp = -1; i < img_w - offset; i++, tmp = -1) {
            for (y = -offset; y <= offset; y++)
            for (x = -offset; x <= offset; x++)
            if (src[j + y][x + i] > tmp) tmp = src[j + y][x + i];
            imageNG[j][i] = tmp;
        }
        break;
    case flt_m_Erosion:
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, tmp = 256; i < img_w - offset; i++, tmp = 256) {
            for (y = -offset; y <= offset; y++)
            for (x = -offset; x <= offset; x++)
            if (src[j + y][x + i] < tmp) tmp = src[j + y][x + i];
            imageNG[j][i] = tmp;
        }
        break;
    default:
#pragma omp parallel for
        for (j = offset; j < img_h - offset; j++)
        for (i = offset, tmp = 0; i < img_w - offset; i++)
            imageNG[j][i] = src[j][i];
        break;
    }

    if (filter->method == flt_m_Convolution) {
        if (filter->reBoundOnly)
            reBound(imageNG);
        else
            reNormalize(imageNG);
    }

    u16** imageNG_16 = (u16**)malloc(img_w*img_h * sizeof(u16*));
    if (!imageNG_16) return NULL;
    for (j = 0; j < img_h; j++) {
        imageNG_16[j] = (u16*)calloc(img_w, sizeof(u16));
        if (!imageNG_16[j]) return NULL;
#pragma omp parallel for
        for (i = 0; i < img_w; i++)
            imageNG_16[j][i] = imageNG[j][i];
    }

#pragma omp parallel for
    for (j = 0; j < img_h; j++)
        secure_free(imageNG[j]);
    secure_free(imageNG);

    return imageNG_16;
}

u16** gradientMagnitude(u16 ** src_x, u16 ** src_y)
{
    u16** imageNG = (u16**)malloc(img_w*img_h * sizeof(u16*));
    if (!imageNG) return NULL;

    int i, j;

    for (j = 0; j < img_h; j++) {
        imageNG[j] = (u16*)calloc(img_w, sizeof(u16));
        if (!imageNG[j]) return NULL;
    }

#pragma omp parallel for
    for (j = 0; j < img_h; j++)
    for (i = 0; i < img_w; i++)
        imageNG[j][i] = (u16)(sqrt(src_x[j][i] * src_x[j][i] + src_y[j][i] * src_y[j][i]));

    return imageNG;
}

void mask_copy_3(int** dest, int src[3][3])
{
    uint i, j;
    for (j = 0; j < 3; j++)
    for (i = 0; i < 3; i++)
        dest[j][i] = src[j][i];
}

void mask_copy_5(int** dest, int src[5][5])
{
    uint i, j;
    for (j = 0; j < 5; j++)
    for (i = 0; i < 5; i++)
        dest[j][i] = src[j][i];
}

void matrix_copy(int** dest, int** src, uint cols, uint rows)
{
    uint i, j;
    for (j = 0; j < rows; j++)
    for (i = 0; i < cols; i++)
        dest[j][i] = src[j][i];
}

filter_t* createCustomFilter(uint size, filter_method_t method, bool needNormalization)
{
    filter_t* filter = (filter_t*)malloc(sizeof(filter_t));
    if (!filter) return NULL;
    filter->mask = (int**)calloc(size*size, sizeof(int*));
    if (!filter->mask) return NULL;

    uint i, j;

    for (j = 0; j < size; j++) {
        filter->mask[j] = (int*)calloc(size, sizeof(int*));
        if (!filter->mask[j]) return NULL;
        for (i = 0; i < size; i++) {
            printf("Masque[%u][%u] = ", j + 1, i + 1);
            scanf_s("%d", &(filter->mask[j][i]));
            printf("\n");
        }
    }

    filter->needNormalization = needNormalization;
    filter->method = method;

    return filter;
}

double** generateNormalizedGaussianNKernel(int size)
{
    int i, j;

    double** coeffs = (double**)malloc(size * sizeof(double*));
    if (!coeffs) return NULL;
    for (j = 0; j < size; j++) {
        coeffs[j] = (double*)calloc(size, sizeof(double));
        if (!coeffs[j]) return NULL;
    }

    double sigma = 1.035f; // well... :D good enough until I find a better way ....
    double cst = (double)(1. / (2 * 3.14159265359 * sigma)); //-V624

#pragma omp parallel for
    for (j = 0; j < size; j++)
    for (i = 0; i < size; i++)
        coeffs[j][i] = cst * exp(-(((i - size / 2)*(i - size / 2) + (j - size / 2)*(j - size / 2)) / (2 * sigma*sigma)));

    double min = 9999;
    for (j = 0; j < size; j++)
    for (i = 0; i < size; i++)
        if (coeffs[j][i] < min) min = coeffs[j][i];

#pragma omp parallel for
    for (j = 0; j < size; j++)
    for (i = 0; i < size; i++)
        coeffs[j][i] = (double)((int)(0.5 + coeffs[j][i] / min)); // hmmm...

    return coeffs;
}

filter_t* createGaussianFilterN(uint size)
{
    filter_t* filter = (filter_t*)malloc(sizeof(filter_t));
    if (!filter) return NULL;
    filter->mask = (int**)calloc(size, sizeof(int*));
    if (!filter->mask) return NULL;

    uint i, j;

    float** coeffs = (float**)generateNormalizedGaussianNKernel(size);

    float weightSum = 0;
    for (j = 0; j < size; j++)
    for (i = 0; i < size; i++)
        weightSum += coeffs[j][i];

    for (j = 0; j < size; j++) {
        filter->mask[j] = (int*)calloc(size, sizeof(int*));
        if (!filter->mask[j]) return NULL;
        for (i = 0; i < size; i++)
            filter->mask[j][i] = (int)coeffs[j][i];
    }

    for (j = 0; j < size; j++)
        secure_free(coeffs[j]);
    secure_free(coeffs);

    filter->div = 0;
    filter->size = size;
    filter->needNormalization = true;
    filter->reBoundOnly = false;
    filter->method = flt_m_Convolution;

    return filter;
}

filter_t** createFilters()
{
    filters = (filter_t**)malloc(NBR_FILTRES * sizeof(filter_t*));
    if (!filters) return NULL;

    uint i, j, size;

    int c;

    c = 2; // Sobel
    int tmp_sobelX[3][3] = { { -1, 0, 1 }, { -c, 0, c }, { -1, 0, 1 } };
    int tmp_sobelY[3][3] = { { -1, -c, -1 }, { 0, 0, 0 }, { 1, c, 1 } };

    c = 1; // Prewitt
    int tmp_prewittX[3][3] = { { -1, 0, 1 }, { -c, 0, c }, { -1, 0, 1 } };
    int tmp_prewittY[3][3] = { { -1, -c, -1 }, { 0, 0, 0 }, { 1, c, 1 } };

    int tmp_robertsX[3][3] = { { 0, 1, 0 }, { 0, 0, -1 } };
    int tmp_robertsY[3][3] = { { 0, 0, 1 }, { 0, -1, 0 } };

    int tmp_gaussien3[3][3] = { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1 } };

    int tmp_gaussien5[5][5] = { { 1, 4, 6, 4, 1 }, { 4, 16, 24, 16, 4 }, { 6, 24, 36, 24, 6 }, { 4, 16, 24, 16, 4 }, { 1, 4, 6, 4, 1 } };

    int tmp_laplacien_4[3][3] = { { 0, -1, 0 }, { -1, 4, -1 }, { 0, -1, 0 } };

    int tmp_laplacien_8[3][3] = { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1 } };


    // default properties
    for (i = 0; i < NBR_FILTRES; i++)
    {
        filters[i] = (filter_t*)malloc(sizeof(filter_t));
        if (!filters[i]) return NULL;
        filters[i]->needNormalization = (i == flt_Gaussien3 || i == flt_Gaussien5 || i == flt_Average || i == flt_Laplacien_4 || i == flt_Laplacien_8);
        filters[i]->div = 0;
        filters[i]->reBoundOnly = (i == flt_SobelX || i == flt_SobelY || i == flt_PrewittX || i == flt_PrewittY || i == flt_RobertsX || i == flt_RobertsY);
        size = (i == flt_Gaussien5) ? 5 : 3;
        filters[i]->size = size;
        if (i != flt_Median && i != flt_Average && i != flt_Dilatation && i != flt_Erosion) {
            filters[i]->mask = (int**)malloc(size * sizeof(int*));
            if (!filters[i]->mask) return NULL;
            for (j = 0; j < size; j++) {
                filters[i]->mask[j] = (int*)calloc(size, sizeof(int));
                if (!filters[i]->mask[j]) return NULL;
            }
        } else {
            filters[i]->mask = NULL;
        }
    }

    filters[flt_Gaussien3]->method   = flt_m_Convolution;
    filters[flt_Gaussien5]->method   = flt_m_Convolution;
    filters[flt_SobelX]->method      = flt_m_Convolution;
    filters[flt_SobelY]->method      = flt_m_Convolution;
    filters[flt_PrewittX]->method    = flt_m_Convolution;
    filters[flt_PrewittY]->method    = flt_m_Convolution;
    filters[flt_Laplacien_4]->method = flt_m_Convolution;
    filters[flt_Laplacien_8]->method = flt_m_Convolution;
    filters[flt_RobertsX]->method    = flt_m_Convolution;
    filters[flt_RobertsY]->method    = flt_m_Convolution;
    filters[flt_Median]->method      = flt_m_Median;
    filters[flt_Average]->method     = flt_m_Average;
    filters[flt_Dilatation]->method  = flt_m_Dilatation;
    filters[flt_Erosion]->method     = flt_m_Erosion;

    filters[flt_Average]->div = 9;
    filters[flt_Laplacien_4]->div = 4;
    filters[flt_Laplacien_8]->div = 8;

    mask_copy_3(filters[flt_Gaussien3]->mask  , tmp_gaussien3);
    mask_copy_5(filters[flt_Gaussien5]->mask  , tmp_gaussien5);
    mask_copy_3(filters[flt_SobelX]->mask     , tmp_sobelX);
    mask_copy_3(filters[flt_SobelY]->mask     , tmp_sobelY);
    mask_copy_3(filters[flt_PrewittX]->mask   , tmp_prewittX);
    mask_copy_3(filters[flt_PrewittY]->mask   , tmp_prewittY);
    mask_copy_3(filters[flt_Laplacien_4]->mask, tmp_laplacien_4);
    mask_copy_3(filters[flt_Laplacien_8]->mask, tmp_laplacien_8);
    mask_copy_3(filters[flt_RobertsX]->mask   , tmp_robertsX);
    mask_copy_3(filters[flt_RobertsY]->mask   , tmp_robertsY);

    return filters;
}
