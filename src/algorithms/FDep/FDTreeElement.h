#pragma once

#include <memory>
#include <bitset>
#include <vector>

// For printing Dependencies
#include <string>
#include <fstream>
#include <iostream>

// TODO: Think about replacing global variable with something else
// The maximum number of columns in the dataset. Using in std::bitset template.
constexpr int kMaxAttrNum = 256;

class FDTreeElement{
 public:
    explicit FDTreeElement(const size_t& maxAttributeNumber);

    FDTreeElement (const FDTreeElement&) = delete;
    FDTreeElement& operator=(const FDTreeElement&) = delete;

    void addMostGeneralDependencies();

    // Using in cover-trees as post filtration of functional dependencies with redundant left-hand side.
    void filterSpecializations();

    [[nodiscard]] bool checkFd(const size_t& index) const;

    [[nodiscard]] FDTreeElement* getChild(const size_t& index) const;

    void addFunctionalDependency(const std::bitset<kMaxAttrNum>& lhs, const size_t& a);

    // Searching for generalization of functional dependency in cover-trees.
    bool getGeneralizationAndDelete(const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                                    const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhs);

    [[nodiscard]] bool containsGeneralization(const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                                              const size_t& currentAttr) const;

    // Printing found dependencies in output file.
    void printDep(const std::string& file, std::vector<std::string>& columnNames);
 private:
    std::vector<std::unique_ptr<FDTreeElement>> children_;
    std::bitset<kMaxAttrNum> rhsAttributes_;
    size_t maxAttributeNumber_;
    std::bitset<kMaxAttrNum> isFd_;

    void addRhsAttribute(const size_t& index);

    [[nodiscard]] const std::bitset<kMaxAttrNum>& getRhsAttributes() const;

    void markAsLast(const size_t& index);

    // Checking whether node is a leaf or not.
    [[nodiscard]] bool isFinalNode(const size_t& a) const;

    // Searching for specialization of functional dependency in cover-trees.
    bool getSpecialization (const std::bitset<kMaxAttrNum>& lhs, const size_t& a,
                            const size_t& currentAttr, std::bitset<kMaxAttrNum>& specLhsOut) const;

    void filterSpecializationsHelper(FDTreeElement& filteredTree, std::bitset<kMaxAttrNum>& activePath);

    // Helper function for printDep.
    void printDependencies(std::bitset<kMaxAttrNum>& activePath, std::ofstream& file,
                           std::vector<std::string>& columnNames);
};