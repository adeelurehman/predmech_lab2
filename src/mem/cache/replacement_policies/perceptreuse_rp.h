/**
    This file defines a replacement policy based on the Perceptron Learning for
    Reuse Prediction paper by Teran et. al.

    Authors:
    Adeel Rehman
    Roy Mor

    UT CS 395T Prediction Mechanisms in Comp Arch - Sp25
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_SHIP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_SHIP_RP_HH__

#include <array>

#include "base/sat_counter.hh"
#include "mem/cache/replacement_policies/brrip_rp.hh"
#include "mem/packet.hh"

namespace gem5 {

struct PRParams; //TODO: figure out how to pass params properly

namespace replacement_policy {

class PerceptReuse : public BRRIP
{

    protected: //might need this to print via some gem5 logging
    // tools? otherwise could just be private
    // TODO: decide if we want to figure out how to do
    // negative sat counters or just keep unsigned
    // and deal with shifting all the thresholds
    // and initialization stuff up by half of size
    // TODO: figure out sampler sets

        unsigned weight_size; // TODO: get from params
        int weight_table_size ; //TODO: same
        int weight_init; // TODO: same
        int TAU_REPLACE; // TODO: same
        int TAU_BYPASS; // TODO: same

        // is this even legal
        // typedef GenericSatCounter<uint16_t>::
        //     GenericSatCounter(signature_counter_size, weight_init)
        // PRSatCount;
        typedef SatCounter8(signature_counter_size, weight_init) PRSatCount;
        typedef std::array<uint32_t, 6> Signs;

        // TODO: get core count from params and make these per core
        std::array<std::array<PRSatCount, sign_weight_table_size>, 6> PRCPT_WT;
        std::array<Addr, 4> PCi;

        struct PRReplData : public BRRIPReplData
        { // CONTAINS PER CACHE LINE METADATA (I THINK)
            Signs signatures;
            int64_t prediction;
            bool used;
        }

    public:
        PerceptReuse(const PRParams &p);
        ~PerceptReuse() = default;


        void touch(const std::shared_ptr<ReplacementData>& replacement_data,
            const PacketPtr pkt) override;
        void touch(const std::shared_ptr<ReplacementData>& replacement_data)
            const override;


        bool inSamplerSet() const;
        Signs getSignatures(const PacketPtr pkt) const;
        int64_t getPrediction(Signs signs);
        void trainWeights(Signs signs, bool polarity);
}



} //rp namespace
} //gem5 namespace


#endif
