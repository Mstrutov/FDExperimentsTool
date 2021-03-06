#include "AgreeSetFactory.h"

#include <unordered_set>

#include "IdentifierSet.h"
#include "logging/easylogging++.h"

using std::set, std::vector, std::unordered_set;

template<AgreeSetsGenMethod method>
AgreeSetFactory::SetOfAgreeSets AgreeSetFactory::genAgreeSets() const {
    auto start_time = std::chrono::system_clock::now();
    std::string method_str;
    SetOfAgreeSets agree_sets;

    if constexpr (method == AgreeSetsGenMethod::kUsingVectorOfIDSets) {
        method_str = "`kUsingVectorOfIDSets`";
        vector<IdentifierSet> identifier_sets;
        SetOfVectors const max_representation = genPLIMaxRepresentation();

        auto start_time = std::chrono::system_clock::now();

        // compute identifier sets
        // identifier_sets is vector
        std::unordered_set<int> cache;
        for (auto const& cluster : max_representation) {
            for (auto p = cluster.begin(); p != cluster.end(); ++p) {
                if (!cache.insert(*p).second) {
                    continue;
                }
                identifier_sets.emplace_back(IdentifierSet(relation_, *p));
            }
        }

        auto elapsed_mills_to_gen_id_sets =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - start_time
            );
        LOG(INFO) << "TIME TO IDENTIFIER SETS GENERATION: "
                  << elapsed_mills_to_gen_id_sets.count();

        LOG(DEBUG) << "Identifier sets:";
        for (auto const& id_set : identifier_sets) {
            LOG(DEBUG) << id_set.toString();
        }

        // compute agree sets using identifier sets
        // using vector of identifier sets
        if (!identifier_sets.empty()) {
            auto back_it = std::prev(identifier_sets.end());
            for (auto p = identifier_sets.begin(); p != back_it; ++p) {
                for (auto q = std::next(p); q != identifier_sets.end(); ++q) {
                    agree_sets.insert(p->intersect(*q));
                }
            }
        }
    } else if constexpr (method == AgreeSetsGenMethod::kUsingMapOfIDSets) {
        method_str = "`kUsingMapOfIDSets`";
        std::unordered_map<int, IdentifierSet> identifier_sets;
        SetOfVectors const max_representation = genPLIMaxRepresentation();

        auto start_time = std::chrono::system_clock::now();

        for (auto const& cluster : max_representation) {
            for (auto p = cluster.begin(); p != cluster.end(); ++p) {
                identifier_sets.emplace(*p, IdentifierSet(relation_, *p));
            }
        }

        auto elapsed_mills_to_gen_id_sets =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - start_time
            );
        LOG(INFO) << "TIME TO IDENTIFIER SETS GENERATION: "
                  << elapsed_mills_to_gen_id_sets.count();


        LOG(DEBUG) << "Identifier sets:";
        for (auto const& [index, id_set] : identifier_sets) {
            LOG(DEBUG) << id_set.toString();
        }


        // compute agree sets using identifier sets
        // metanome approach (using map of identifier sets)
        for (auto const &cluster : max_representation) {
            auto back_it = std::prev(cluster.end());
            for (auto p = cluster.begin(); p != back_it; ++p) {
                for (auto q = std::next(p); q != cluster.end(); ++q) {
                    IdentifierSet const& id_set1 = identifier_sets.at(*p);
                    IdentifierSet const& id_set2 = identifier_sets.at(*q);
                    agree_sets.insert(id_set1.intersect(id_set2));
                }
            }
        }
    } else if constexpr (method == AgreeSetsGenMethod::kUsingMCAndGetAgreeSet) {
        method_str = "`kUsingMCAndGetAgreeSet`";
        SetOfVectors const max_representation = genPLIMaxRepresentation();

        // Compute agree sets from maximal representation using getAgreeSet()
        // ~3300 ms on CIPublicHighway700 (Debug build), ~250 ms (Release)
        for (auto const& cluster : max_representation) {
            for (auto p = cluster.begin(); p != cluster.end(); ++p) {
                for (auto q = std::next(p); q != cluster.end(); ++q) {
                    agree_sets.insert(getAgreeSet(*p, *q));
                }
            }
        }
    } else if constexpr (method == AgreeSetsGenMethod::kUsingGetAgreeSet) {
        method_str = "`kUsingGetAgreeSet`";
        vector<ColumnData> const& columns_data = relation_->getColumnData();

        // Compute agree sets from stripped partitions (simplest method by Wyss)
        // ~40436 ms on CIPublicHighway700 (Debug build)
        for (ColumnData const& column_data : columns_data) {
            PositionListIndex const* const pli = column_data.getPositionListIndex();
            for (vector<int> const& cluster : pli->getIndex()) {
                for (auto p = cluster.begin(); p != cluster.end(); ++p) {
                    for (auto q = std::next(p); q != cluster.end(); ++q) {
                        agree_sets.insert(getAgreeSet(*p, *q));
                    }
                }
            }
        }
    }

    // metanome kostil, doesn't work properly in general
    agree_sets.insert(*relation_->getSchema()->emptyVertical);

    auto elapsed_mills_to_gen_agree_sets =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - start_time
            );
    LOG(INFO) << "TIME TO AGREE SETS GENERATION WITH METHOD "
              << method_str << ": "
              << elapsed_mills_to_gen_agree_sets.count();

    return agree_sets;
}

