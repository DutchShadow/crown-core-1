// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion.SetGenesisVersion(1);
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The inception of Crowncoin 10/Oct/2014";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 2100000;
        consensus.BIP16Exception = uint256S("0x00000000000002dc756eebf4f49723ed8d30cc28a5f108eb94b1ba88ac4f9c22");
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 388381; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 363725; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 1 * 60; // Crown: 1 minute
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000028822fef1c230963535a90d");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000002e63058c023a9a1de233554f28c7b21380b6c9003f36a8"); //534292

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xb8;
        pchMessageStart[1] = 0xeb;
        pchMessageStart[2] = 0xb3;
        pchMessageStart[3] = 0xdf;
        nDefaultPort = 9340;
        nBlockPoSStart = 2330000;
        nPruneAfterHeight = 100000;
        nAuxpowChainId = 20;
        nChainStallDuration = 60*60;

        genesis = CreateGenesisBlock(1412760826, 1612467894, 0x1d00ffff, 1, 10 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000000085370d5e122f64f4ab19c68614ff3df78c8d13cb814fd7e69a1dc6da"));
        assert(genesis.hashMerkleRoot == uint256S("0x80ad356118a9ab8db192db66ef77146cc36d958f959251feace550e4ca3d1446"));

/** Main net nameservers defined here, they are geographic strategic and redundant through 2 domains.
* The nameservers are location-XXseedNS.crownplatform. net/org they will point to main net dns seed VPS's
* through location-XX.seeder.crowncoin.net which can be set up by community members, and also wil be set up by the infra team.
* Community members wanting to contribute a VPS will request (outside of public view) for a DNS entry to be made to the main nameservers for their dns seed's IP,
* getting help setting it up from the infra team.
*/


        //vSeeds.push_back(CDNSSeedData("eur1-crwdns", "europe-01seedns.crowncoin.org"));
        vSeeds.emplace_back("europe-01seedns.crowncoin.org");
        vSeeds.emplace_back("europe-02seedns.crowncoin.net");
        vSeeds.emplace_back("canada-01seedns.crowncoin.org");
        vSeeds.emplace_back("latam-01seedns.crowncoin.net");
        vSeeds.emplace_back("SEAsia-01seedns.crowncoin.org");
        vSeeds.emplace_back("pacific-01seedns.crowncoin.net");

        //base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        // TODO fix
        // Crown addresses start with 'CRW'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>{0x01, 0x75, 0x07}; //list_of(0x01)(0x75)(0x07).convert_to_container<std::vector<unsigned char> >();
        // Crown script addresses start with 'CRM'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>{0x01, 0x74, 0xF1}; //list_of(0x01)(0x74)(0xF1).convert_to_container<std::vector<unsigned char> >();
        //base58PrefixesOld[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,28);                   // Crown script addresses start with 'C'
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1,128);                  // Crown private keys start with '5'
        base58Prefixes[EXT_PUBLIC_KEY] = std::vector<unsigned char>{0x04, 0x88, 0xB2, 0x1E}; //list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = std::vector<unsigned char>{0x04, 0x88, 0xAD, 0xE4}; //list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        //base58Prefixes[EXT_COIN_TYPE]  = list_of(0x80000048).convert_to_container<std::vector<unsigned char> >();             // Crown BIP44 coin type is '72'

        //// Identity addresses start with 'CRP'
        //base58Prefixes[IDENTITY_ADDRESS] = list_of(0x01)(0x74)(0xF6).convert_to_container<std::vector<unsigned char> >();
        //// App service addresses start with 'CRA'
        //base58Prefixes[APP_SERVICE_ADDRESS] = list_of(0x01)(0x74)(0xD5).convert_to_container<std::vector<unsigned char> >();
        //// Title addresses start with 'CRT'
        //base58Prefixes[TITLE_ADDRESS] = list_of(0x01)(0x75)(0x00).convert_to_container<std::vector<unsigned char> >();

        //convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        //fRequireRPCPassword = true;
        //fMiningRequiresPeers = true;
        //fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                { 11111, uint256S("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d")},
                { 33333, uint256S("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")},
                { 74000, uint256S("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")},
                {105000, uint256S("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")},
                {134444, uint256S("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe")},
                {168000, uint256S("0x000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763")},
                {193000, uint256S("0x000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317")},
                {210000, uint256S("0x000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e")},
                {216116, uint256S("0x00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e")},
                {225430, uint256S("0x00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932")},
                {250000, uint256S("0x000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214")},
                {279000, uint256S("0x0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40")},
                {295000, uint256S("0x00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983")},
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 0000000000000000002e63058c023a9a1de233554f28c7b21380b6c9003f36a8
            /* nTime    */ 1532884444,
            /* nTxCount */ 331282217,
            /* dTxRate  */ 2.4
        };

        /* disable fallback fee on mainnet */
        m_fallback_fee_enabled = false;
        strLegacySignerDummyAddress = "18WTcWvwrNnfqeQAn6th9QQ2EpnXMq5Th8";
    }

    bool StrictChainId() const
    {
        return true;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 130000;
        consensus.BIP16Exception = uint256S("0x00000000dd30457c001f4095d208cc1296b0eed002427aa599874af7a432b105");
        consensus.BIP34Height = 2111111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.BIP65Height = 581885; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 330776; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 1.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000007dbe94253893cbd463");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000037a8cd3e06cd5edbfe9dd1dbcc5dacab279376ef7cfc2b4c75"); //1354312

        pchMessageStart[0] = 0x0f;
        pchMessageStart[1] = 0x18;
        pchMessageStart[2] = 0x0e;
        pchMessageStart[3] = 0x06;
        nDefaultPort = 19340;
        nBlockPoSStart = 141000;
        nPruneAfterHeight = 1000;

        //genesis = CreateGenesisBlock(1296688602, 414098458, 0x1d00ffff, 1, 50 * COIN);
        genesis = CreateGenesisBlock(1412760826, 1612467894, 0x1d00ffff, 1, 10 * COIN);
        //genesis.nTime    = 1412760826;
        //genesis.nNonce   = 1612467894;

        consensus.hashGenesisBlock = genesis.GetHash();
        std::cout << "hash " << consensus.hashGenesisBlock.ToString() << std::endl;
        assert(consensus.hashGenesisBlock == uint256S("0x0000000085370d5e122f64f4ab19c68614ff3df78c8d13cb814fd7e69a1dc6da"));
        assert(genesis.hashMerkleRoot == uint256S("0x80ad356118a9ab8db192db66ef77146cc36d958f959251feace550e4ca3d1446"));

        vFixedSeeds.clear();
        vSeeds.clear();
