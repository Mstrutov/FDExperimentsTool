#include "FDep.h"
#include "ColumnLayoutRelationData.h"

#include <iostream>
#include <chrono>

unsigned long long FDep::execute(){

    initialize();

    auto startTime = std::chrono::system_clock::now();

    negativeCover();

    this->tuples.clear(); 

    this->posCoverTree = std::make_shared<FDTreeElement>(this->numberAttributes);
    this->posCoverTree->addMostGeneralDependencies();

    boost::dynamic_bitset<> activePath(this->numberAttributes + 1);
    calculatePositiveCover(*negCoverTree, activePath);

    std::chrono::milliseconds elapsed_milliseconds = 
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);

    std::cout << "Ellapsed Time: " << elapsed_milliseconds.count() << std::endl;

    boost::dynamic_bitset<> aPath(this->numberAttributes + 1);
    posCoverTree->printDependencies(aPath);

    return elapsed_milliseconds.count(); 
}

void FDep::initialize(){
    loadData();
    //setColumnIndetifiers();
}

void FDep::negativeCover(){
    /* Building Negative Cover */
    this->negCoverTree = std::make_shared<FDTreeElement>(this->numberAttributes);
    for (size_t i = 0; i < tuples.size(); ++i)
        for (size_t j = i + 1; j < tuples.size(); ++j)
            violatedFDs(tuples[i], tuples[j]);

    this->negCoverTree->filterSpecializations();
}

void FDep::violatedFDs(const std::vector<size_t>& t1, const std::vector<size_t>& t2){
    /* Adding the least general dependencies, violated by t1 and t2 to Negative Cover*/
    boost::dynamic_bitset<> equalAttr(this->numberAttributes + 1);
    for (size_t i = 1; i <= this->numberAttributes; ++i)
        equalAttr.set(i);
    boost::dynamic_bitset<> diffAttr(this->numberAttributes + 1);

    for (size_t attr = 0; attr < this->numberAttributes; ++attr){
        if (t1[attr] != t2[attr])
            diffAttr.set(attr + 1);
    }

    equalAttr = equalAttr - diffAttr;
    for (size_t attr = diffAttr.find_first(); attr != boost::dynamic_bitset<>::npos; attr = diffAttr.find_next(attr)){
        this->negCoverTree->addFunctionalDependency(equalAttr, attr);
    }
}

void FDep::calculatePositiveCover(FDTreeElement const& negCoverSubtree, boost::dynamic_bitset<> activePath){
    /* Building Positive Cover from Negative*/

    for (size_t attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.checkFd(attr - 1)){
            this->specializePositiveCover(activePath, attr);
        }
    }

    for (size_t attr = 1; attr <= this->numberAttributes; ++attr){
        if (negCoverSubtree.getChild(attr - 1)){
            activePath.set(attr);
            this->calculatePositiveCover(*negCoverSubtree.getChild(attr - 1), activePath);
            activePath.reset(attr);
        }
    }

}

void FDep::specializePositiveCover(const boost::dynamic_bitset<>& lhs, const size_t& a){
    boost::dynamic_bitset<> specLhs(this->numberAttributes + 1);

    while (this->posCoverTree->getGeneralizationAndDelete(lhs, a, 0, specLhs))
    {

        for (size_t attr = this->numberAttributes; attr > 0; --attr){
            if (!lhs.test(attr) && (attr != a)){
                specLhs.set(attr);
                if (!this->posCoverTree->containsGeneralization(specLhs, a, 0)){
                    this->posCoverTree->addFunctionalDependency(specLhs, a);
                }
                specLhs.reset(attr);
            }
        }

        specLhs.clear();
        specLhs.resize(this->numberAttributes + 1);
    }
}


void FDep::loadData(){
    this->numberAttributes = inputGenerator_.getNumberOfColumns();
    this->columnNames.resize(this->numberAttributes);

    for (size_t i = 0; i < this->numberAttributes; ++i){
        columnNames[i] = inputGenerator_.getColumnName(i);
    }

    std::vector<std::string> nextLine; 
    while (inputGenerator_.getHasNext()){
        nextLine = inputGenerator_.parseNext();
        this->tuples.push_back(std::vector<size_t>(this->numberAttributes));
        for (size_t i = 0; i < this->numberAttributes; ++i){
            this->tuples.back()[i] = std::hash<std::string>{}(nextLine[i]);
        }
    } 

}