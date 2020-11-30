// Copyright (c) 2014-2018 The Crown developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activesystemnode.h"
#include "systemnode-sync.h"
#include "systemnode.h"
#include "systemnodeman.h"
#include "spork.h"
#include "shutdown.h"
#include "util.h"
#include "ui_interface.h"
#include "addrman.h"
#include "netmessagemaker.h"

class CSystemnodeSync;
CSystemnodeSync systemnodeSync;

CSystemnodeSync::CSystemnodeSync()
{
    Reset();
}

bool CSystemnodeSync::IsSynced()
{
    return RequestedSystemnodeAssets == SYSTEMNODE_SYNC_FINISHED;
}

bool CSystemnodeSync::IsBlockchainSynced()
{
    static bool fBlockchainSynced = false;
    static int64_t lastProcess = GetTime();

    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset the sync process
    if(GetTime() - lastProcess > 60*60) {
        Reset();
        fBlockchainSynced = false;
    }
    lastProcess = GetTime();

    if(fBlockchainSynced) return true;

    if (fImporting || fReindex) return false;

    TRY_LOCK(cs_main, lockMain);
    if(!lockMain) return false;

    CBlockIndex* pindex = chainActive.Tip();
    if(pindex == NULL) return false;


    if(!gArgs.GetBoolArg("-jumpstart", false) && pindex->nTime + 60*60 < GetTime())
        return false;

    fBlockchainSynced = true;

    return true;
}

void CSystemnodeSync::Reset()
{   
    lastSystemnodeList = 0;
    lastSystemnodeWinner = 0;
    lastBudgetItem = 0;
    mapSeenSyncSNB.clear();
    mapSeenSyncSNW.clear();
    lastFailure = 0;
    nCountFailures = 0;
    sumSystemnodeList = 0;
    sumSystemnodeWinner = 0;
    sumBudgetItemProp = 0;
    sumBudgetItemFin = 0;
    countSystemnodeList = 0;
    countSystemnodeWinner = 0;
    countBudgetItemProp = 0;
    countBudgetItemFin = 0;
    RequestedSystemnodeAssets = SYSTEMNODE_SYNC_INITIAL;
    RequestedSystemnodeAttempt = 0;
    nAssetSyncStarted = GetTime();
}

void CSystemnodeSync::AddedSystemnodeList(uint256 hash)
{
    if(snodeman.mapSeenSystemnodeBroadcast.count(hash)) {
        if(mapSeenSyncSNB[hash] < SYSTEMNODE_SYNC_THRESHOLD) {
            lastSystemnodeList = GetTime();
            mapSeenSyncSNB[hash]++;
        }
    } else {
        lastSystemnodeList = GetTime();
        mapSeenSyncSNB.insert(make_pair(hash, 1));
    }
}

void CSystemnodeSync::AddedSystemnodeWinner(uint256 hash)
{
    if(systemnodePayments.mapSystemnodePayeeVotes.count(hash)) {
        if(mapSeenSyncSNW[hash] < SYSTEMNODE_SYNC_THRESHOLD) {
            lastSystemnodeWinner = GetTime();
            mapSeenSyncSNW[hash]++;
        }
    } else {
        lastSystemnodeWinner = GetTime();
        mapSeenSyncSNW.insert(make_pair(hash, 1));
    }
}

void CSystemnodeSync::GetNextAsset()
{
    switch(RequestedSystemnodeAssets)
    {
        case(SYSTEMNODE_SYNC_INITIAL):
        case(SYSTEMNODE_SYNC_FAILED):
            ClearFulfilledRequest();
            RequestedSystemnodeAssets = SYSTEMNODE_SYNC_SPORKS;
            break;
        case(SYSTEMNODE_SYNC_SPORKS):
            RequestedSystemnodeAssets = SYSTEMNODE_SYNC_LIST;
            break;
        case(SYSTEMNODE_SYNC_LIST):
            RequestedSystemnodeAssets = SYSTEMNODE_SYNC_SNW;
            break;
        case(SYSTEMNODE_SYNC_SNW):
            LogPrintf("CSystemnodeSync::GetNextAsset - Sync has finished\n");
            RequestedSystemnodeAssets = SYSTEMNODE_SYNC_FINISHED;
            break;
    }
    RequestedSystemnodeAttempt = 0;
    nAssetSyncStarted = GetTime();
}

std::string CSystemnodeSync::GetSyncStatus()
{
    switch (systemnodeSync.RequestedSystemnodeAssets) {
        case SYSTEMNODE_SYNC_INITIAL: return _("Synchronization pending...");
        case SYSTEMNODE_SYNC_SPORKS: return _("Synchronizing systemnode sporks...");
        case SYSTEMNODE_SYNC_LIST: return _("Synchronizing systemnodes...");
        case SYSTEMNODE_SYNC_SNW: return _("Synchronizing systemnode winners...");
        case SYSTEMNODE_SYNC_FAILED: return _("Systemnode synchronization failed");
        case SYSTEMNODE_SYNC_FINISHED: return _("Systemnode synchronization finished");
    }
    return "";
}

void CSystemnodeSync::ProcessMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv)
{
    if (strCommand == "snssc") {

        if (IsSynced())
            return;

        int nItemID;
        int nCount;
        vRecv >> nItemID >> nCount;

        //this means we will receive no further communication
        switch(nItemID) {
            case(SYSTEMNODE_SYNC_LIST):
                if(nItemID != RequestedSystemnodeAssets) return;
                sumSystemnodeList += nCount;
                countSystemnodeList++;
                break;
            case(SYSTEMNODE_SYNC_SNW):
                if(nItemID != RequestedSystemnodeAssets) return;
                sumSystemnodeWinner += nCount;
                countSystemnodeWinner++;
                break;
        }
        
        LogPrintf("CSystemnodeSync:ProcessMessage - snssc - got inventory count %d %d\n", nItemID, nCount);
    }
}

