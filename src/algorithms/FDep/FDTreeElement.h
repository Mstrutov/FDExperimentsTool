#pragma once

#include <memory>
#include <boost/dynamic_bitset.hpp>
#include <vector>

class FDTreeElement{
protected:
    static std::vector<FDTreeElement*> children;
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

    int getMaxAttrNumber();

    void addRhsAttribute(const int& index);

    boost::dynamic_bitset<> getRhsAttributes();

    void markAsLast(const int& index);

    bool isFinalNode(const int& a);

    bool getGeneralizationAndDelete
    (boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhs);

    bool containsGeneralization(boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr);

    bool getSpecialization
    (boost::dynamic_bitset<> lhs, const int& a, const int& currentAttr, boost::dynamic_bitset<> specLhsOut);

};