// Testnet seeders update is outside of current hotfix scope
        vSeeds.emplace_back("fra-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("blr-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("sgp-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("lon-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("nyc-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("tor-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("sfo-testnet-crwdns.crowndns.info");
        vSeeds.emplace_back("ams-testnet-crwdns.crowndns.info");

        // TODO fix
        // Testnet crown addresses start with 'tCRW'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char> {0x01, 0x7A, 0xCD, 0x67};
        //base58PrefixesOld[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        // Testnet crown script addresses start with 'tCRM'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char> {0x01, 0x7A, 0xCD, 0x51};
        //base58PrefixesOld[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);                    // Testnet crown script addresses start with '8' or '9'
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1,239);                    // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = std::vector<unsigned char> {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = std::vector<unsigned char> {0x04, 0x35, 0x83, 0x94};
        //base58Prefixes[EXT_COIN_TYPE]  = std::vector<unsigned char> (0x80000001).convert_to_container<std::vector<unsigned char> >();             // Testnet crown BIP44 coin type is '5' (All coin's testnet default)

        // Identity addresses start with 'tCRP'
        //base58Prefixes[IDENTITY_ADDRESS] = std::vector<unsigned char> {0x01, 0x7A, 0xCD, 0x56};
        // App service addresses start with 'tCRA'
        //base58Prefixes[APP_SERVICE_ADDRESS] = std::vector<unsigned char> {0x01, 0x7A, 0xCD, 0x35};
        // Title addresses start with 'tCRT'
        //base58Prefixes[TITLE_ADDRESS] = std::vector<unsigned char> {0x01, 0x7A, 0xCD, 0x60};

        //convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        //fRequireRPCPassword = true;
        //fMiningRequiresPeers = true;
        //fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        //fTestnetToBeDeprecatedFieldRPC = true;

        //nPoolMaxTransactions = 2;
        //strSporkKey = "04EA9AF53E4F12CE41F78B666EBDBE96C966ABDD8832979228BD3299E13089F117936EF97B7B9D4644B8B9D2BC7A30029BD7FDDCAC36E40AAC0E03891E493CF197";
        strDevfundAddress = "mr59c3aniaN3qHXej5L8UBsssRZbiUUMnz";
        strLegacySignerDummyAddress = "mr59c3aniaN3qHXej5L8UBsssRZbiUUMnz";
        nStartMasternodePayments = 1420837558; //Fri, 09 Jan 2015 21:05:58 GMT

        nStakePointerValidityPeriod = 3000;
    }
    //const Checkpoints::CCheckpointData& Checkpoints() const 
    //{
    //    return dataTestnet;
    //}
    int AuxpowStartHeight() const
    {
        return 0;
    }

    bool StrictChainId() const
    {
        return false;
    }

    bool AllowLegacyBlocks(unsigned) const
    {
        return true;
    }
};
static CTestNetParams testNetParams;

