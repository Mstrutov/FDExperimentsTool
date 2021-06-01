#include "FDTree.h"

void FDTree::addMostGeneralDependencies(){
    this->rhsAttributes.resize(this->maxAttributeNumber + 1);
    for (int i = 0; i < this->maxAttributeNumber; ++i){
        isfd[i] = true;
    }
}

void FDTree::addFunctionalDependency(boost::dynamic_bitset<> lhs, const int& a){
    FDTreeElement* treeElement;

    FDTreeElement* currentNode = this;
    currentNode->addRhsAttribute(a);
    
    for (int i = lhs.find_next(0); i >= 0; i = lhs.find_next(i + 1)){
        if (currentNode->children[i - 1]->getMaxAttrNumber() == 0){
            treeElement = new FDTreeElement(maxAttributeNumber);
            currentNode->children[i - 1] = treeElement;
        }

        currentNode = currentNode->getChild(i - 1);
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


    // !!! Обязательно глянуть корректно ли работает dynamic_bitset<>::find_next(). 
    // Ибо кажется, что там разница в знаке > || >=  ???


    // Возможно find_next(0) стоит заменить на find_first()
    // А find_next(i + 1) замениь на find_next(i)

