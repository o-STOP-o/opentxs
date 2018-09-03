// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTCLIENT_HPP
#define OPENTXS_CLIENT_OTCLIENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <string>

namespace opentxs
{
class OTClient
{
public:
    bool AcceptEntireNymbox(
        Ledger& theNymbox,
        ServerContext& context,
        Message& theMessage);
    inline OTMessageOutbuffer& GetMessageOutbuffer()
    {
        return m_MessageOutbuffer;
    }
    bool ProcessNotification(
        const otx::Reply& notification,
        ServerContext& context);
    bool processServerReply(
        const std::set<ServerContext::ManagedNumber>& managed,
        const bool resync,
        ServerContext& context,
        std::shared_ptr<Message>& reply);
    bool processServerReply(
        const std::set<ServerContext::ManagedNumber>& managed,
        ServerContext& context,
        std::shared_ptr<Message>& reply,
        Ledger* pNymbox);
    bool processServerReply(
        const std::set<ServerContext::ManagedNumber>& managed,
        const bool resync,
        ServerContext& context,
        std::shared_ptr<Message>& reply,
        Ledger* pNymbox);
    std::int32_t ProcessUserCommand(
        const MessageType requestedCommand,
        ServerContext& context,
        Message& theMessage,
        const Identifier& pHisNymID,
        const Identifier& pHisAcctID,
        const Amount lTransactionAmount = 0,
        const Account* pAccount = nullptr,
        const UnitDefinition* pMyUnitDefinition = nullptr);
    void QueueOutgoingMessage(const Message& theMessage);

    explicit OTClient(
        OTWallet& theWallet,
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::client::Workflow& workflow);

private:
#if OT_CASH
    OTWallet& m_pWallet;
#endif
    const api::Core& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    const api::client::Workflow& workflow_;
    OTMessageOutbuffer m_MessageOutbuffer;

    bool add_item_to_workflow(
        const Nym& nym,
        const Message& transportItem,
        const std::string& item) const;
    bool createInstrumentNoticeFromPeerObject(
        const ServerContext& context,
        const Message& message,
        const PeerObject& peerObject,
        const TransactionNumber number);
    bool harvest_unused(ServerContext& context);
    bool init_new_account(const Identifier& accountID, ServerContext& context)
        const;
    void ProcessIncomingTransaction(
        const Message& theReply,
        ServerContext& context,
        std::shared_ptr<OTTransaction> pTransaction,
        String& strReceiptID) const;
    void ProcessIncomingTransactions(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context) const;
#if OT_CASH
    void ProcessWithdrawalResponse(
        const Message& theReply,
        ServerContext& context,
        OTTransaction& theTransaction) const;
#endif  // OT_CASH
    void ProcessDepositResponse(
        const Message& theReply,
        const ServerContext& context,
        OTTransaction& theTransaction) const;
    void ProcessDepositChequeResponse(
        const ServerContext& context,
        std::shared_ptr<Item> pReplyItem) const;
    void ProcessPayDividendResponse(OTTransaction& theTransaction) const;
    void load_str_trans_add_to_ledger(
        const Identifier& the_nym_id,
        const String& str_trans,
        const String& str_box_type,
        const TransactionNumber& lTransNum,
        const Nym& the_nym,
        Ledger& ledger) const;
    void setRecentHash(
        const Message& theReply,
        bool setNymboxHash,
        ServerContext& context);
    bool processServerReplyTriggerClause(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyCheckNym(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyNotarizeTransaction(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool processServerReplyGetTransactionNumbers(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyGetNymBox(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyGetBoxReceipt(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyGetBoxReceipt(
        OTTransaction& receipt,
        ServerContext& context,
        const String& serialized,
        const std::int64_t boxType);
    bool processServerReplyProcessBox(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyProcessInbox(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context,
        OTTransaction* pTransaction,
        OTTransaction* pReplyTransaction);
    bool processServerReplyProcessNymbox(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context,
        OTTransaction* pTransaction,
        OTTransaction* pReplyTransaction);
    bool processServerReplyGetAccountData(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyGetInstrumentDefinition(
        const Message& theReply,
        ServerContext& context);
#if OT_CASH
    bool processServerReplyGetMint(const Message& theReply);
#endif  // OT_CASH
    bool processServerReplyGetMarketList(const Message& theReply);
    bool processServerReplyGetMarketOffers(const Message& theReply);
    bool processServerReplyGetMarketRecentTrades(const Message& theReply);
    bool processServerReplyGetNymMarketOffers(const Message& theReply);
    bool processServerReplyUnregisterNym(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyUnregisterAccount(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyRegisterInstrumentDefinition(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool processServerReplyRegisterAccount(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool processServerReplyResyncContext(
        const Message& theReply,
        ServerContext& context);
    void ProcessIncomingCronItemReply(
        std::shared_ptr<Item> pReplyItem,
        std::unique_ptr<OTCronItem>& pCronItem,
        ServerContext& context,
        const TransactionNumber& lNymOpeningNumber,
        std::shared_ptr<OTTransaction> pTransaction,
        const String& strCronItem) const;
};
}  // namespace opentxs

#endif
