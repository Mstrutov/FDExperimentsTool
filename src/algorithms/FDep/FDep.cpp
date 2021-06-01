#include "FDep.h"
#include "ColumnLayoutRelationData.h"

unsigned long long FDep::execute(){
    initialize();
    negativeCover();

    this->tuples.clear(); 

    this->posCoverTree = std::make_unique<FDTree>(this->numberAttributes);
    posCoverTree->addMostGeneralDependencies();

    boost::dynamic_bitset<> activePath;
    calculatePositiveCover(*negCoverTree, activePath);

    return 0; 
}

void FDep::initialize(){
    loadData();
    //setColumnIndetifiers();
}

void FDep::negativeCover(){
    /* Building Negative Cover */
    this->negCoverTree = std::make_unique<FDTree>(this->numberAttributes);
    for (int i = 0; i < tuples.size(); ++i)
        for (int j = i + 1; j < tuples.size(); ++j)
            violatedFDs(tuples[i], tuples[j]);

    this->negCoverTree->filterSpecializations();
}

void FDep::violatedFDs(const std::vector<int>& t1, const std::vector<int>& t2){
    /* Adding the least general dependencies, violated by t1 and t2 to Negative Cover*/
    boost::dynamic_bitset<> equalAttr(this->numberAttributes + 1);
    for (int i = 1; i < this->numberAttributes + 1; ++i)
        equalAttr.set(i);
    boost::dynamic_bitset<> diffAttr(this->numberAttributes + 1);

    for (int i = 0; i < t1.size(); ++i){
        diffAttr[i + 1] = t1[i] != t2[i];
    }

    equalAttr = equalAttr - diffAttr;
    for (int attr = diffAttr.find_first(); attr != boost::dynamic_bitset<>::npos; attr = diffAttr.find_next(attr)){
        this->negCoverTree->addFunctionalDependency(equalAttr, attr);
    }
}

void FDep::calculatePositiveCover(FDTreeElement const& negCoverSubtree, boost::dynamic_bitset<> activePath){
    /* Building Positive Cover from Negative*/
    for (int attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.checkFd(attr - 1)){
            this->specializePositiveCover(activePath, attr);
        }
    }

    for (int attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.getChild(attr - 1)->getMaxAttrNumber() != 0){
            activePath.set(attr);
            this->calculatePositiveCover(*negCoverSubtree.getChild(attr - 1), activePath);
            activePath.reset(attr);
        }
    }

}

void FDep::specializePositiveCover(boost::dynamic_bitset<> lhs, const int& a){
    boost::dynamic_bitset<> specLhs;

    while (posCoverTree->getGeneralizationAndDelete(lhs, a, 0, specLhs)){
        for (int attr = this->numberAttributes; attr > 0; --attr){
            if (!lhs[attr] && attr != a){
                specLhs.set(attr);
                if (!posCoverTree->containsGeneralization(specLhs, a, 0))
                    posCoverTree->addFunctionalDependency(specLhs, a);
                specLhs.reset(attr);
            }
        }
        specLhs.clear();
    }
}

void FDep::loadData(){
    std::unique_ptr<ColumnLayoutRelationData> relation = ColumnLayoutRelationData::createFrom(inputGenerator_, true);

    this->numberAttributes = relation->getNumColumns();
    this->numberRows = relation->getNumRows();
    this->tuples.resize(numberRows);
    
    for (int i = 0; i < numberRows; ++i){
        tuples[i] = relation->getTuple(i);
    }
}