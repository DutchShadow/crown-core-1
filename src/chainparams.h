// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include <chainparamsbase.h>
#include <consensus/params.h>
#include <primitives/block.h>
#include <protocol.h>

#include <memory>
#include <vector>

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

typedef std::map<int, uint256> MapCheckpoints;

struct CCheckpointData {
    MapCheckpoints mapCheckpoints;
};

/**
 * Holds various statistics on transactions within a chain. Used to estimate
 * verification progress during chain sync.
 *
 * See also: CChainParams::TxData, GuessVerificationProgress.
 */
struct ChainTxData {
    int64_t nTime;    //!< UNIX timestamp of last known number of transactions
    int64_t nTxCount; //!< total number of transactions between genesis and that timestamp
    double dTxRate;   //!< estimated number of transactions per second after that timestamp
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,

        MAX_BASE58_TYPES
    };

    enum AddressType
    {
        NEW_ADDRESS_TYPE,
        DEPRECATED_ADDRESS_TYPE
    };

    const Consensus::Params& GetConsensus() const { return consensus; }
    Consensus::Params& GetConsensusNonConst() { return consensus; }
    const CMessageHeader::MessageStartChars& MessageStart() const { return pchMessageStart; }
    int GetDefaultPort() const { return nDefaultPort; }

    const CBlock& GenesisBlock() const { return genesis; }
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    /** Policy: Filter transactions that do not match well-defined patterns */
    bool RequireStandard() const { return fRequireStandard; }
    uint64_t PruneAfterHeight() const { return nPruneAfterHeight; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string NetworkIDString() const { return strNetworkID; }
    /** Return true if the fallback fee is by default enabled for this network */
    bool IsFallbackFeeEnabled() const { return m_fallback_fee_enabled; }
    /** Return the list of hostnames to look up for DNS seeds */
    const std::vector<std::string>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::string& Bech32HRP() const { return bech32_hrp; }
    const std::vector<SeedSpec6>& FixedSeeds() const { return vFixedSeeds; }
    //const CCheckpointData& Checkpoints() const { return checkpointData; }
    const ChainTxData& TxData() const { return chainTxData; }
    const std::string NetworkID() const { return strNetworkID; }
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout);
    std::string LegacySignerDummyAddress() const { return strLegacySignerDummyAddress; }
    int ValidStakePointerDuration() const { return nStakePointerValidityPeriod; }
    int MaxReorganizationDepth() const { return nMaxReorgDepth; }
    int KernelModifierOffset() const { return nKernelModifierOffset; }

    // TODO old values
    // const uint256& HashGenesisBlock() const { return hashGenesisBlock; }
    // const uint256& HashDevnetGenesisBlock() const { return hashDevnetGenesisBlock; }
    // const CMessageHeader::MessageStartChars& MessageStart() const { return pchMessageStart; }
    // const std::vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
    // int GetDefaultPort() const { return nDefaultPort; }
    //const uint256& ProofOfWorkLimit() const { return bnProofOfWorkLimit; }
    //void SetProofOfWorkLimit(const uint256& limit) { bnProofOfWorkLimit = limit; }
    // int SubsidyHalvingInterval() const { return nSubsidyHalvingInterval; }
    // /** Used to check majorities for block version upgrade */
    // int EnforceBlockUpgradeMajority() const { return nEnforceBlockUpgradeMajority; }
    // int RejectBlockOutdatedMajority() const { return nRejectBlockOutdatedMajority; }
    // int ToCheckBlockUpgradeMajority() const { return nToCheckBlockUpgradeMajority; }

    // /** Used if GenerateBitcoins is called with a negative number of threads */
    // int DefaultMinerThreads() const { return nMinerThreads; }
    // const CBlock& GenesisBlock() const { return genesis; }
    // const CBlock& DevNetGenesisBlock() const { return devnetGenesis; }
    // bool RequireRPCPassword() const { return fRequireRPCPassword; }
    // /** Make miner wait to have peers to avoid wasting work */
    // bool MiningRequiresPeers() const { return fMiningRequiresPeers; }
    // /** Default value for -checkmempool and -checkblockindex argument */
    // bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    // /** Allow mining of a min-difficulty block */
    // bool AllowMinDifficultyBlocks() const { return fAllowMinDifficultyBlocks; }
    // /** Make standard checks */
    // bool RequireStandard() const { return fRequireStandard; }
    // int64_t TargetTimespan() const { return nTargetTimespan; }
    // int64_t TargetSpacing() const { return nTargetSpacing; }
    // int64_t Interval() const { return nTargetTimespan / nTargetSpacing; }
    // int64_t MaxTipAge() const { return nMaxTipAge; }
    // /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    // bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }
    // /** In the future use NetworkIDString() for RPC fields */
    // bool TestnetToBeDeprecatedFieldRPC() const { return fTestnetToBeDeprecatedFieldRPC; }
    // /** Return the BIP70 network string (main, test or regtest) */
    // std::string NetworkIDString() const { return strNetworkID; }
    // const std::vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    // const std::vector<unsigned char>& Base58Prefix(Base58Type type, AddressType addressType = NEW_ADDRESS_TYPE) const
    // {
    //     if (addressType == DEPRECATED_ADDRESS_TYPE)
    //     {
    //         return base58PrefixesOld[type];
    //     }
    //     return base58Prefixes[type];
    // }
    // const std::vector<CAddress>& FixedSeeds() const { return vFixedSeeds; }
    // virtual const Checkpoints::CCheckpointData& Checkpoints() const = 0;
    // int PoolMaxTransactions() const { return nPoolMaxTransactions; }
    std::string SporkKey() const { return strSporkKey; }
    // std::string LegacySignerDummyAddress() const { return strLegacySignerDummyAddress; }
    // std::string DevfundAddress() const { return strDevfundAddress; }
    int64_t StartMasternodePayments() const { return nStartMasternodePayments; }
    // CBaseChainParams::Network NetworkID() const { return networkID; }
    // /* Return the auxpow chain ID.  */
    inline int32_t AuxpowChainId () const { return nAuxpowChainId; }
    int32_t PoSChainId () const { return nPoSChainId; }
    int PoSStartHeight() const { return nBlockPoSStart; }
    // int ValidStakePointerDuration() const { return nStakePointerValidityPeriod; }
    // int MaxReorganizationDepth() const { return nMaxReorgDepth; }
    // int KernelModifierOffset() const { return nKernelModifierOffset; }
    int ChainStallDuration() const { return nChainStallDuration; }
    // /* Return start height of auxpow and the retarget interval change.  */
    // virtual int AuxpowStartHeight() const = 0;
    /* Return whether or not to enforce strict chain ID checks.  */
    virtual bool StrictChainId() const = 0;
    // /* Return whether to allow blocks with a "legacy" version.  */
    // virtual bool AllowLegacyBlocks(unsigned nHeight) const = 0;
