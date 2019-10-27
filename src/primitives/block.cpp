// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include "auxpow.h"
#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>

void CBlockHeader::SetAuxpow (CAuxPow* apow)
{
    if (apow)
    {
        auxpow.reset(apow);
        nVersion.SetAuxpow(true);
    } else
    {
        auxpow.reset();
        nVersion.SetAuxpow(false);
    }
}

void CBlockHeader::SetProofOfStake(bool fProofOfStake)
{
    nVersion.SetProofOfStake(fProofOfStake);
}

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

bool CBlock::IsProofOfStake() const
{
    return vtx.size() > 1 && vtx[1]->IsCoinStake();
}

bool CBlock::IsProofOfWork() const
{
    return !IsProofOfStake();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion.GetFullVersion(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

void CBlockHeader::ResetAuxPow()
{
    auxpow.reset(new CAuxPow());
}
