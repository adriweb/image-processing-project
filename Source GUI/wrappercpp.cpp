#include "wrappercpp.h"
#include "TD2-TDI.h"
#include "utils.h"

WrapperCPP::WrapperCPP()
{
    printf("wrapper created.\n"); fflush(stdout);
}

WrapperCPP::~WrapperCPP()
{
    printf("wrapper destroyed.\n");
    fflush(stdout);fflush(stderr);
}

void WrapperCPP::setManSeuil(int seuil)
{
    man_seuil = seuil;
}

void WrapperCPP::setTailleGaussien(int taille)
{
    taille_gaussien = taille;
}

void WrapperCPP::doPaletteReduction(int level) {
    do_PaletteReduction(level);
}

void WrapperCPP::freeCData()
{
    freeStuff();
    printf("freed stuff.\n");
    fflush(stdout);fflush(stderr);
}

void WrapperCPP::openFile(const std::string& fileName)
{
    char* tmp[2];
    tmp[1] = (char*)calloc(fileName.length(), sizeof(char));
    strcpy(tmp[1], fileName.c_str());
    initData(2, tmp);
    printf("inited stuff\n");
    fflush(stdout);fflush(stderr);
    secure_free(tmp[1]);
}

void WrapperCPP::choix(int i)
{
    choixAction(i);
}

char* WrapperCPP::getLatestNewImage()
{
    return getLatestSavedImageName();
}
