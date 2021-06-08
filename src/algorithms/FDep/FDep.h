#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector> 
#include <string>

#include "CSVParser.h"
#include "FDAlgorithm.h"
#include "RelationData.h"
#include "FDTreeElement.h"

class FDep : public FDAlgorithm {
private:
    std::vector <std::string> columnNames;
    size_t numberAttributes=0;

    std::shared_ptr<FDTreeElement> negCoverTree;
    std::shared_ptr<FDTreeElement> posCoverTree;
    
    std::vector<std::vector<size_t>> tuples;

    void initialize();
    void negativeCover();

    void violatedFDs(const std::vector<size_t>& t1, const std::vector<size_t>& t2);

    void calculatePositiveCover(FDTreeElement const& negCoverSubtree, boost::dynamic_bitset<>& activePath);
    void specializePositiveCover(const boost::dynamic_bitset<>& lhs, const size_t& a);

    void loadData();
public:

    explicit FDep(std::filesystem::path const& path, char separator = ',', bool hasHeader = true):
        FDAlgorithm(path, separator, hasHeader){}
    ~FDep() override {};

    unsigned long long execute() override;
};