AgreeSet AgreeSetFactory::getAgreeSet(int const tuple1_index,
                                      int const tuple2_index) const {
    std::vector<int> const tuple1 = relation_->getTuple(tuple1_index);
    std::vector<int> const tuple2 = relation_->getTuple(tuple2_index);
    boost::dynamic_bitset<> agree_set_indices(relation_->getNumColumns());

    for (size_t i = 0; i < agree_set_indices.size(); ++i) {
        if (tuple1[i] != 0 && tuple1[i] == tuple2[i]) {
            agree_set_indices.set(i);
        }
    }

    return relation_->getSchema()->getVertical(agree_set_indices);
}

AgreeSetFactory::SetOfVectors AgreeSetFactory::genPLIMaxRepresentation() const {
    auto start_time = std::chrono::system_clock::now();
    vector<ColumnData> const& columns_data = relation_->getColumnData();
    auto not_empty_pli = std::find_if(columns_data.begin(), columns_data.end(),
                                      [](ColumnData const& c) {
        return c.getPositionListIndex()->getSize() != 0;
    });

    if (not_empty_pli == columns_data.end()) {
        return {};
    }

    SetOfVectors max_representation(
        not_empty_pli->getPositionListIndex()->getIndex().begin(),
        not_empty_pli->getPositionListIndex()->getIndex().end()
    );

    for (auto p = columns_data.begin(); p != columns_data.end(); ++p) {
        //already examined
        if (p == not_empty_pli) {
            continue;
        }

        PositionListIndex const* pli = p->getPositionListIndex();
        if (pli->getSize() != 0) {
            calculateSupersets(max_representation, pli->getIndex());
        }
    }

    auto elapsed_mills_to_gen_max_representation =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time
        );
    LOG(INFO) << "TIME TO MAX REPRESENTATION GENERATION: "
              << elapsed_mills_to_gen_max_representation.count();

    return max_representation;
}

void AgreeSetFactory::calculateSupersets(SetOfVectors& max_representation,
                                         std::deque<vector<int>> const& partition) const {
    SetOfVectors to_add_to_mc;
    SetOfVectors to_delete_from_mc;
    //auto erase_from_partition = partition.end();
    set<std::deque<vector<int>>::const_iterator> to_exclude_from_partition;
    for (auto const& max_set : max_representation) {
        for (auto p = partition.begin();
             to_exclude_from_partition.size() != partition.size() && p != partition.end();
             ++p) {
            if (to_exclude_from_partition.find(p) != to_exclude_from_partition.end()) {
                continue;
            }

            if (max_set.size() >= p->size() &&
                std::includes(max_set.begin(), max_set.end(), p->begin(), p->end())) {
                to_add_to_mc.erase(*p);
                //erase_from_partition = p;
                to_exclude_from_partition.insert(p);
                break;
            }
            if (p->size() >= max_set.size() &&
                std::includes(p->begin(), p->end(), max_set.begin(), max_set.end())) {
                to_delete_from_mc.insert(max_set);
            }
            to_add_to_mc.insert(*p);
        }

        /*if (erase_from_partition != partition.end()) {
            partition.erase(erase_from_partition);
            erase_from_partition = partition.end();
        }*/
    }

    for (auto& cluster : to_add_to_mc) {
        max_representation.insert(std::move(cluster));
    }
    for (auto& cluster : to_delete_from_mc) {
        max_representation.erase(cluster);
    }
}

template AgreeSetFactory::SetOfAgreeSets
AgreeSetFactory::genAgreeSets<AgreeSetsGenMethod::kUsingVectorOfIDSets>() const;
template AgreeSetFactory::SetOfAgreeSets
AgreeSetFactory::genAgreeSets<AgreeSetsGenMethod::kUsingMapOfIDSets>() const;
template AgreeSetFactory::SetOfAgreeSets
AgreeSetFactory::genAgreeSets<AgreeSetsGenMethod::kUsingGetAgreeSet>() const;
template AgreeSetFactory::SetOfAgreeSets
AgreeSetFactory::genAgreeSets<AgreeSetsGenMethod::kUsingMCAndGetAgreeSet>() const;
