#include "FDTreeElement.h"


bool FDTreeElement::checkFd(const int& i) const{
    return isfd[i];
};

FDTreeElement* FDTreeElement::getChild(const int& i) const{
    return children[i];
}

void FDTreeElement::addRhsAttribute(const int& i){
    this->rhsAttributes.set(i);
}

boost::dynamic_bitset<> FDTreeElement::getRhsAttributes(){
    return this->rhsAttributes;
}

void FDTreeElement::markAsLast(const int& i){
    isfd[i] = true;
}

int FDTreeElement::getMaxAttrNumber(){
    return this->maxAttributeNumber;
}

bool FDTreeElement::isFinalNode(const int& a){
    if (!this->rhsAttributes[a]){
        return false;
    }
    for (int attr = 0; attr < this->maxAttributeNumber; ++attr){
        if (children[attr]->maxAttributeNumber != 0 && children[attr]->getRhsAttributes()[a]){
            return false;
        }
    }
    return true;
}

bool FDTreeElement::containsGeneralization
(boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr){
    if (this->isfd[a - 1]){
        return true;
    }

    bool found = false;
    int nextSetAttr = lhs.find_next(currentAttr + 1);
    if (nextSetAttr < 0){
        return false;
    }

    if (this->children[nextSetAttr - 1]->maxAttributeNumber != 0 && this->children[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children[nextSetAttr - 1]->containsGeneralization(lhs, a, nextSetAttr);
    }

    if (!found){
        return this->containsGeneralization(lhs, a, nextSetAttr);
    }

    return true;
}

bool FDTreeElement::getGeneralizationAndDelete
(boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhs)
{
    bool found = false;
    if (this->isfd[a - 1]){
        this->isfd[a - 1] = false;
        this->rhsAttributes.reset(a);
        return true;
    }
    int nextSetAttr = lhs.find_next(currentAttr + 1);
    if (nextSetAttr < 0){     // УСЛОВИЕ ТОГО ЧТО ФАЙНД НЕКСТ НЕ СРАБОТАЕТ, ПРОВЕРИТЬ
        return false;
    }

    if (this->children[nextSetAttr - 1]->maxAttributeNumber != 0 && this->children[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children[nextSetAttr - 1]->getGeneralizationAndDelete(lhs, a, nextSetAttr, specLhs);
    
        if (found){
            if (this->isFinalNode(a)){
                this->rhsAttributes.reset(a);
            }
            specLhs.set(nextSetAttr);
        }
    }

    if (!found){
        found = this->getGeneralizationAndDelete(lhs, a, nextSetAttr, specLhs);
    }

    return found;
}

bool FDTreeElement::getSpecialization
(boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhsOut)
{
    if (!this->rhsAttributes[a]){
        return false;
    }

    bool found = false;
    int attr = std::max(currentAttr, 1);
    int nextSetAttr = lhs.find_next(currentAttr + 1);

    if (nextSetAttr < 0){
        while (!found && attr <= this->maxAttributeNumber){
            if (this->children[attr - 1]->maxAttributeNumber != 0 && this->children[attr - 1]->getRhsAttributes()[a]){
                found = this->children[attr - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);
            }
            ++attr;
        }
        if (found){
            specLhsOut.set(attr - 1);
        }
        return true;
    }

    while (!found && attr <= nextSetAttr){
        if (this->children[attr - 1]->maxAttributeNumber != 0 && this->children[attr - 1]->getRhsAttributes()[a]){
           found = this->children[std::min(attr, nextSetAttr) - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);
        }
        ++attr;
    }
    if (found){
        specLhsOut.set(attr - 1);
    }

    return found;
}