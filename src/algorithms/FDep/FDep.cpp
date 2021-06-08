#include "FDep.h"
#include "ColumnLayoutRelationData.h"

#include <iostream>

unsigned long long FDep::execute(){
    initialize();
    negativeCover();

    this->tuples.clear(); 

    this->posCoverTree = std::make_shared<FDTreeElement>(this->numberAttributes);
    this->posCoverTree->addMostGeneralDependencies();

    boost::dynamic_bitset<> activePath(this->numberAttributes + 1);
    calculatePositiveCover(*negCoverTree, activePath);

    boost::dynamic_bitset<> aPath(this->numberAttributes + 1);
    posCoverTree->printDependencies(aPath);

    return 0; 
}

void FDep::initialize(){
    loadData();
    //setColumnIndetifiers();
}

void FDep::negativeCover(){
    /* Building Negative Cover */
    this->negCoverTree = std::make_shared<FDTreeElement>(this->numberAttributes);
    for (size_t i = 0; i < tuples.size(); ++i)
        for (size_t j = i + 1; j < tuples.size(); ++j)
            violatedFDs(tuples[i], tuples[j]);

    this->negCoverTree->filterSpecializations();
}

void FDep::violatedFDs(const std::vector<int>& t1, const std::vector<int>& t2){
    /* Adding the least general dependencies, violated by t1 and t2 to Negative Cover*/
    boost::dynamic_bitset<> equalAttr(this->numberAttributes + 1);
    for (size_t i = 1; i <= this->numberAttributes; ++i)
        equalAttr.set(i);
    boost::dynamic_bitset<> diffAttr(this->numberAttributes + 1);

    for (size_t attr = 0; attr < this->numberAttributes; ++attr){
        if (t1[attr] != t2[attr])
            diffAttr.set(attr + 1);
    }

    equalAttr = equalAttr - diffAttr;
    for (size_t attr = diffAttr.find_first(); attr != boost::dynamic_bitset<>::npos; attr = diffAttr.find_next(attr)){
        this->negCoverTree->addFunctionalDependency(equalAttr, attr);
    }
}

void FDep::calculatePositiveCover(FDTreeElement const& negCoverSubtree, boost::dynamic_bitset<> activePath){
    /* Building Positive Cover from Negative*/

    for (size_t attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.checkFd(attr - 1)){
            this->specializePositiveCover(activePath, attr);
        }
    }

    for (size_t attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.getChild(attr - 1)){
            activePath.set(attr);
            this->calculatePositiveCover(*negCoverSubtree.getChild(attr - 1), activePath);
            activePath.reset(attr);
        }
    }

}

void FDep::specializePositiveCover(const boost::dynamic_bitset<>& lhs, const size_t& a){
    boost::dynamic_bitset<> specLhs(this->numberAttributes + 1);

    while (this->posCoverTree->getGeneralizationAndDelete(lhs, a, 0, specLhs))
    {

        for (size_t attr = this->numberAttributes; attr > 0; --attr){
            if (!lhs.test(attr) && (attr != a)){
                specLhs.set(attr);
                if (!this->posCoverTree->containsGeneralization(specLhs, a, 0)){
                    this->posCoverTree->addFunctionalDependency(specLhs, a);
                }
                specLhs.reset(attr);
            }
        }

        specLhs.clear();
        specLhs.resize(this->numberAttributes + 1);
    }
}


void FDep::loadData(){
    const int numberRows = 4;
    this->numberAttributes = 4;

    this->tuples.resize(numberRows);
    tuples[0] = std::vector<int>{0, 0, 0, 0};
    tuples[1] = std::vector<int>{1, 1, 0, 0};
    tuples[2] = std::vector<int>{0, 2, 0, 2};   
    tuples[3] = std::vector<int>{1, 2, 3, 4};
}