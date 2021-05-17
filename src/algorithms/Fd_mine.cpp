#include "Fd_mine.h"

#include <map>
#include <queue>
#include <set>
#include <vector>

#include "ColumnLayoutRelationData.h"
#include "LatticeVertex.h"
#include "Vertical.h"
#include <boost/unordered_map.hpp>

unsigned long long Fd_mine::execute() {
    // 1
    relation = ColumnLayoutRelationData::createFrom(inputGenerator_, true);
    schema = relation->getSchema();
    auto startTime = std::chrono::system_clock::now();

    r = dynamic_bitset<>(schema->getNumColumns());

    for (size_t columnIndex = 0; columnIndex < schema->getNumColumns(); columnIndex++) {
        dynamic_bitset<> tmp(schema->getNumColumns());
        tmp[columnIndex] = 1;
        r[columnIndex] = 1;
        candidateSet.insert(std::move(tmp));
    }

    for (auto &xi : candidateSet) {
        closure[xi] = dynamic_bitset<>(schema->getNumColumns());
    }

    // 2

    while (!candidateSet.empty()) {
        for (auto &xi : candidateSet) {
            computeNonTrivialClosure(xi);
            obtainFDandKey(xi);
        }
        obtainEQSet();
        pruneCandidates();
        generateCandidates();
    }

    // 3
    display();

    std::chrono::milliseconds elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
    return elapsed_milliseconds.count();
}

void Fd_mine::computeNonTrivialClosure(dynamic_bitset<> xi) {
    if (!closure.count(xi)) {
        closure[xi] = dynamic_bitset<>(xi.size());
    }
    for (int columnIndex = 0; columnIndex < schema->getNumColumns(); columnIndex++) {
        if ((r - xi - closure[xi])[columnIndex]) {
            dynamic_bitset<> xiy = xi;
            dynamic_bitset<> y(schema->getNumColumns());
            xiy[columnIndex] = 1;
            y[columnIndex] = 1;
            shared_ptr<ColumnData> columnData;

            if (!plis.count(xi)) {
                columnData = relation->getColumnData(xi.find_first());
                plis[xi] = columnData->getPositionListIndex();
            }

            if (!plis.count(y)) {
                columnData = relation->getColumnData(columnIndex);
                plis[y] = columnData->getPositionListIndex();
            }

            if (!plis.count(xiy)) {
                plis[xiy] = plis[xi]->intersect(plis[y]);
            }

            if (plis[xi]->getNumCluster() == plis[xiy]->getNumCluster()) {
                closure[xi][columnIndex] = 1;
            }
        }
    }
}

void Fd_mine::obtainFDandKey(dynamic_bitset<> xi) {
    fdSet[xi] = closure[xi];
    if (r == (xi | closure[xi])) {
        keySet.insert(xi);
    }
}

void Fd_mine::obtainEQSet() {
    for (auto &xi : candidateSet) {
        for (auto &[x, xClosure] : fdSet) {
            dynamic_bitset<> z = xi & x;
            if ((xi - z).is_subset_of(xClosure) && (x - z).is_subset_of(closure[xi])) {
                if (x != xi) {
                    eqSet[x].insert(xi);
                    eqSet[xi].insert(x);
                }
            }
        }
    }
}

void Fd_mine::pruneCandidates() {
    std::set<dynamic_bitset<>>::iterator it = candidateSet.begin();
    while (it != candidateSet.end()) {
        bool found = false;
        const dynamic_bitset<> &xi = *it;

        for (auto &xj : eqSet[xi]) {
            if (candidateSet.find(xj) != candidateSet.end()) {
                it = candidateSet.erase(it);
                found = true;
                break;
            }
        }

        if (found) continue;

        if (keySet.find(xi) != keySet.end()) {
            it = candidateSet.erase(it);
            continue;
        }
        it++;
    }
}

