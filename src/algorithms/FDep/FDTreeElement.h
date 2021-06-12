#pragma once

#include <memory>
#include <bitset>
#include <vector>

// For printing Dependencies
#include <string>
#include <fstream>
#include <iostream>

// The maximum number of columns in the dataset. Using in std::bitset template.
// TODO: Think about replacement global variable with something else
constexpr int kMaxAttrNum = 256;

class FDTreeElement{
 public:
    explicit FDTreeElement(const size_t& maxAttributeNumber): maxAttributeNumber_(maxAttributeNumber){
        children_.resize(maxAttributeNumber);
    }

    FDTreeElement (const FDTreeElement&) = delete;
    FDTreeElement& operator=(const FDTreeElement&) = delete;

    void addMostGeneralDependencies();

    void filterSpecializations();

    [[nodiscard]] bool checkFd(const size_t& index) const;

    [[nodiscard]] FDTreeElement* getChild(const size_t& index) const;

    void addFunctionalDependency(const std::bitset<kMaxAttrNum>& lhs, const size_t& a);

    bool getGeneralizationAndDelete(const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                                    const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhs);

    [[nodiscard]] bool containsGeneralization(const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                                              const size_t& currentAttr) const;

//    void printDep(const std::string& file, std::vector<std::string>& columnNames);
 private:
    std::vector<std::unique_ptr<FDTreeElement>> children_;
    std::bitset<kMaxAttrNum> rhsAttributes_;
    size_t maxAttributeNumber_;
    std::bitset<kMaxAttrNum> isfd_;

    void addRhsAttribute(const size_t& index);

    [[nodiscard]] const std::bitset<kMaxAttrNum>& getRhsAttributes() const;

    void markAsLast(const size_t& index);

    [[nodiscard]] bool isFinalNode(const size_t& a) const;

    bool getSpecialization (const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                            const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhsOut) const;

    void filterSpecializationsHelper(FDTreeElement& filteredTree, std::bitset<kMaxAttrNum>& activePath);
// TODO: Is there any way NOT to comment and uncomment PrintFunction all the time?

//    void printDependencies(std::bitset<kMaxAttrNum>& activePath,std::ofstream& file,
//                           std::vector<std::string>& columnNames);
};