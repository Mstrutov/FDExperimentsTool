#include "FDTree.h"

void FDTree::addMostGeneralDependencies(){
//  Warning! This might be weak point. Check carefully
    this->rhsAttributes.resize(this->maxAttributeNumber + 1);
    for (int i = 0; i < this->maxAttributeNumber; ++i){
        isfd[i] = true;
    }
}

void FDTree::addFunctionalDependency(const boost::dynamic_bitset<>& lhs, const int& a){

    FDTree* currentNode = this;
    currentNode->addRhsAttribute(a);

    for (int i = lhs.find_first(); i >= 0; i = lhs.find_next(i)){
        if (currentNode->children[i - 1]->getMaxAttrNumber() == 0){
            FDTreeElement* treeElement = new FDTreeElement(maxAttributeNumber);
            currentNode->children[i - 1] = treeElement;
        }

        currentNode = static_cast<FDTree*>(currentNode->getChild(i - 1));
        currentNode->addRhsAttribute(a);
    }
    currentNode->markAsLast(a - 1);
}

void FDTree::filterSpecializations(){
    boost::dynamic_bitset<> activePath;
    FDTree* filteredTree = new FDTree(this->maxAttributeNumber);
    this->filterSpecializations(filteredTree, activePath);

    this->children = filteredTree->children;
    this->isfd = filteredTree->isfd;
}

void FDTree::filterSpecializations(FDTree* filteredTree, boost::dynamic_bitset<> activePath){
    for (int attr = 1; attr <= maxAttributeNumber; ++attr){
        if (this->children[attr - 1]->getMaxAttrNumber() != 0){
            activePath.set(attr);
            static_cast<FDTree*> (this->children[attr - 1])->filterSpecializations(filteredTree, activePath);
            activePath.reset(attr);
        }
    }

    for (int attr = 1; attr <= maxAttributeNumber; ++attr){
        boost::dynamic_bitset<> tmp;
        if (this->isfd[attr - 1] && !filteredTree->getSpecialization(activePath, attr, 0, tmp)){
            filteredTree->addFunctionalDependency(activePath, attr);
        }
    }
}