#include "mem/cache/replacement_policies/perceptreuse_rp.hh"

#include "base/logging.hh"
#include "params/PerceptReuseRP.hh"

namespace gem5
{

namespace replacement_policy
{


// PerceptReuse::PRReplData::PRReplData() :
//     signatures(NULL), prediction(-1), used(false) {

// }

PerceptReuse::PerceptReuse(const Params &p) :
    BRRIP(p), weight_size(p.weight_size),
    weight_table_size(p.weight_table_size),
    weight_init(p.weight_init), TAU_REPLACE(p.TAU_REPLACE),
    TAU_BYPASS(p.TAU_BYPASS), theta(p.theta) {
    //TODO: initialize sampler set

    // PRCPT_WT.resize(p.system->numContexts()); for multicore
    
    for (auto& vec : PRCPT_WT) {
        vec = std::vector<SatCounter8>(weight_table_size, SatCounter8(weight_size));
    }
}


void
PerceptReuse::touch(const std::shared_ptr<ReplacementData>& replacement_data,
    const PacketPtr pkt) {
    std::shared_ptr<PRReplData> casted_replacement_data =
        std::static_pointer_cast<PRReplData>(replacement_data);

    // Get signatures
    UpdateHistory(pkt);
    Signs signatures = getSignatures(pkt);

    // When a hit happens and if the hit was in sampler set then
    // positively train its' weights
    if (inSamplerSet()) {
        trainWeights(signatures, true);
    }
    casted_replacement_data->used = true;

    // This was a hit; update replacement data accordingly
    // BRRIP::touch(replacement_data); either resets to insert val or
    // decremenets rrpv based on hit priority, we just want to set
    // to 0 or max depending on tau_replace
    if (getPrediction(signatures) >= TAU_REPLACE) {
        casted_replacement_data->rrpv.saturate();
    } else {
        casted_replacement_data->rrpv.reset(); // = 0;
    }
}

void 
PerceptReuse::touch(const std::shared_ptr<ReplacementData>& replacement_data)
    const {
    panic("Cant train percept's predictor without access information.");
}

void
PerceptReuse::reset(const std::shared_ptr<ReplacementData>& replacement_data, const PacketPtr pkt)
{
    std::shared_ptr<BRRIPReplData> casted_replacement_data =
        std::static_pointer_cast<BRRIPReplData>(replacement_data);

    // Get signatures, do not update history
    Signs signatures = getSignatures(pkt);

    // Reset RRPV on insertion
    int prediction = getPrediction(signatures);
    casted_replacement_data->rrpv.saturate();
    casted_replacement_data->rrpv--;
    if (prediction >= TAU_REPLACE) {
        // Should have been bypassed, probably never happens
        casted_replacement_data->rrpv.reset(); // = 0;
    }

    // Mark entry as ready to be used
    casted_replacement_data->valid = true;
}

void 
PerceptReuse::reset(const std::shared_ptr<ReplacementData>& replacement_data)
    const {
    panic("Cant insert percept's predictor without access information.");
}

/*
If true, the access is bypassed, no other functions are called.

*/
// bool PerceptReuse::checkBypass(const PacketPtr pkt) { 
//     // TODO: Update sampler.
//     Signs signatures = getSignatures(pkt);
//     if (getPrediction(signatures) >= TAU_BYPASS) {
//         return true;
//     } else {
//         return false;
//     }
// }

ReplaceableEntry*
PerceptReuse::getVictim(const ReplacementCandidates& candidates, const PacketPtr pkt)
{
    UpdateHistory(pkt);
    // TODO: Check for bypass
    Signs signatures = getSignatures(pkt);
    if (getPrediction(signatures) >= TAU_BYPASS && !pkt->cmd.isWrite()) {
        return nullptr;
    }

    return BRRIP::getVictim(candidates);
}

ReplaceableEntry*
PerceptReuse::getVictim(const ReplacementCandidates& candidates) const {
    panic("Cant select victim without pkt information.");
    return nullptr;
}

bool PerceptReuse::inSamplerSet() const {
    //TODO: do this (wait how the fuck do you get set/way (in ReplaceableEntry
    // but no access to it?))
    return false;
}

void PerceptReuse::UpdateHistory(const PacketPtr pkt) {
    if (pkt->req->hasPC()) {
        PCi[3] = PCi[2];
        PCi[2] = PCi[1];
        PCi[1] = PCi[0];
        PCi[0] = pkt->req->getPC();
    }
}

PerceptReuse::Signs PerceptReuse::getSignatures(const PacketPtr pkt) const {
    PerceptReuse::Signs signs;

    Addr addr = pkt->getAddr();

    // 64-byte line size (6 bits), and 13 bits for index. 
    Addr tag = addr >> 19;

    if (pkt->req->hasPC()) {

        signs = {
            (PCi[0] ^ (PCi[0] >> 2)) % weight_table_size,
            (PCi[0] ^ (PCi[1] >> 1)) % weight_table_size,
            (PCi[0] ^ (PCi[2] >> 2)) % weight_table_size,
            (PCi[0] ^ (PCi[3] >> 3)) % weight_table_size,
            (PCi[0] ^ (tag >> 4)) % weight_table_size,
            (PCi[0] ^ (tag >> 7)) % weight_table_size
        };
    } else {
        signs = {};
        panic("Can't select signature without PC");
    }
    return signs;
}

//TODO: make these work for multi-core
int64_t PerceptReuse::getPrediction(Signs signs) {
    int64_t pred = 0;
    for (int i = 0; i < 6; ++i) {
        pred += PRCPT_WT[i][signs[i]];
        // pred += PRCPT_WT[core_id][i][signs[i]]; multicore
    }
    return pred;
}

//true is lower is better
void PerceptReuse::trainWeights(Signs signs, bool polarity) {
    for (int i = 0; i < 6; ++i) {
        if (polarity) {
            --PRCPT_WT[i][signs[i]];
        } else {
            ++PRCPT_WT[i][signs[i]];
        }
    }
    return;
}

} //ns replacement_policy
} //ns gem5