void Fd_mine::generateCandidates() {
    std::vector<dynamic_bitset<>> candidates(candidateSet.begin(), candidateSet.end());

    dynamic_bitset<> xi;
    dynamic_bitset<> xj;
    dynamic_bitset<> xij;

    for (size_t i = 0; i < candidates.size(); i++) {
        xi = candidates[i];

        for (size_t j = i + 1; j < candidates.size(); j++) {
            xj = candidates[j];

            bool similar = true;
            int set_bits = 0;
            int set_bits_amount = xi.count();

            for (int k = 0; set_bits < set_bits_amount - 1; k++) {
                if(xi[k] == xj[k]) {
                    if(xi[k]) {
                        set_bits++;
                    }
                }
                else {
                    similar = false;
                    break;
                }
            }

            if (similar){
                xij = xi | xj;

                if (!(xj).is_subset_of(fdSet[xi]) && !(xi).is_subset_of(fdSet[xj])) {
                    if (!plis.count(xi)) {
                        shared_ptr<ColumnData> columnData =
                            relation->getColumnData(xi.find_first());
                        plis[xi] = columnData->getPositionListIndex();
                    }
                    if (!plis.count(xj)) {
                        shared_ptr<ColumnData> columnData =
                            relation->getColumnData(xj.find_first());
                        plis[xj] = columnData->getPositionListIndex();
                    }

                    plis[xij] = plis[xi]->intersect(plis[xj]);

                    dynamic_bitset<> closureXij = closure[xi] | closure[xj];
                    if (r == (xij | closureXij)) {
                        keySet.insert(xij);
                    } else {
                        candidateSet.insert(xij);
                    }
                }
            }
        }

        candidateSet.erase(xi);
    }
}

void Fd_mine::display() {
    int count_fd = 0;
    std::queue<dynamic_bitset<>> queue;
    dynamic_bitset<> generatedLhs(r.size());
    dynamic_bitset<> generatedLhs_tmp(r.size());
    boost::unordered_map<dynamic_bitset<>, dynamic_bitset<>> final_fdSet;

    for (const auto &[lhs, rhs] : fdSet) {
        std::unordered_map<dynamic_bitset<>, bool> observed;

        observed[lhs] = true;
        dynamic_bitset<> Rhs = rhs;
        queue.push(lhs);

        for (const auto &[eq, eqset] : eqSet) {
            if (eq.is_subset_of(Rhs)) {
                for (const auto &eqRhs : eqset) {
                    Rhs |= eqRhs;
                }
            }
        }
        bool rhsWillNotChange = false;

        while (!queue.empty()) {
            dynamic_bitset<> currentLhs = queue.front();
            queue.pop();
            int Rhs_count = Rhs.count();
            for (const auto &[eq, eqset] : eqSet) {
                if (!rhsWillNotChange  && eq.is_subset_of(Rhs)) {
                    for (const auto &eqRhs : eqset) {
                        Rhs |= eqRhs;
                    }
                }

                if (eq.is_subset_of(currentLhs)) {
                    generatedLhs_tmp = currentLhs - eq;
                    for (const auto &newEq : eqset) {
                        generatedLhs = generatedLhs_tmp;
                        generatedLhs |= newEq;

                        if (!observed[generatedLhs]) {
                            queue.push(generatedLhs);
                            observed[generatedLhs] = true;
                        }
                    }
                }
            }
            if (Rhs_count == Rhs.count()) {
                rhsWillNotChange  = true;
            }
        }

        for (auto &[lhs, rbool] : observed) {
            if (final_fdSet.count(lhs)) {
                final_fdSet[lhs] |= Rhs;
            }
            else {
                final_fdSet[lhs] = Rhs;
            }
        }
    }

    for (auto &[lhs, rhs] : final_fdSet) {
        for (size_t j = 0; j < rhs.size(); j++) {
            if (!rhs[j] || rhs[j] && lhs[j]) continue;
            std::cout << "Discovered FD: ";
            for (size_t i = 0; i < lhs.size(); i++) {
                if (lhs[i]) std::cout << schema->getColumn(i)->getName() << " ";
            }
            std::cout << "-> " << schema->getColumn(j)->getName() << std::endl;
            registerFD(Vertical(schema, lhs), *schema->getColumn(j));
            count_fd++;
        }
    }
    std::cout << "TOTAL FDs " << count_fd << std::endl;
}