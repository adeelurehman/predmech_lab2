/**
    This file defines a replacement policy based on the Perceptron Learning for
    Reuse Prediction paper by Teran et. al.

    Authors:
    Adeel Rehman
    Roy Mor

    UT CS 395T Prediction Mechanisms in Comp Arch - Sp25
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_PERCEPTREUSE_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_PERCEPTREUSE_RP_HH__

#include <array>

#include "base/sat_counter.hh"
#include "mem/cache/replacement_policies/brrip_rp.hh"
#include "mem/packet.hh"

namespace gem5 {

//TODO: figure out how to pass params properly
struct PerceptReuseRPParams; 

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
        int theta;

        Addr PCi[4] = {0,0,0,0};

        // TODO: is this even legal (A: No.)
        // typedef GenericSatCounter<uint16_t>::
        //     GenericSatCounter(signature_counter_size, weight_init)
        // PRSatCount;
        // typedef SatCounter8(signature_counter_size, weight_init) PRSatCount;
        typedef std::array<Addr, 6> Signs;

        // TODO: get core count from params and make these per core
        std::array<std::vector<SatCounter8>, 6> PRCPT_WT;
        // std::array<std::array<SatCounter8(6, 0), 256>, 6> PRCPT_WT;
        // std::vector<std::array<std::array<PRSatCount, sign_weight_table_size>, 6>> PRCPT_WT; for multicore

        // std::array<Addr, 4> PCi;

        struct PRReplData : public BRRIPReplData
        { // CONTAINS PER CACHE LINE METADATA (I THINK) (yes it does)
            Signs signatures;
            // int64_t prediction;
            bool used;
            Addr address;
        };

        bool inSamplerSet() const;
        void UpdateHistory(const PacketPtr pkr);
        Signs getSignatures(const PacketPtr pkt) const;
        int64_t getPrediction(Signs signs);
        void trainWeights(Signs signs, bool polarity);

    public:
        typedef PerceptReuseRPParams Params;
        PerceptReuse(const Params &p);
        ~PerceptReuse() = default;


        void touch(const std::shared_ptr<ReplacementData>& replacement_data,
            const PacketPtr pkt) override;
        void touch(const std::shared_ptr<ReplacementData>& replacement_data)
            const override;

        void reset(const std::shared_ptr<ReplacementData>& replacement_data,
            const PacketPtr pkt) override;
        void reset(const std::shared_ptr<ReplacementData>& replacement_data)
            const override;

        // bool PerceptReuse::checkBypass(const PacketPtr pkt) override;

        ReplaceableEntry* getVictim(const ReplacementCandidates& candidates, const PacketPtr pkt) override;
        ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;
};



} //rp namespace
} //gem5 namespace


#endif