void CSystemnodeSync::ClearFulfilledRequest()
{
    for (const auto& pnode : g_connman->CopyNodeVector()) {
        pnode->ClearFulfilledRequest("sngetspork");
        pnode->ClearFulfilledRequest("snsync");
        pnode->ClearFulfilledRequest("snwsync");
    }
}

void CSystemnodeSync::Process()
{
    const CNetMsgMaker msgMaker(PROTOCOL_VERSION);
    static int tick = 0;

    if (tick++ % SYSTEMNODE_SYNC_TIMEOUT != 0)
        return;

    if (IsSynced()) {
        return;
    }

    //try syncing again
    if (RequestedSystemnodeAssets == SYSTEMNODE_SYNC_FAILED && lastFailure + (1*60) < GetTime()) {
        Reset();
    } else if (RequestedSystemnodeAssets == SYSTEMNODE_SYNC_FAILED) {
        return;
    }

    // Calculate "progress" for LOG reporting / GUI notification
    double nSyncProgress = double(RequestedSystemnodeAttempt + (RequestedSystemnodeAssets - 1) * 8) / (8*4);
    uiInterface.NotifyAdditionalDataSyncProgressChanged(nSyncProgress);
    LogPrintf("CSystemnodeSync::ProcessTick -- nTick %d nRequestedSystemnodeAssets %d nRequestedSystemnodeAttempt %d nSyncProgress %f\n", tick, RequestedSystemnodeAssets, RequestedSystemnodeAttempt, nSyncProgress);

    if (!IsBlockchainSynced() && RequestedSystemnodeAssets > SYSTEMNODE_SYNC_SPORKS) return;
    if (RequestedSystemnodeAssets == SYSTEMNODE_SYNC_INITIAL) GetNextAsset();

    std::vector<CNode*> vNodesCopy = g_connman->CopyNodeVector();
    for (auto& pnode : vNodesCopy) {
        if(RequestedSystemnodeAssets == SYSTEMNODE_SYNC_SPORKS){
            if (pnode->HasFulfilledRequest("sngetspork"))
                continue;
            pnode->FulfilledRequest("sngetspork");

            g_connman->PushMessage(pnode, msgMaker.Make("getsporks")); //get current network sporks
            if (RequestedSystemnodeAttempt >= 2)
               GetNextAsset();
            RequestedSystemnodeAttempt++;
            
            return;
        }

        if (pnode->nVersion >= systemnodePayments.GetMinSystemnodePaymentsProto()) {
            if(RequestedSystemnodeAssets == SYSTEMNODE_SYNC_LIST) {
                LogPrintf("CSystemnodeSync::Process() - lastSystemnodeList %lld (GetTime() - SYSTEMNODE_SYNC_TIMEOUT) %lld\n", lastSystemnodeList, GetTime() - SYSTEMNODE_SYNC_TIMEOUT);
                if(lastSystemnodeList > 0 && lastSystemnodeList < GetTime() - SYSTEMNODE_SYNC_TIMEOUT*2 && RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD){ //hasn't received a new item in the last five seconds, so we'll move to the
                    GetNextAsset();
                    return;
                }

                if (pnode->HasFulfilledRequest("snsync"))
                    continue;
                pnode->FulfilledRequest("snsync");

                // timeout
                if(lastSystemnodeList == 0 && (RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD*3 || GetTime() - nAssetSyncStarted > SYSTEMNODE_SYNC_TIMEOUT*5)) {
                    if(IsSporkActive(SPORK_14_SYSTEMNODE_PAYMENT_ENFORCEMENT)) {
                        LogPrintf("CSystemnodeSync::Process - ERROR - Sync has failed, will retry later\n");
                        RequestedSystemnodeAssets = SYSTEMNODE_SYNC_FAILED;
                        RequestedSystemnodeAttempt = 0;
                        lastFailure = GetTime();
                        nCountFailures++;
                    } else {
                        GetNextAsset();
                    }
                    return;
                }

                if(RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD*3) return;

                snodeman.DsegUpdate(pnode);
                RequestedSystemnodeAttempt++;
                return;
            }

            if(RequestedSystemnodeAssets == SYSTEMNODE_SYNC_SNW) {
                if(lastSystemnodeWinner > 0 && lastSystemnodeWinner < GetTime() - SYSTEMNODE_SYNC_TIMEOUT*2 && RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD){ //hasn't received a new item in the last five seconds, so we'll move to the
                    GetNextAsset();
                    return;
                }

                if(pnode->HasFulfilledRequest("snwsync")) continue;
                pnode->FulfilledRequest("snwsync");

                // timeout
                if(lastSystemnodeWinner == 0 && (RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD*3 || GetTime() - nAssetSyncStarted > SYSTEMNODE_SYNC_TIMEOUT*5)) {
                    if(IsSporkActive(SPORK_14_SYSTEMNODE_PAYMENT_ENFORCEMENT)) {
                        LogPrintf("CSystemnodeSync::Process - ERROR - Sync has failed, will retry later\n");
                        RequestedSystemnodeAssets = SYSTEMNODE_SYNC_FAILED;
                        RequestedSystemnodeAttempt = 0;
                        lastFailure = GetTime();
                        nCountFailures++;
                    } else {
                        GetNextAsset();
                    }
                    return;
                }

                if (RequestedSystemnodeAttempt >= SYSTEMNODE_SYNC_THRESHOLD*3)
                    return;

                int nSnCount = snodeman.CountEnabled();
                g_connman->PushMessage(pnode, msgMaker.Make("snget", nSnCount)); //sync payees
                RequestedSystemnodeAttempt++;

                return;
            }
        }
    }
}
