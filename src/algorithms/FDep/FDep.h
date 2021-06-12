#pragma once

#include "CSVParser.h"
#include "FDAlgorithm.h"
#include "RelationData.h"
#include "FDTreeElement.h"

#include <vector> 
#include <string>

class FDep : public FDAlgorithm {
 public:
    explicit FDep(std::filesystem::path const& path, char separator = ',', bool hasHeader = true):
            FDAlgorithm(path, separator, hasHeader){}

    ~FDep() override = default;

    unsigned long long execute() override;
 private:
    std::vector <std::string> columnNames_;
    size_t numberAttributes_{};

    FDTreeElement* negCoverTree_{};
    FDTreeElement* posCoverTree_{};
    
    std::vector<std::vector<size_t>> tuples_;

    void initialize();
    void negativeCover();

    void violatedFDs(const std::vector<size_t>& t1, const std::vector<size_t>& t2);

    void calculatePositiveCover(FDTreeElement const& negCoverSubtree, std::bitset<kMaxAttrNum>& activePath);
    void specializePositiveCover(const std::bitset<kMaxAttrNum>& lhs, const size_t& a);

    void loadData();
};