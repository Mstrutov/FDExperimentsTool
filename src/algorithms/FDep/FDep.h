#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector> 
#include <string>

#include "CSVParser.h"
#include "FDAlgorithm.h"
#include "RelationData.h"
#include "FDTreeElement.h"
#include "FDTree.h"

class FDep : public FDAlgorithm {
private:
    std::vector <std::string> columnNames;
    int numberAttributes=0;
    int numberRows=0;

    std::unique_ptr<FDTree> negCoverTree;
    std::unique_ptr<FDTree> posCoverTree;
    
    std::vector<std::vector<int>> tuples;

    void initialize();
    void negativeCover();

    void violatedFDs(const std::vector<int>& t1, const std::vector<int>& t2);

    void calculatePositiveCover(FDTreeElement const& negCoverSubtree, boost::dynamic_bitset<> activePath);
    void specializePositiveCover(boost::dynamic_bitset<> lhs, const int& a);

    void loadData();
public:

    explicit FDep(std::filesystem::path const& path, char separator = ',', bool hasHeader = true):
        FDAlgorithm(path, separator, hasHeader){}
    ~FDep() override {};

    unsigned long long execute() override;
};