protected:
    CChainParams() {}

    Consensus::Params consensus;
    CMessageHeader::MessageStartChars pchMessageStart;
    int nDefaultPort;
    uint64_t nPruneAfterHeight;
    std::vector<std::string> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    std::string bech32_hrp;
    std::string strNetworkID;
    CBlock genesis;
    std::vector<SeedSpec6> vFixedSeeds;
    bool fDefaultConsistencyChecks;
    bool fRequireStandard;
    bool fMineBlocksOnDemand;
    CCheckpointData checkpointData;
    ChainTxData chainTxData;
    bool m_fallback_fee_enabled;
    std::string strLegacySignerDummyAddress;
    std::string strSporkKey;
    std::string strDevfundAddress;
    int64_t nStartMasternodePayments{0};
    int32_t nAuxpowChainId;
    int32_t nPoSChainId;
    int nBlockPoSStart;
    int nStakePointerValidityPeriod;
    int nMaxReorgDepth;
    int nKernelModifierOffset;
    int nChainStallDuration;
};

/**
 * Creates and returns a std::unique_ptr<CChainParams> of the chosen chain.
 * @returns a CChainParams* of the chosen chain.
 * @throws a std::runtime_error if the chain is not supported.
 */
std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
CChainParams &Params();

/**
 * Sets the params returned by Params() to those for the given BIP70 chain name.
 * @throws std::runtime_error when the chain is not supported.
 */
void SelectParams(const std::string& chain);

/**
 * Allows modifying the Version Bits regtest parameters.
 */
void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout);

#endif // BITCOIN_CHAINPARAMS_H
