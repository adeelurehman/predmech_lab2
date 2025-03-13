#include "mem/cache/replacement_policies/perceptreuse_rp.hh"

namespace gem5
{

namespace replacement_policy
{




void
PerceptReuse::touch(const std::shared_ptr<ReplacementData>& replacement_data,
    const PacketPtr pkt) {
    std::shared_ptr<PRReplData> casted_replacement_data =
        std::static_pointer_cast<PRReplData>(replacement_data);

    // When a hit happens and if the hit was in sampler set then
    // positively train its' weights
    if (inSamplerSet()) {
        trainWeights(getSignatures(pkt), true);
    }
    casted_replacement_data->used = true;

    // This was a hit; update replacement data accordingly
    // BRRIP::touch(replacement_data); either resets to insert val or
    // decremenets rrpv based on hit priority, we just want to set
    // to 0 or max depending on tau_replace
    if (casted_replacement_data->prediction >= TAU_REPLACE) {
        casted_replacement_data->rrpv.saturate();
    } else {
        casted_replacement_data->rrpv = 0;
    }
}

bool PerceptReuse::inSamplerSet() const {
    //TODO: do this (wait how the fuck do you get set/way (in ReplaceableEntry
    // but no access to it?))
}

Signs PerceptReuse::getSignatures(const PacketPtr pkt) const {
    Signs signs;

    if (pkt->req->hasPC()) {
        PCi[3] = PCi[2];
        PCi[2] = PCi[1];
        PCi[1] = PCi[0];
        PCi[0] = pkt->req->getPC();

        //TODO: make the anding variable to # of weight table entries,
        //right now cuts off top so only bottom 256 remains
        signs = {
            (PCi[0] ^ (PCi[0] >> 2)) & 0xFF,
            (PCi[0] ^ (PCi[1] >> 1)) & 0xFF,
            (PCi[0] ^ (PCi[2] >> 2)) & 0xFF,
            (PCi[0] ^ (PCi[3] >> 3)) & 0xFF,
            (PCi[0] ^ (PCi[0] >> 4)) & 0xFF,
            (PCi[0] ^ (PCi[0] >> 7)) & 0xFF,
        };
    } else {
        signs = NULL;
    }
    return signs;
}

//TODO: make these work for multi-core
int64_t PerceptReuse::getPrediction(Signs signs) {
    int64_t pred = 0;
    for (int i = 0; i < 6; ++i) {
        pred += PRCPT_WT[i][signs[i]];
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
