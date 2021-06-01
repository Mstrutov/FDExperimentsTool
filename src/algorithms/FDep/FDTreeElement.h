#pragma once

#include <memory>
#include <boost/dynamic_bitset.hpp>
#include <vector>

class FDTreeElement{
protected:
    std::vector<FDTreeElement*> children;
    boost::dynamic_bitset<> rhsAttributes;
    std::vector<bool> isfd; 
    int maxAttributeNumber;
public:
    explicit FDTreeElement(const int& maxAttributeNumber=0): maxAttributeNumber(maxAttributeNumber){
        this->children.resize(maxAttributeNumber);
        this->isfd.resize(maxAttributeNumber);
    };
    
    bool checkFd(const int& index) const;

    FDTreeElement* getChild(const int& index) const;

    int getMaxAttrNumber() const;

    void addRhsAttribute(const int& index);

    boost::dynamic_bitset<> getRhsAttributes() const;

    void markAsLast(const int& index);

    bool isFinalNode(const int& a) const;

    bool getGeneralizationAndDelete
    (const boost::dynamic_bitset<>& lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhs);

    bool containsGeneralization(const boost::dynamic_bitset<>& lhs, const int& a, const int& currentAttr) const;

    bool getSpecialization
    (const boost::dynamic_bitset<>& lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhsOut) const;

};