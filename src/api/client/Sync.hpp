// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace std
{
using PAYMENTTASK =
    pair<opentxs::OTIdentifier, shared_ptr<const opentxs::OTPayment>>;

template <>
struct less<PAYMENTTASK> {
    bool operator()(const PAYMENTTASK& lhs, const PAYMENTTASK& rhs) const
    {
        /* TODO: these lines will cause a segfault in the clang-5 ast parser.
                const auto & [ lID, lPayment ] = lhs;
                const auto & [ rID, rPayment ] = rhs;
        */
        const auto& lID = std::get<0>(lhs);
        const auto& lPayment = std::get<1>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rPayment = std::get<1>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lPayment.get() < rPayment.get()) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs::api::client::implementation
{
class Sync : virtual public client::Sync, Lockable
{
public:
    bool AcceptIncoming(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const std::size_t max = DEFAULT_PROCESS_INBOX_ITEMS) const override;
    Depositability CanDeposit(
        const Identifier& recipientNymID,
        const OTPayment& payment) const override;
    Depositability CanDeposit(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const override;
    Messagability CanMessage(
        const Identifier& senderNymID,
        const Identifier& recipientContactID) const override;
    std::size_t DepositCheques(const Identifier& nymID) const override;
    std::size_t DepositCheques(
        const Identifier& nymID,
        const std::set<OTIdentifier>& chequeIDs) const override;
    OTIdentifier DepositPayment(
        const Identifier& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    OTIdentifier DepositPayment(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    OTIdentifier FindNym(const Identifier& nymID) const override;
    OTIdentifier FindNym(
        const Identifier& nymID,
        const Identifier& serverIDHint) const override;
    OTIdentifier FindServer(const Identifier& serverID) const override;
    const Identifier& IntroductionServer() const override;
    OTIdentifier MessageContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        const std::string& message) const override;
    std::pair<ThreadStatus, OTIdentifier> MessageStatus(
        const Identifier& taskID) const override;
    OTIdentifier PayContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment> payment) const override;
#if OT_CASH
    OTIdentifier PayContactCash(
        const Identifier& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy) const override;
#endif  // OT_CASH
    void Refresh() const override;
    std::uint64_t RefreshCount() const override;
    OTIdentifier RegisterNym(
        const Identifier& nymID,
        const Identifier& server,
        const bool setContactData,
        const bool forcePrimary = false) const override;
    OTIdentifier SetIntroductionServer(
        const ServerContract& contract) const override;
    OTIdentifier ScheduleDownloadAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const override;
    OTIdentifier ScheduleDownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    OTIdentifier ScheduleDownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const override;
    OTIdentifier ScheduleDownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    OTIdentifier ScheduleIssueUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID) const override;
    OTIdentifier SchedulePublishServerContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    OTIdentifier ScheduleRegisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID) const override;
    OTIdentifier ScheduleRegisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    OTIdentifier SendCheque(
        const Identifier& localNymID,
        const Identifier& sourceAccountID,
        const Identifier& recipientContactID,
        const Amount value,
        const std::string& memo,
        const Time validFrom,
        const Time validTo) const override;
    OTIdentifier SendExternalTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    OTIdentifier SendTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    void StartIntroductionServer(const Identifier& localNymID) const override;
    ThreadStatus Status(const Identifier& taskID) const override;

    ~Sync();

private:
    static const std::string DEFAULT_INTRODUCTION_SERVER;

    friend opentxs::Factory;

    /** ContextID: localNymID, serverID */
    using ContextID = std::pair<OTIdentifier, OTIdentifier>;
    /** MessageTask: recipientID, message */
    using MessageTask = std::pair<OTIdentifier, std::string>;
    /** PaymentTask: recipientID, payment */
    using PaymentTask =
        std::pair<OTIdentifier, std::shared_ptr<const OTPayment>>;
#if OT_CASH
    /** PayCashTask: recipientID, recipientCopyOfPurse, senderCopyOfPurse */
    using PayCashTask = std::tuple<
        OTIdentifier,
        std::shared_ptr<const Purse>,
        std::shared_ptr<const Purse>>;
#endif  // OT_CASH
    /** DepositPaymentTask: accountID, payment */
    using DepositPaymentTask =
        std::pair<OTIdentifier, std::shared_ptr<const OTPayment>>;
    /** SendTransferTask: localNymID, serverID, sourceAccountID, targetAccountID
     */
    using SendTransferTask =
        std::tuple<OTIdentifier, OTIdentifier, uint64_t, std::string>;

    struct OperationQueue {
        UniqueQueue<OTIdentifier> check_nym_;
        UniqueQueue<DepositPaymentTask> deposit_payment_;
        UniqueQueue<OTIdentifier> download_account_;
        UniqueQueue<OTIdentifier> download_contract_;
        UniqueQueue<bool> download_nymbox_;
        UniqueQueue<OTIdentifier> issue_unit_definition_;
        UniqueQueue<OTIdentifier> register_account_;
        UniqueQueue<bool> register_nym_;
        UniqueQueue<MessageTask> send_message_;
        UniqueQueue<PaymentTask> send_payment_;
#if OT_CASH
        UniqueQueue<PayCashTask> send_cash_;
#endif  // OT_CASH
        UniqueQueue<SendTransferTask> send_transfer_;
        UniqueQueue<OTIdentifier> publish_server_contract_;
    };

    ContextLockCallback lock_callback_;
    const Flag& running_;
    const api::client::Manager& client_;
    OTClient& ot_client_;
    mutable std::mutex introduction_server_lock_{};
    mutable std::mutex nym_fetch_lock_{};
    mutable std::mutex task_status_lock_{};
    mutable std::atomic<std::uint64_t> refresh_counter_{0};
    mutable std::map<ContextID, OperationQueue> operations_;
    mutable std::map<OTIdentifier, UniqueQueue<OTIdentifier>> server_nym_fetch_;
    UniqueQueue<OTIdentifier> missing_nyms_;
    UniqueQueue<OTIdentifier> missing_servers_;
    mutable std::map<ContextID, std::unique_ptr<std::thread>> state_machines_;
    mutable std::unique_ptr<OTIdentifier> introduction_server_id_;
    mutable std::map<OTIdentifier, ThreadStatus> task_status_;
    // taskID, messageID
    mutable std::map<OTIdentifier, OTIdentifier> task_message_id_;
    OTZMQListenCallback account_subscriber_callback_;
    OTZMQSubscribeSocket account_subscriber_;
    OTZMQListenCallback notification_listener_callback_;
    OTZMQPullSocket notification_listener_;
    OTZMQPublishSocket task_finished_;

    std::pair<bool, std::size_t> accept_incoming(
        const rLock& lock,
        const std::size_t max,
        const Identifier& accountID,
        ServerContext& context) const;
    void add_task(const Identifier& taskID, const ThreadStatus status) const;
    void associate_message_id(
        const Identifier& messageID,
        const Identifier& taskID) const;
    Depositability can_deposit(
        const OTPayment& payment,
        const Identifier& recipient,
        const Identifier& accountIDHint,
        OTIdentifier& depositServer,
        OTIdentifier& depositAccount) const;
    Messagability can_message(
        const Identifier& senderID,
        const Identifier& recipientID,
        OTIdentifier& recipientNymID,
        OTIdentifier& serverID) const;
    void check_nym_revision(const ServerContext& context, OperationQueue& queue)
        const;
    bool check_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        std::shared_ptr<const ServerContext>& context) const;
    bool check_server_contract(const Identifier& serverID) const;
    OTIdentifier check_server_name(const ServerContext& context) const;
    bool deposit_cheque(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& cheque,
        UniqueQueue<DepositPaymentTask>& retry) const;
    bool download_account(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& accountID) const;
    bool download_contract(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& contractID) const;
    bool download_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const;
    bool download_nymbox(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID) const;
    bool extract_payment_data(
        const OTPayment& payment,
        OTIdentifier& nymID,
        OTIdentifier& serverID,
        OTIdentifier& unitID) const;
    bool find_nym(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetServerID) const;
    bool find_server(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const;
    bool finish_task(const Identifier& taskID, const bool success) const;
    bool get_admin(
        const Identifier& nymID,
        const Identifier& serverID,
        const OTPassword& password) const;
    OTIdentifier get_introduction_server(const Lock& lock) const;
    UniqueQueue<OTIdentifier>& get_nym_fetch(const Identifier& serverID) const;
    OperationQueue& get_operations(const ContextID& id) const;
    OTIdentifier import_default_introduction_server(const Lock& lock) const;
    bool issue_unit_definition(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    void load_introduction_server(const Lock& lock) const;
    bool message_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const std::string& text) const;
    bool pay_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        std::shared_ptr<const OTPayment>& payment) const;
#if OT_CASH
    bool pay_nym_cash(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy) const;
#endif  // OT_CASH
    void process_account(
        const opentxs::network::zeromq::Message& message) const;
    void process_notification(
        const opentxs::network::zeromq::Message& message) const;
    bool publish_server_contract(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& contractID) const;
    bool publish_server_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        const bool forcePrimary) const;
    bool queue_cheque_deposit(const Identifier& nymID, const Cheque& cheque)
        const;
    void refresh_accounts() const;
    void refresh_contacts() const;
    bool register_account(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    bool register_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID) const;
    OTIdentifier schedule_download_nymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const;
    OTIdentifier schedule_register_account(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    bool send_transfer(
        const Identifier& taskID,
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const int64_t value,
        const std::string& memo) const;
    void set_contact(const Identifier& nymID, const Identifier& serverID) const;
    OTIdentifier set_introduction_server(
        const Lock& lock,
        const ServerContract& contract) const;
    OTIdentifier start_task(const Identifier& taskID, bool success) const;
    void state_machine(const ContextID id, OperationQueue& queue) const;
    ThreadStatus status(const Lock& lock, const Identifier& taskID) const;
    void update_task(const Identifier& taskID, const ThreadStatus status) const;
    void start_introduction_server(const Identifier& nymID) const;
    Depositability valid_account(
        const OTPayment& payment,
        const Identifier& recipient,
        const Identifier& serverID,
        const Identifier& unitID,
        const Identifier& accountIDHint,
        OTIdentifier& depositAccount) const;
    bool valid_context(const Identifier& nymID, const Identifier& serverID)
        const;
    Depositability valid_recipient(
        const OTPayment& payment,
        const Identifier& specifiedNymID,
        const Identifier& recipient) const;

    Sync(
        const Flag& running,
        const api::client::Manager& client,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    Sync() = delete;
    Sync(const Sync&) = delete;
    Sync(Sync&&) = delete;
    Sync& operator=(const Sync&) = delete;
    Sync& operator=(Sync&&) = delete;
};
}  // namespace opentxs::api::client::implementation