/**
 * Devnet
 */
class CDevNetParams : public CChainParams {
public:
    CDevNetParams() {
        // TODO fix
        //networkID = CBaseChainParams::DEVNET;
        strNetworkID = "dev";
        //nSubsidyHalvingInterval = 210240;
        //bnProofOfWorkLimit = ~arith_uint256(0) >> 8;
        //nMaxTipAge = 6 * 60 * 60;

        //nTargetTimespan = 2 * 60 * 60;  // 2 hours
        //nTargetSpacing = 30;      // 30 seconds

        //nEnforceBlockUpgradeMajority = 51;
        //nRejectBlockOutdatedMajority = 75;
        //nToCheckBlockUpgradeMajority = 100;

        pchMessageStart[0] = 0x0f;
        pchMessageStart[1] = 0x18;
        pchMessageStart[2] = 0x0e;
        pchMessageStart[3] = 0x06;
        //vAlertPubKey = ParseHex("04977aae0411f4e1757e8682c87ee79180ad577ef0351054e6cda5c9381fcd8c7333e88ac250d3ab3e3aafd5d1c1d946f2ca62372db7f35c84398a878aa145f09a");
        nDefaultPort = 19342;

        //genesis = CreateGenesisBlock(1412760826, 175963287, bnProofOfWorkLimit.GetCompact(), 4, 10 * COIN);
        //hashGenesisBlock = genesis.GetHash();
        
        //assert(hashGenesisBlock == uint256S("0x0055024546d26a67e2b0a13c86bc841efeeadb7b16155d5ea9a192cf6eeb56e6"));
        assert(genesis.hashMerkleRoot == uint256S("0x80ad356118a9ab8db192db66ef77146cc36d958f959251feace550e4ca3d1446"));

        bech32_hrp = "tb";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        //nPoolMaxTransactions = 3;

        nBlockPoSStart = 500;
        nAuxpowChainId = 20;
        //nPoSChainId = 22;
        nStakePointerValidityPeriod = 10080; // Valid for 7 days, to make sure they are valid after downtime
        //nMaxReorgDepth = 100;
        //nKernelModifierOffset = 10;
        //nChainStallDuration = 60*60;

        nStartMasternodePayments = 1420837558; //Fri, 09 Jan 2015 21:05:58 GMT
    }

    // TODO fix
        //checkpointData = {
        //    {
        //        {546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")},
        //    }
        //};

        //chainTxData = ChainTxData{
        //    // Data from rpc: getchaintxstats 4096 0000000000000037a8cd3e06cd5edbfe9dd1dbcc5dacab279376ef7cfc2b4c75
        //    /* nTime    */ 1531929919,
        //    /* nTxCount */ 19438708,
        //    /* dTxRate  */ 0.626
        //};

        /* enable fallback fee on testnet */
        //m_fallback_fee_enabled = true;
    //}
    bool StrictChainId() const
    {
        return false;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "bcrt";

        /* enable fallback fee on regtest */
        //m_fallback_fee_enabled = true;
    }
    bool StrictChainId() const
    {
        return true;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
