// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <chainparams.h>
#include <util.h>
#include <auxpow.h>
#include <validation.h>

unsigned int static DarkGravityWave(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    /* current difficulty formula, crown - DarkGravity v3, written by Evan Duffield - evan@crown.tech */
    arith_uint256 nProofOfWorkLimit = UintToArith256(params.powLimit);
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    bool isAdjustmentPeriod = BlockLastSolved->nHeight >= Params().PoSStartHeight() - 1 && BlockLastSolved->nHeight < Params().PoSStartHeight() + PastBlocksMax;
    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin || isAdjustmentPeriod)
    {
        return nProofOfWorkLimit.GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if(CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (arith_uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1); }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if(LastBlockTime > 0){
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.nPowTargetSpacing;

    if (nActualTimespan < _nTargetTimespan/3)
        nActualTimespan = _nTargetTimespan/3;
    if (nActualTimespan > _nTargetTimespan*3)
        nActualTimespan = _nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > nProofOfWorkLimit){
        bnNew = nProofOfWorkLimit;
    }

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int retarget = DIFF_DGW;
    if (pindexLast->nHeight + 1 >= 1059780)
        retarget = DIFF_DGW;
    else retarget = DIFF_BTC;

    if (Params().NetworkID() == CBaseChainParams::TESTNET && pindexLast->nHeight >= 140400)
    {
        // Use Dark Gravity Wave like in mainnet
        retarget = DIFF_DGW;
    }

    if (Params().NetworkID() == CBaseChainParams::TESTNET && pindexLast->nHeight >= 140394)
    {
        // Increase testnet difficulty
        Params().GetConsensusNonConst().powLimit = uint256S("0003ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    }

    if (Params().NetworkID() == CBaseChainParams::MAIN && pindexLast->nHeight >= Params().PoSStartHeight() - 1)
    {
        // Increase mainnet difficulty for POS
        Params().GetConsensusNonConst().powLimit = uint256S("0000003fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    }

    if (retarget == DIFF_BTC)
    {
        unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

        // Only change once per difficulty adjustment interval
        if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
        {
            if (params.fPowAllowMinDifficultyBlocks)
            {
                // Special difficulty rule for testnet:
                // If the new block's timestamp is more than 2* 10 minutes
                // then allow mining of a min-difficulty block.
                if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                    return nProofOfWorkLimit;
                else
                {
                    // Return the last non-special-min-difficulty-rules-block
                    const CBlockIndex* pindex = pindexLast;
                    while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                        pindex = pindex->pprev;
                    return pindex->nBits;
                }
            }
            return pindexLast->nBits;
        }

        // Go back by what we want to be 14 days worth of blocks
        int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
        assert(nHeightFirst >= 0);
        const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
        assert(pindexFirst);

        return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
    }
    // Retarget using Dark Gravity Wave 3
    else if (retarget == DIFF_DGW)
    {
        return DarkGravityWave(pindexLast, params);
    }

    return DarkGravityWave(pindexLast, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    if (Params().NetworkID() == CBaseChainParams::TESTNET && chainActive.Height() >= 140394)
    {
        // Increase testnet difficulty
        Params().GetConsensusNonConst().powLimit = uint256S("0003ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    }

    if (Params().NetworkID() == CBaseChainParams::MAIN && chainActive.Height() >= Params().PoSStartHeight() - 1)
    {
        // Increase mainnet difficulty for POS
        Params().GetConsensusNonConst().powLimit = uint256S("000003ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    }

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

/**
 * Check proof-of-work of a block header, taking auxpow into account.
 * @param block The block header.
 * @return True iff the PoW is correct.
 */
bool CheckProofOfWork(const CBlockHeader& block, const Consensus::Params& params)
{
    /* Except for legacy blocks with full version 1, ensure that
       the chain ID is correct.  Legacy blocks are not allowed since
       the merge-mining start, which is checked in AcceptBlockHeader
       where the height is known.  */
    if (!block.nVersion.IsLegacy() && Params().StrictChainId()
            && block.nVersion.GetChainId() != Params().AuxpowChainId())
        return error("%s : block does not have our chain ID"
                " (got %d, expected %d, full nVersion %d)",
                __func__, block.nVersion.GetChainId(),
                Params().AuxpowChainId(), block.nVersion.GetFullVersion());

    /* If there is no auxpow, just check the block hash.  */
    if (!block.auxpow)
    {
        if (block.nVersion.IsAuxpow())
            return error("%s : no auxpow on block with auxpow version",
                    __func__);

        if (!CheckProofOfWork(block.GetHash(), block.nBits, params))
            return error("%s : non-AUX proof of work failed", __func__);

        return true;
    }

    /* We have auxpow.  Check it.  */
    if (!block.nVersion.IsAuxpow())
        return error("%s : auxpow on block with non-auxpow version", __func__);

    /* Temporary check:  Disallow parent blocks with auxpow version.  This is
       for compatibility with the old client.  */
    /* FIXME: Remove this check with a hardfork later on.  */
    if (block.auxpow->getParentBlock().nVersion.IsAuxpow())
        return error("%s : auxpow parent block has auxpow version", __func__);

    if (!block.auxpow->check(block.GetHash(), block.nVersion.GetChainId()))
        return error("%s : AUX POW is not valid", __func__);
    if (!CheckProofOfWork(block.auxpow->getParentBlockHash(), block.nBits, params))
        return error("%s : AUX proof of work failed", __func__);

    return true;
}
