// Adrien Bertrand
// Mini-Projet TdI
// v4.5 - 11/01/2014

#include "TD2-TDI.h"
#include "filtering.h"
#include "watershed.h"
#include "utils.h"


// Petites variables globales et pointeurs globaux

int img_w, img_h;
char* nomFichier = NULL;

int man_seuil = 100;
int taille_gaussien = 0;
int nbrCouleursReduc = 0;

DonneesImageRGB * image_orig;
DonneesImageRGB * image;
DonneesImageRGB * histo_img;
DonneesImageRGB * tmp_img;
u16 ** tmp_ng1;
u16 ** tmp_ng2;
u16 ** tmp_ng3;
u16 ** matrice_bleue;
u16 ** matrice_rouge;
u16 ** matrice_verte;
u16 ** image_ng;
uint * histo;
uint * histoCumule;



void cree3matrices(u16** mat_bleue, u16** mat_rouge, u16** mat_verte, DonneesImageRGB* image) {
    int i, j, k = 0;
    for (j = 0; j < image->hauteurImage; j++) {
        for (i = 0; i < image->largeurImage; i++) {
            mat_bleue[j][i] = (u16)image->donneesRGB[k++];
            mat_verte[j][i] = (u16)image->donneesRGB[k++];
            mat_rouge[j][i] = (u16)image->donneesRGB[k++];
        }
    }
}

void creeImage(DonneesImageRGB* image, u16** mat_bleue, u16** mat_rouge, u16** mat_verte) {
    int i, j, k = 0;
    for (j = 0; j < image->hauteurImage; j++) {
        for (i = 0; i < image->largeurImage; i++) {
            image->donneesRGB[k++] = (u8)mat_bleue[j][i];
            image->donneesRGB[k++] = (u8)mat_verte[j][i];
            image->donneesRGB[k++] = (u8)mat_rouge[j][i];
        }
    }
}

void negatifImage(u16** mat_bleue, u16** mat_rouge, u16** mat_verte) {
    int i, j;
#pragma omp parallel for
    for (j = 0; j < img_h; j++) {
        for (i = 0; i < img_w; i++) {
            mat_rouge[j][i] = 255 - mat_rouge[j][i];
            mat_verte[j][i] = 255 - mat_verte[j][i];
            mat_bleue[j][i] = 255 - mat_bleue[j][i];
        }
    }
}

u16** couleur2NG(u16** mat_bleue, u16** mat_rouge, u16** mat_verte, bool perceptive) {
    int i, j;
    double coeff_r = perceptive ? 0.2125 : 1,
        coeff_g = perceptive ? 0.7154 : 1,
        coeff_b = perceptive ? 0.0721 : 1;
    uint divi = perceptive ? 1 : 3;
    u16** imageNG = (u16**)malloc(img_w*img_h * sizeof(u16*));
    if (!imageNG) return NULL;
    for (j = 0; j < img_h; j++) {
        imageNG[j] = (u16*)calloc(img_w, sizeof(u16));
        if (!imageNG[j]) return NULL;
        for (i = 0; i < img_w; i++)
            imageNG[j][i] = (u16)((mat_rouge[j][i] * coeff_r + mat_verte[j][i] * coeff_g + mat_bleue[j][i] * coeff_b) / divi);
    }
    return imageNG;
}

void seuilleImageNG(u16** imageNG, uint seuil) {
    int i, j = 0;
#pragma omp parallel for
    for (j = 0; j < img_h; j++)
    for (i = 0; i < img_w; i++)
        imageNG[j][i] = (imageNG[j][i] > seuil) ? 255 : 0;
}

uint* histogramme(u16** imageNG) {
    histo = (uint*)calloc(GRAYLEVELS, sizeof(uint));
    if (!histo) return NULL;

    int i, j;
    for (j = 0; j < img_h; j++)
    for (i = 0; i < img_w; i++)
        histo[imageNG[j][i]]++;

    return histo;
}

uint* histogrammeCumule(u16** imageNG) {
    histoCumule = histogramme(imageNG);

    int i;
    for (i = 1; i < GRAYLEVELS; i++)
        histoCumule[i] += histoCumule[i - 1];

    return histoCumule;
}

