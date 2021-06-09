#pragma once

#include <memory>
#include <boost/dynamic_bitset.hpp>
#include <vector>

// For printing Dependencies
#include <string>
#include <iostream>

class FDTreeElement{
private:
    std::vector<std::unique_ptr<FDTreeElement>> children;
    boost::dynamic_bitset<> rhsAttributes;
    size_t maxAttributeNumber;
    boost::dynamic_bitset<> isfd; 
public:
    explicit FDTreeElement(const size_t& maxAttributeNumber): maxAttributeNumber(maxAttributeNumber){
        children.resize(maxAttributeNumber);
        isfd.resize(this->maxAttributeNumber);
        rhsAttributes.resize(maxAttributeNumber + 1);
    }

    FDTreeElement (const FDTreeElement&) = delete;
    FDTreeElement& operator=(const FDTreeElement&) = delete;

    bool checkFd(const size_t& index) const;

    FDTreeElement* getChild(const size_t& index) const;

    void addRhsAttribute(const size_t& index);

    const boost::dynamic_bitset<>& getRhsAttributes() const;

    void markAsLast(const size_t& index);

    bool isFinalNode(const size_t& a) const;

    bool getGeneralizationAndDelete
    (const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr, boost::dynamic_bitset<>& specLhs);

    bool containsGeneralization(const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr) const;

    bool getSpecialization
    (const boost::dynamic_bitset<>& lhs, const size_t& a, const size_t& currentAttr, boost::dynamic_bitset<>& specLhsOut) const;

    void addMostGeneralDependencies();

    void addFunctionalDependency(const boost::dynamic_bitset<>& lhs, const size_t& a);

    void filterSpecializations();

    void filterSpecializationsHelper(FDTreeElement& filteredTree, boost::dynamic_bitset<>& activePath);

    void printDependencies(boost::dynamic_bitset<>& activePath);
};