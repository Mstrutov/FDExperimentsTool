#pragma once

#include "FDTreeElement.h"

class FDTree: public FDTreeElement{
public:
    FDTree(const int& maxAttributeNumber);

    void addMostGeneralDependencies();

    void addFunctionalDependency(boost::dynamic_bitset<> lhs, const int& a);

    void filterSpecialization();
    void containtsGeneralization();

};