void histo_egalisation(u16** imageNG) {
    histoCumule = histogrammeCumule(imageNG);

    int i, j = 0;
    float ratio = 255.0f / (float)(img_h*img_w);
#pragma omp parallel for
    for (j = 0; j < img_h; j++)
    for (i = 0; i < img_w; i++)
        imageNG[j][i] = (u16)((float)histoCumule[imageNG[j][i]] * ratio);

    secure_free(histoCumule);
}

DonneesImageRGB* imageHistogramme(uint* histo) {
    DonneesImageRGB* imageHisto = (DonneesImageRGB*)malloc(sizeof(DonneesImageRGB));
    if (!imageHisto) return NULL;

    int i, j;
    uint colonnes = GRAYLEVELS, lignes = 180;

    float ratio = histo[array_max_idx((int*)histo, GRAYLEVELS)] / (float)lignes;

    imageHisto->donneesRGB = (unsigned char*)malloc(colonnes * lignes * 3 * sizeof(unsigned char));
    if (!imageHisto->donneesRGB) return NULL;
    memset(imageHisto->donneesRGB, 255, colonnes * lignes * 3 * sizeof(unsigned char)); // fond blanc
    imageHisto->hauteurImage = lignes;	imageHisto->largeurImage = colonnes;

    uint* histoNorm = (uint*)malloc(GRAYLEVELS*sizeof(uint));
    if (!histoNorm) return NULL;

#pragma omp parallel for
    for (i = 0; i < GRAYLEVELS; i++)
        histoNorm[i] = (uint)(histo[i] / ratio); // normalisation

#pragma omp parallel for
    for (i = 0; i < GRAYLEVELS; i++)
    for (j = 0; j < (int)histoNorm[i]; j++)
        memset(&(imageHisto->donneesRGB[(3 * (i + j * colonnes))]), (j >= (int)histoNorm[i - 1] || j >= (int)histoNorm[i + 1]) ? 0 : j % 235, 3);

    secure_free(histoNorm);

    return imageHisto;
}


rect_t* rectangleEnglobant(u16** imageNG) {
    rect_t* rect = (rect_t*)malloc(sizeof(rect_t));

    // absurd init values.
    rect->x = -1; rect->w = img_w + 1;
    rect->y = -1; rect->h = img_h + 1;

    int break1, break2, break3, break4;
    break1 = break2 = break3 = break4 = 0;

    int i, j;

    // haut
    for (j = img_h - 1; j > 0 && break1 == 0; j--)
    for (i = 0; i < img_w && break1 == 0; i++)
    if (imageNG[j][i]>0) { rect->y = j; break1 = 1; }

    // bas
    for (j = 0; j < img_h && break2 == 0; j++)
    for (i = 0; i < img_w && break2 == 0; i++)
    if (imageNG[j][i]>0) { rect->h = rect->y - j; break2 = 1; }

    // gauche
    for (i = 0; i < img_w && break3 == 0; i++)
    for (j = img_h - 1; j > 0 && break3 == 0; j--)
    if (imageNG[j][i]>0) { rect->x = i; break3 = 1; }

    // droite
    for (i = img_w - 1; i > 0 && break4 == 0; i--)
    for (j = img_h - 1; j > 0 && break4 == 0; j--)
    if (imageNG[j][i] > 0) { rect->w = i - rect->x; break4 = 1; }

    return rect;
}

