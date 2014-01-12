#ifndef WRAPPERCPP_H
#define WRAPPERCPP_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>

class WrapperCPP
{

public:
    WrapperCPP();
    ~WrapperCPP();

    void choix(int i);
    void openFile(const std::string& fileName);
    void freeCData();
    char* getLatestNewImage();
    void setManSeuil(int seuil);
    void setTailleGaussien(int taille);
    void doPaletteReduction(int level);

private:


};

#endif // WRAPPERCPP_H
