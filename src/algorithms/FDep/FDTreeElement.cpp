#include "FDTreeElement.h"

bool FDTreeElement::checkFd(const size_t& i) const{
    return this->isfd_[i];
}

FDTreeElement* FDTreeElement::getChild(const size_t& i) const{
    return this->children_[i].get();
}

void FDTreeElement::addRhsAttribute(const size_t& i){
    this->rhsAttributes_.set(i);
}

const std::bitset<kMaxAttrNum>& FDTreeElement::getRhsAttributes() const{
    return this->rhsAttributes_;
}

void FDTreeElement::markAsLast(const size_t& i){
    this->isfd_.set(i);
}

bool FDTreeElement::isFinalNode(const size_t& a) const{
    if (!this->rhsAttributes_[a]){
        return false;
    }
    for (size_t attr = 0; attr < this->maxAttributeNumber_; ++attr){
        if (children_[attr] && children_[attr]->getRhsAttributes()[a]){
            return false;
        }
    }
    return true;
}

bool FDTreeElement::containsGeneralization
(const std::bitset<kMaxAttrNum>& lhs, const size_t& a, const size_t& currentAttr) const
{
    if (this->isfd_[a - 1]){
        return true;
    }
    
    size_t nextSetAttr = lhs._Find_next(currentAttr);
    if (nextSetAttr == kMaxAttrNum){
        return false;
    }
    bool found = false;
    if (this->children_[nextSetAttr - 1] && this->children_[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children_[nextSetAttr - 1]->containsGeneralization(lhs, a, nextSetAttr);
    }

    if (found){
        return true;
    }
    return this->containsGeneralization(lhs, a, nextSetAttr);
}

bool FDTreeElement::getGeneralizationAndDelete
(const std::bitset<kMaxAttrNum>& lhs, const size_t& a, const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhs)
{
    if (this->isfd_[a - 1]){
        this->isfd_.reset(a - 1);
        this->rhsAttributes_.reset(a);
        return true;
    }

    size_t nextSetAttr = lhs._Find_next(currentAttr);
    if (nextSetAttr == kMaxAttrNum){
        return false;
    }

    bool found = false;
    if (this->children_[nextSetAttr - 1] && this->children_[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children_[nextSetAttr - 1]->getGeneralizationAndDelete(lhs, a, nextSetAttr, specLhs);
        if (found){
            if (this->isFinalNode(a)){
                this->rhsAttributes_.reset(a);
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
(const std::bitset<kMaxAttrNum>& lhs, const size_t& a, const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhsOut) const
{
    if (!this->rhsAttributes_[a]){
        return false;
    }

    bool found = false;
    size_t attr = (currentAttr > 1 ? currentAttr : 1);
    size_t nextSetAttr = lhs._Find_next(currentAttr);

    if (nextSetAttr == kMaxAttrNum){
        while (!found && attr <= this->maxAttributeNumber_){
            if (this->children_[attr - 1] && this->children_[attr - 1]->getRhsAttributes()[a]){
                found = this->children_[attr - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);
            }
            ++attr;
        }
        if (found){
            specLhsOut.set(attr - 1);
        }
        return true;
    }

    while (!found && attr < nextSetAttr){
        if (this->children_[attr - 1] && this->children_[attr - 1]->getRhsAttributes()[a]){
            found = this->children_[attr - 1]->getSpecialization(lhs, a, currentAttr, specLhsOut);   
        }
        ++attr;
    }
    if (!found && this->children_[nextSetAttr - 1] && this->children_[nextSetAttr - 1]->getRhsAttributes()[a]){
        found = this->children_[nextSetAttr - 1]->getSpecialization(lhs, a, nextSetAttr, specLhsOut);
    }

    specLhsOut.set(attr - 1, found);

    return found;
}

void FDTreeElement::addMostGeneralDependencies(){
    for (size_t i = 1; i <= this->maxAttributeNumber_; ++i){
        this->rhsAttributes_.set(i);
    }

    for (size_t i = 0; i < this->maxAttributeNumber_; ++i){
        this->isfd_[i] = true;
    }
}

void FDTreeElement::addFunctionalDependency(const std::bitset<kMaxAttrNum>& lhs, const size_t& a){
    FDTreeElement* currentNode = this;
    this->addRhsAttribute(a);

    for (size_t i = lhs._Find_first(); i != kMaxAttrNum; i = lhs._Find_next(i)){
        if (currentNode->children_[i - 1] == nullptr){
            currentNode->children_[i - 1] = std::make_unique<FDTreeElement>(this->maxAttributeNumber_);
        }

        currentNode = currentNode->getChild(i - 1);
        currentNode->addRhsAttribute(a);
    }

    currentNode->markAsLast(a - 1);
}

void FDTreeElement::filterSpecializations(){
    std::bitset<kMaxAttrNum> activePath;
    std::unique_ptr<FDTreeElement> filteredTree = std::make_unique<FDTreeElement>(this->maxAttributeNumber_);

    this->filterSpecializationsHelper(*filteredTree, activePath);

    this->children_ = std::move(filteredTree->children_);
    this->isfd_ = filteredTree->isfd_;
}

void FDTreeElement::filterSpecializationsHelper(FDTreeElement& filteredTree, std::bitset<kMaxAttrNum>& activePath){
    for (size_t attr = 1; attr <= this->maxAttributeNumber_; ++attr){
        if (this->children_[attr - 1]){
            activePath.set(attr);
            this->children_[attr - 1]->filterSpecializationsHelper(filteredTree, activePath);
            activePath.reset(attr);
        }
    }

    for (size_t attr = 1; attr <= this->maxAttributeNumber_; ++attr){
        std::bitset<kMaxAttrNum> specLhsOut; 
        if (this->isfd_[attr - 1] && !filteredTree.getSpecialization(activePath, attr, 0, specLhsOut)){
            filteredTree.addFunctionalDependency(activePath, attr);
        }
    }
}

// void FDTreeElement::printDep(const std::string& fileName, std::vector<std::string>& columnNames){
//     std::ofstream file;
//     file.open(fileName);
//     std::bitset<kMaxAttrNum> activePath;
//     printDependencies(activePath, file, columnNames);
//     file.close();
// }

// void FDTreeElement::printDependencies(std::bitset<kMaxAttrNum>& activePath, std::ofstream& file,
// std::vector<std::string>& columnNames) {
//     std::string columnId = "";
//     if (std::isdigit(columnNames[0][0])){
//         columnId = "column";
//     }
//     std::string out;
//     for (size_t attr = 1; attr <= this->maxAttributeNumber_; ++attr){
//         if (this->isfd_[attr - 1]){
//             out = "{";

//             for (size_t i = activePath._Find_first(); i != kMaxAttrNum; i = activePath._Find_next(i)){
//                 if (!columnId.empty())
//                     out += columnId + std::to_string(std::stoi(columnNames[i - 1]) + 1) + ",";
//                 else
//                     out += columnNames[i - 1] + ",";
//             }

//             if (out.size() > 1){
//                 out = out.substr(0, out.size() - 1);
//             }
//             if (!columnId.empty())
//                 out += "} -> " + columnId + std::to_string(std::stoi(columnNames[attr - 1]) + 1);
//             else
//                 out += "} -> " + columnId + columnNames[attr - 1];
//             file << out << std::endl;
//         }
//     }

//     for (size_t attr = 1; attr <= this->maxAttributeNumber_; ++attr){
//         if (this->children_[attr - 1]){
//             activePath.set(attr);
//             this->children_[attr - 1]->printDependencies(activePath, file, columnNames);
//             activePath.reset(attr);
//         }
//     }

// }