int get_seuil_otsu(u16** src)
{
    int* histo = (int*)histogramme(src);

    int i, threshold = 0, taille = img_w * img_h;
    int wB = 0, wF = 0;
    float mB, mF, varBetween;
    float sum = 0, sumB = 0, varMax = 0;

    for (i = 0; i < GRAYLEVELS; i++)
        sum += (float)i * histo[i];

    for (i = 0; i < GRAYLEVELS; i++) {
        wB += histo[i];					// Weight Background
        if (wB == 0) continue;

        wF = taille - wB;				// Weight Foreground
        if (wF == 0) break;

        sumB += (float)(i * histo[i]);

        mB = sumB / wB;					// Mean Background
        mF = (sum - sumB) / wF;			// Mean Foreground

        // Calculate Between Class Variance
        varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        // Check if new maximum found
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    secure_free(histo);

    return threshold;
}

void do_Seuil(int seuil)
{
#ifdef CONSOLE
    seuil = -1;
    while (seuil < 0 || seuil > 255) {
        printf("Seuil ? (0-255)\n");
        scanf_s("%d", &seuil);
        printf("\n");
    }
#endif
    if (seuil > 0 && seuil < 256) {
        seuilleImageNG(image_ng, seuil);
        sauveImageNG(image, image_ng);
        saveBMPwithCurrentName(image, "seuil.bmp");
    }
}


void doGaussienN(int gaussien_size)
{
#ifdef CONSOLE
    while (gaussien_size < 0 || gaussien_size > 50 || gaussien_size % 2 == 0) {
        printf("Taille du filtre gaussien ? (entier impair positif)\n");
        scanf_s("%d", &gaussien_size);
        printf("\n");
    }
#endif

    filter_t* flt = createGaussianFilterN((gaussien_size > 9) ? 9 : gaussien_size);

    tmp_ng1 = apply_filter(image_ng, flt);

    // todo : faire qqchose pour cette arnaque du siècle :
    int i;
    if (gaussien_size > 9)
    for (i = 9; i < gaussien_size; i++)
        tmp_ng1 = apply_filter(tmp_ng1, filters[flt_Gaussien3]);

    freeFilter(flt);

    sauveImageNG(image, tmp_ng1);

    char tmp[30] = "filter_gaussienPerso_";
    char tmp2[3];
    sprintf(tmp2, "%d", gaussien_size);
    strcat(tmp, tmp2);
    strcat(tmp, ".bmp");
    saveBMPwithCurrentName(image, tmp);
}


u16 ** paletteReduction(u16 ** src, int levelsAmount)
{
    levelsAmount--;
    int i, j, k;
    float interval = 256 / (float)levelsAmount;
    float half_interval = interval / 2;

    // palette init
    u16* levels = (u16*)malloc((levelsAmount + 1) * sizeof(u16));
    if (!levels) return NULL;
#pragma omp parallel for
    for (i = 0; i < levelsAmount + 1; i++)
        levels[i] = (u16)(i * interval);
    levels[levelsAmount] = 255;

    //    for (i = 0; i < levelsAmount + 1; i++)
    //        error("levels[%d] = %d\n", i, levels[i]);

    // reduced img init + processing
    u16** reduced = (u16**)malloc(img_w*img_h * sizeof(u16*));
    if (!reduced) return NULL;

    for (j = 0; j < img_h; j++) {
        reduced[j] = (u16*)calloc(img_w, sizeof(u16)); // will make everything mm_default (0)
        if (!reduced[j]) return NULL;

#pragma omp parallel for
        for (i = 0; i < img_w; i++)
            for (k = 0; k < levelsAmount + 1; k++)
                if ((src[j][i] >= levels[k] - (k>0 ? half_interval + 0.5 : 0)) && (src[j][i] < levels[k] + half_interval + 0.5))
                    reduced[j][i] = levels[k];
    }

    secure_free(levels);

    return reduced;
}

void do_PaletteReduction(int level)
{
#ifdef CONSOLE
    level = -1;
    while (level < 2 || level > 254) {
        printf("Combien de niveaux ? (2-254)\n");
        scanf_s("%d", &level);
        printf("\n");
    }
#endif
    error("level : %d\n", level);
    if (level > 1 && level < 255) {
        u16** tmp = paletteReduction(image_ng, level);
        error("apres tmp \n", level);
        sauveImageNG(image, tmp);
        error("apres sauve \n");

        saveBMPwithCurrentName(image, "reduction_palette.bmp");
        error("apres save \n");
        int i;
        for (i = 0; i < img_h; i++) secure_free(tmp[i]);
        secure_free(tmp);
        error("apres free \n");

    }
}


void choixAction(int choix)
{
    int i, seuil;

    static bool isDoingAll = false;

    bool end = (choix == 0);

    bool withinMorpho = false;
    bool withinFilters = false;
    bool withinFilters1 = false;
    bool withinFilters2 = false;
    bool withinOthers = false;

    static const int liste[] = { 1, 2, 4, 5, 611, 612, 613, 614, 621, 622, 623, 631, 632, 633, 64, 65, 66, 72, 73, 8 };
    static const char * const listeNom[] = { "Négatif", "Niveau-de-gris", "Histogramme", "Seuillage auto.", "Dilatation", "Erosion", "Ouverture", "Fermeture", "Moyenneur", "Gaussien 3x3", "Gaussien 5x5", "Roberts", "Prewitt", "Sobel", "Laplacien 4", "Laplacien 8", "Median", "Histogramme cumulé", "Egalisation d'histogramme", "Watershed" };

    while (!end) {

        cree3matrices(matrice_bleue, matrice_rouge, matrice_verte, image_orig);
        image_ng = couleur2NG(matrice_bleue, matrice_rouge, matrice_verte, true);

#ifdef CONSOLE

        if (!isDoingAll) {

            printf("******************************\n");
            printf("******* Adrien BERTRAND ******\n");
            printf("*** TD Traitement d'images ***\n");
            printf("******  v4.5 12/01/14  *******\n");
            printf("******************************\n\n");
            printf("Image en cours : %s\n\n", nomFichier);
            printf("* 1) Negatif d'une image \n");
            printf("* 2) Image couleur -> NG \n");
            printf("* 3) Seuillage manuel d'une image \n");
            printf("* 4) Histogramme d'une image \n");
            printf("* 5) Seuillage automatique \n");
            printf("* 6) Filtres... \n");
            if (withinFilters) {
                printf("* |____ 61) Morphologiques... \n");
                if (withinMorpho) {
                    printf("* \t|____ 611) Dilatation \n");
                    printf("* \t|____ 612) Erosion \n");
                    printf("* \t|____ 613) Ouverture \n");
                    printf("* \t|____ 614) Fermeture \n");
                }
                printf("* |____ 62) Pondérés... \n");
                if (withinFilters1) {
                    printf("* \t|____ 621) Moyenneur \n");
                    printf("* \t|____ 622) Gaussien 3x3 \n");
                    printf("* \t|____ 623) Gaussien 5x5 \n");
                    printf("* \t|____ 624) Gaussien NxN... \n");
                }
                printf("* |____ 63) Gradients... \n");
                if (withinFilters2) {
                    printf("* \t|____ 631) Roberts \n");
                    printf("* \t|____ 632) Prewitt \n");
                    printf("* \t|____ 633) Sobel \n");
                }
                printf("* |____ 64) Laplacien (4) \n");
                printf("* |____ 65) Laplacien (8) \n");
                printf("* |____ 66) Median \n");
            }
            printf("* 7) Divers... \n");
            if (withinOthers) {
                printf("* |____ 71) Réduction de la palette... \n");
                printf("* |____ 72) Histogramme cumulé \n");
                printf("* |____ 73) Egalisation d'histogramme \n");
                printf("* |____ 74) Rectangle englobant \n");
            }
            printf("* 8) Watershed \n");
            printf("------\n");
            printf("* -1) Tout lancer \n");
            printf("------\n");
            printf("* 0) Quitter \n\n");
            printf("******************************\n");
            printf("Choix ? \n");
            scanf_s("%d", &choix);
            printf("\n");

        }
#endif

        switch (choix) {
        case -1:
        {
                   isDoingAll = true;

                   clock_t t = clock();

                   for (i = 0; i < (int)(sizeof(liste) / sizeof(liste[0])); i++) {
                       error("En cours : %s \n", listeNom[i]);
                       choixAction(liste[i]);
                   }

                   isDoingAll = false;

                   t = clock() - t;
                   double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds
                   printf("\nTemps total mis (environ) : %f s. \n\n", time_taken);
        }
            break;
        case 1:
            negatifImage(matrice_bleue, matrice_rouge, matrice_verte);
            creeImage(image, matrice_bleue, matrice_rouge, matrice_verte);
            saveBMPwithCurrentName(image, "negatif.bmp");
            break;
        case 2:
            sauveImageNG(image, image_ng);
            saveBMPwithCurrentName(image, "ng.bmp");
            break;
        case 3:
            do_Seuil(man_seuil);
            break;
        case 4:
            cree3matrices(matrice_bleue, matrice_rouge, matrice_verte, image_orig);
            image_ng = couleur2NG(matrice_bleue, matrice_rouge, matrice_verte, false);
            histo = histogramme(image_ng);
            histo_img = imageHistogramme(histo);
            saveBMPwithCurrentName(histo_img, "histogramme.bmp");
            secure_free(histo);
            break;
        case 5:
            seuil = get_seuil_otsu(image_ng);
            seuilleImageNG(image_ng, seuil);
            sauveImageNG(image, image_ng);
            saveBMPwithCurrentName(image, "auto_seuillee.bmp");
            break;
        case 6:
            withinMorpho = withinFilters2 = withinFilters1 = false;
            withinFilters = !withinFilters;
            break;
        case 61:
            withinMorpho = !withinMorpho;
            break;
        case 62:
            withinFilters1 = !withinFilters1;
            break;
        case 63:
            withinFilters2 = !withinFilters2;
            break;
        case 611:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Dilatation]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_dilatation.bmp");
            break;
        case 612:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Erosion]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_erosion.bmp");
            break;
        case 613:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Erosion]);
            tmp_ng2 = apply_filter(tmp_ng1, filters[flt_Dilatation]);
            sauveImageNG(image, tmp_ng2);
            saveBMPwithCurrentName(image, "filter_opening.bmp");
            break;
        case 614:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Dilatation]);
            tmp_ng2 = apply_filter(tmp_ng1, filters[flt_Erosion]);
            sauveImageNG(image, tmp_ng2);
            saveBMPwithCurrentName(image, "filter_closing.bmp");
            break;
        case 621:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Average]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_moyenneur.bmp");
            break;
        case 622:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Gaussien3]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_gaussien3.bmp");
            break;
        case 623:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Gaussien5]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_gaussien5.bmp");
            break;
        case 624:
            doGaussienN(taille_gaussien);
            break;
        case 631:
