#include "FDTreeElement.h"

bool FDTreeElement::checkFd(const size_t& i) const{
    return this->isfd[i];
}

FDTreeElement* FDTreeElement::getChild(const size_t& i) const{
    return this->children[i].get();
}

void FDTreeElement::addRhsAttribute(const size_t& i){
    this->rhsAttributes.set(i);
}

const boost::dynamic_bitset<>& FDTreeElement::getRhsAttributes() const{
    return this->rhsAttributes;
}

void FDTreeElement::markAsLast(const size_t& i){
    this->isfd.set(i);
}

bool FDTreeElement::isFinalNode(const size_t& a) const{
    if (!this->rhsAttributes[a]){
        return false;
    }
    for (size_t attr = 0; attr < this->maxAttributeNumber; ++attr){
        if (children[attr] && children[attr]->getRhsAttributes()[a]){
            return false;
        }
    }
    return true;
}

bool FDTreeElement::containsGeneralization
(const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr) const
{
    if (this->isfd[a - 1]){
        return true;
    }
    
    size_t nextSetAttr = lhs.find_next(currentAttr);
    if (nextSetAttr == boost::dynamic_bitset<>::npos){
        return false;
    }
    bool found = false;
    if (this->children[nextSetAttr - 1] && this->children[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children[nextSetAttr - 1]->containsGeneralization(lhs, a, nextSetAttr);
    }

    if (found){
        return true;
    }
    return this->containsGeneralization(lhs, a, nextSetAttr);
}

bool FDTreeElement::getGeneralizationAndDelete
(const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr, boost::dynamic_bitset<>& specLhs)
{
    if (this->isfd[a - 1]){
        this->isfd.reset(a - 1);
        this->rhsAttributes.reset(a);
        return true;
    }

    size_t nextSetAttr = lhs.find_next(currentAttr);
    if (nextSetAttr == boost::dynamic_bitset<>::npos){
        return false;
    }

    bool found = false;
    if (this->children[nextSetAttr - 1] && this->children[nextSetAttr - 1]->getRhsAttributes()[a]){
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
(const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr, boost::dynamic_bitset<>& specLhsOut) const
{
    if (!this->rhsAttributes[a]){
        return false;
    }

    bool found = false;
    size_t attr = (currentAttr > 1 ? currentAttr : 1);
    size_t nextSetAttr = lhs.find_next(currentAttr);

    if (nextSetAttr == boost::dynamic_bitset<>::npos){
        while (!found && attr <= this->maxAttributeNumber){
            if (this->children[attr - 1] && this->children[attr - 1]->getRhsAttributes()[a]){
                found = this->children[attr - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);
            }
            ++attr;
        }
        if (found){
            specLhsOut.set(attr - 1);
        }
        return true;
    }

    while (!found && attr < nextSetAttr){
        if (this->children[attr - 1] && this->children[attr - 1]->getRhsAttributes()[a]){
            found = this->children[attr - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);   
        }
        ++attr;
    }
    if (!found && this->children[nextSetAttr - 1] && this->children[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children[nextSetAttr - 1]->getSpecialization(lhs, a, nextSetAttr, specLhsOut);
    }

    specLhsOut.set(attr - 1, found);

    return found;
}

void FDTreeElement::addMostGeneralDependencies(){
    for (size_t i = 1; i <= this->maxAttributeNumber; ++i){
        this->rhsAttributes.set(i);
    }

    for (size_t i = 0; i < this->maxAttributeNumber; ++i){
        this->isfd[i] = true;
    }
}

void FDTreeElement::addFunctionalDependency(const boost::dynamic_bitset<>& lhs, const size_t& a){
    FDTreeElement* currentNode = this;
    this->addRhsAttribute(a);

    for (size_t i = lhs.find_first(); i != boost::dynamic_bitset<>::npos; i = lhs.find_next(i)){
        if (currentNode->children[i - 1] == nullptr){
            currentNode->children[i - 1] = std::make_unique<FDTreeElement>(this->maxAttributeNumber);
        }

        currentNode = currentNode->getChild(i - 1);
        currentNode->addRhsAttribute(a);
    }

    currentNode->markAsLast(a - 1);
}

void FDTreeElement::filterSpecializations(){
    boost::dynamic_bitset<> activePath(this->maxAttributeNumber + 1);
    std::unique_ptr<FDTreeElement> filteredTree = std::make_unique<FDTreeElement>(this->maxAttributeNumber);

    this->filterSpecializationsHelper(*filteredTree, activePath);

    this->children = std::move(filteredTree->children);
    this->isfd = std::move(filteredTree->isfd);
}

void FDTreeElement::filterSpecializationsHelper(FDTreeElement& filteredTree, boost::dynamic_bitset<>& activePath){
    for (size_t attr = 1; attr <= this->maxAttributeNumber; ++attr){
        if (this->children[attr - 1]){
            activePath.set(attr);
            this->children[attr - 1]->filterSpecializationsHelper(filteredTree, activePath);
            activePath.reset(attr);
        }
    }

    for (size_t attr = 1; attr <= this->maxAttributeNumber; ++attr){
        boost::dynamic_bitset<> specLhsOut(this->maxAttributeNumber + 1); 
        if (this->isfd[attr - 1] && !filteredTree.getSpecialization(activePath, attr, 0, specLhsOut)){
            filteredTree.addFunctionalDependency(activePath, attr);
        }
    }
}

void FDTreeElement::printDependencies(boost::dynamic_bitset<>& activePath) {
    std::string out;
    for (size_t attr = 1; attr <= this->maxAttributeNumber; ++attr){
        if (this->isfd[attr - 1]){
            out = "{";

            for (size_t i = activePath.find_first(); i != boost::dynamic_bitset<>::npos; i = activePath.find_next(i)){
                out += std::to_string(i) + ",";
            }

            if (out.size() > 1){
                out = out.substr(0, out.size() - 1);
            }

            out += "} -> " + std::to_string(attr);
            std::cout << out << std::endl;
        }
    }

    for (size_t attr = 1; attr <= this->maxAttributeNumber; ++attr){
        if (this->children[attr - 1]){
            activePath.set(attr);
            this->children[attr - 1]->printDependencies(activePath);
            activePath.reset(attr);
        }
    }

}