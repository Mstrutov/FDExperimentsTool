#pragma once

#include "FDTreeElement.h"

class FDTree: public FDTreeElement{
public:
    explicit FDTree(const int& maxAttributeNumber=0) : FDTreeElement(maxAttributeNumber) {};

    void addMostGeneralDependencies();

    void addFunctionalDependency(const boost::dynamic_bitset<>& lhs, const int& a);

    void filterSpecializations();

    void filterSpecializations(FDTree* filteredTree, boost::dynamic_bitset<> activePath);
};