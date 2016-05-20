// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <sst_config.h>
#include <sst/core/part/linpart.h>
#include <sst/core/output.h>

using namespace std;

bool SSTLinearPartition::initialized = SSTPartitioner::addPartitioner("linear",&SSTLinearPartition::allocate, "Partitions components by dividing Component ID space into roughly equal portions.  Components with sequential IDs will be placed close together.");

SSTLinearPartition::SSTLinearPartition(RankInfo mpiranks, int verbosity) {
	rankcount = mpiranks;
	partOutput = new Output("LinearPartition ", verbosity, 0, SST::Output::STDOUT);
}

void SSTLinearPartition::performPartition(PartitionGraph* graph) {
	assert(rankcount.rank > 0);

	PartitionComponentMap_t& compMap = graph->getComponentMap();

    uint32_t tot_ranks = rankcount.rank * rankcount.thread;

	// const int componentCount = compMap.size();
	const size_t componentCount = graph->getNumComponents();
	size_t componentRemainder = componentCount % tot_ranks;
	const size_t componentPerRank = componentCount / tot_ranks;

	partOutput->verbose(CALL_INFO, 1, 0, "Performing a linear partition scheme for simulation model.\n");
	partOutput->verbose(CALL_INFO, 1, 0, "Expected linear scheme:\n");
	partOutput->verbose(CALL_INFO, 1, 0, "- Component Count:                  %10zu\n", componentCount);
	partOutput->verbose(CALL_INFO, 1, 0, "- Approx. Components per Rank:      %10zu\n", componentPerRank);
	partOutput->verbose(CALL_INFO, 1, 0, "- Remainder (non-balanced dist.):   %10zu\n", componentRemainder);

	RankInfo currentAllocatingRank(0, 0);
	size_t componentsOnCurrentRank = 0;

    /* Special case: Fewer components than ranks: */
    if ( 0 == componentPerRank ) componentRemainder = 0;

	for(PartitionComponentMap_t::iterator compItr = compMap.begin();
		compItr != compMap.end();
		compItr++) {

		compItr->rank = currentAllocatingRank;
		componentsOnCurrentRank++;


        if(componentsOnCurrentRank >= componentPerRank) {
            /* Work off the remainder, by adding one more to this rank */
            if ( componentRemainder > 0 ) {
                --componentRemainder;
                ++compItr;
                compItr->rank = currentAllocatingRank;
            }

            /* Advance our currentAllocatingRank */
            currentAllocatingRank.thread++;
            if ( currentAllocatingRank.thread == rankcount.thread ) {
                currentAllocatingRank.thread = 0;
                currentAllocatingRank.rank++;
            }

            /* Reset the count */
            componentsOnCurrentRank = 0;
        }
	}

	partOutput->verbose(CALL_INFO, 1, 0, "Linear partition scheme completed.\n");
}