#pragma omp parallel
        {
#pragma omp sections
            {
#pragma omp section
                { tmp_ng1 = apply_filter(image_ng, filters[flt_RobertsX]); }
#pragma omp section
                { tmp_ng2 = apply_filter(image_ng, filters[flt_RobertsY]); }
            }
        }
            tmp_ng3 = gradientMagnitude(tmp_ng1, tmp_ng2);
            sauveImageNG(image, tmp_ng3);
            saveBMPwithCurrentName(image, "filter_roberts.bmp");
            break;
        case 632:
#pragma omp parallel
        {
#pragma omp sections
            {
#pragma omp section
                { tmp_ng1 = apply_filter(image_ng, filters[flt_PrewittX]); }
#pragma omp section
                { tmp_ng2 = apply_filter(image_ng, filters[flt_PrewittY]); }
            }
        }
            tmp_ng3 = gradientMagnitude(tmp_ng1, tmp_ng2);
            sauveImageNG(image, tmp_ng3);
            saveBMPwithCurrentName(image, "filter_prewitt.bmp");
            break;
        case 633:
#pragma omp parallel
        {
#pragma omp sections
            {
#pragma omp section
                { tmp_ng1 = apply_filter(image_ng, filters[flt_SobelX]); }
#pragma omp section
                { tmp_ng2 = apply_filter(image_ng, filters[flt_SobelY]); }
            }
        }
            tmp_ng3 = gradientMagnitude(tmp_ng1, tmp_ng2);
            sauveImageNG(image, tmp_ng3);
            saveBMPwithCurrentName(image, "filter_sobel.bmp");
            break;
        case 64:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Laplacien_4]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_Laplacien_4.bmp");
            break;
        case 65:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Laplacien_8]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_Laplacien_8.bmp");
            break;
        case 66:
            tmp_ng1 = apply_filter(image_ng, filters[flt_Median]);
            sauveImageNG(image, tmp_ng1);
            saveBMPwithCurrentName(image, "filter_median.bmp");
            break;
        case 7:
            withinOthers = !withinOthers;
            break;
        case 71:
            do_PaletteReduction(nbrCouleursReduc);
            break;
        case 72:
            cree3matrices(matrice_bleue, matrice_rouge, matrice_verte, image_orig);
            image_ng = couleur2NG(matrice_bleue, matrice_rouge, matrice_verte, false);
            histoCumule = histogrammeCumule(image_ng);
            histo_img = imageHistogramme(histoCumule);
            saveBMPwithCurrentName(histo_img, "histogramme_cumule.bmp");
            secure_free(histoCumule);
            break;
        case 73:
            histo_egalisation(image_ng);
            sauveImageNG(image, image_ng);
            saveBMPwithCurrentName(image, "egalisation_histogramme.bmp");
            break;
        case 74:
        {
           rect_t* rect = rectangleEnglobant(image_ng);
           printf("Rectangle englobant : x=%d ; y=%d ; largeur=%d ; hauteur=%d\n", rect->x, img_h - rect->y, rect->w, rect->h);
           secure_free(rect);
           break;
        }
        case 8:
            //image_ng = paletteReduction(image_ng, 16); // abaissement de la dynamique.
            seuilleImageNG(image_ng, get_seuil_otsu(image_ng));
            tmp_ng1 = apply_filter(image_ng, filters[flt_Median]); // elimination des impuretés
            watershed(tmp_ng1, image);
            saveBMPwithCurrentName(image, "watershed.bmp");
            break;
        case 0:
            end = true;
            break;
        default:
            printf("Mauvais choix !\n\n");
            break;
        }

