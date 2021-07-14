#pragma once

#include "CSVParser.h"
#include "FDAlgorithm.h"
#include "RelationData.h"
#include "FDTreeElement.h"

#include <vector> 
#include <string>

class FDep : public FDAlgorithm {
 public:
    explicit FDep(std::filesystem::path const& path, char separator = ',', bool hasHeader = true);

    ~FDep() override = default;

    unsigned long long execute() override;
 private:
    std::vector <std::string> columnNames_;
    size_t numberAttributes_{};

    FDTreeElement* negCoverTree_{};
    FDTreeElement* posCoverTree_{};
    
    std::vector<std::vector<size_t>> tuples_;

    // Initializing the most common dependencies.
    void initialize();

    // Building negative cover via violated dependencies
    void negativeCover();

    // Iterating over all pairs t1 and t2 of the relation
    // Adding violated FDs to negative cover tree.
    void violatedFDs(const std::vector<size_t>& t1, const std::vector<size_t>& t2);

    // Converting negative cover tree into positive cover tree
    void calculatePositiveCover(FDTreeElement const& negCoverSubtree, std::bitset<kMaxAttrNum>& activePath);

    // Specializing general dependencies for not to be followed from violated dependencies of negative cover tree.
    void specializePositiveCover(const std::bitset<kMaxAttrNum>& lhs, const size_t& a);

    // Loading the relation
    // Presented as vector of vectors (tuples of the relation).
    void loadData();
};