#pragma omp parallel for
        for (i = 0; i < img_h; i++) {
            if (image_ng) secure_free(image_ng[i]);
            if (tmp_ng1) secure_free(tmp_ng1[i]);
            if (tmp_ng2) secure_free(tmp_ng2[i]);
            if (tmp_ng3) secure_free(tmp_ng3[i]);
        }
        secure_free(image_ng);
        secure_free(tmp_ng1);
        secure_free(tmp_ng2);
        secure_free(tmp_ng3);
        libereDonneesImageRGB(&histo_img);
        libereDonneesImageRGB(&tmp_img);


#ifdef CONSOLE
        if (!isDoingAll) {
            if (!end && (choix != 6 && choix != 7 && (choix<9 || choix>63))) system("pause");
            system("cls");
        }
#else
        end = true;
#endif

        if (isDoingAll) break;
    }
}


void initData(int argc, char *argv[])
{
    nomFichier = (char*)calloc(350, sizeof(char));
    if (!nomFichier) return;

    error("argc: %d\n", argc);
#ifndef CONSOLE
    error("loaded : %s\n", argv[1]);
#endif
    strcpy_s(nomFichier, (argc > 1) ? strlen(argv[1]) + 1 : 10, (argc > 1) ? argv[1] : "bat3.bmp");

    if (!strEndsWith(nomFichier, ".bmp")) {
        printf("Image name doesn't contain the extension, adding it...\n");
        strcat(nomFichier, ".bmp");
    }

    if (!(image = lisBMPRGB(nomFichier))) {
        error("Erreur de lecture, fermeture... \n");
        secure_free(nomFichier);
        exit(-1);
    }
    if (!(image_orig = lisBMPRGB(nomFichier))) {
        error("Erreur de mémoire \n");
        secure_free(nomFichier);
        exit(-1);
    }

    filters = createFilters();

    int i;

    img_w = image->largeurImage;
    img_h = image->hauteurImage;

    matrice_rouge = (u16**)malloc(img_h * sizeof(u16*));
    matrice_verte = (u16**)malloc(img_h * sizeof(u16*));
    matrice_bleue = (u16**)malloc(img_h * sizeof(u16*));
    if (!(matrice_rouge && matrice_verte && matrice_bleue)) exit(-1);

    for (i = 0; i < img_h; i++) {
        matrice_rouge[i] = (u16*)malloc(img_w * sizeof(u16));
        matrice_verte[i] = (u16*)malloc(img_w * sizeof(u16));
        matrice_bleue[i] = (u16*)malloc(img_w * sizeof(u16));
    }
    if (!(matrice_rouge[0] && matrice_verte[0] && matrice_bleue[0])) exit(-1);
}

void freeStuff()
{
    int i;
    for (i = 0; i < NBR_FILTRES; i++)
        freeFilter(filters[i]);
    secure_free(filters);

    secure_free(nomFichier);

#pragma omp parallel for
    for (i = 0; i < img_h; i++) {
        if (matrice_bleue) secure_free(matrice_bleue[i]);
        if (matrice_rouge) secure_free(matrice_rouge[i]);
        if (matrice_verte) secure_free(matrice_verte[i]);
    }
    secure_free(matrice_bleue);
    secure_free(matrice_rouge);
    secure_free(matrice_verte);
    libereDonneesImageRGB(&image);
    libereDonneesImageRGB(&image_orig);
}

int MAIN_NAME(int argc, char *argv[])
{
#ifdef CONSOLE
    setlocale(LC_ALL, ""); // support unicode

    initData(argc, argv);

    choixAction(-1);

    freeStuff();
#else
    error("Why calling the console launcher from the GUI ... ?\n");
#endif

    return 0;
}
