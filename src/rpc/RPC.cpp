// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/Proto.hpp"

#include "internal/rpc/Internal.hpp"

#include "RPC.hpp"

#define SESSION_DATA_VERSION 1

#define OT_METHOD "opentxs::rpc::implementation::RPC::"

namespace opentxs
{
rpc::internal::RPC* Factory::RPC(const api::Native& native)
{
    return new rpc::implementation::RPC(native);
}
}  // namespace opentxs

namespace opentxs::rpc::implementation
{
RPC::RPC(const api::Native& native)
    : Lockable()
    , ot_(native)
{
}

proto::RPCResponse RPC::add_claim(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);

    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    auto numadded = 0;
    auto nymdata =
        session.Wallet().mutable_Nym(Identifier::Factory(command.owner()));
    for (const auto& addclaim : command.claim()) {
        const auto& contactitem = addclaim.item();
        std::set<std::uint32_t> attributes(
            contactitem.attribute().begin(), contactitem.attribute().end());
        auto claim = Claim(
            contactitem.id(),
            addclaim.sectionversion(),
            contactitem.type(),
            contactitem.value(),
            contactitem.start(),
            contactitem.end(),
            attributes);
        auto added = nymdata.AddClaim(claim);
        if (added) { ++numadded; }
    }

    if (0 == numadded) {
        success = proto::RPCRESPONSE_NONE;
    } else if (numadded < command.claim_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    } else

        output.set_success(success);

    return output;
}

proto::RPCResponse RPC::create_account(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    const auto client = get_client(command.session());
    if (nullptr == client) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (false == client->Wallet().IsLocalNym(command.owner())) {
        output.set_success(proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerid = Identifier::Factory(command.owner()),
               notaryid = Identifier::Factory(command.notary()),
               unitdefinitionid = Identifier::Factory(command.unit());

    const auto unitdefinition =
        client->Wallet().UnitDefinition(unitdefinitionid);
    if (false == bool(unitdefinition)) {
        output.set_success(proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto registered =
        client->OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);
    if (false != registered) {
        auto action = client->ServerAction().RegisterAccount(
            ownerid, notaryid, unitdefinitionid);
        action->Run();
        if (SendResult::VALID_REPLY == action->LastSendResult()) {

            auto reply = action->Reply();

            if (false == bool(reply) || false == reply->m_bSuccess) {
                success = proto::RPCRESPONSE_ERROR;
            } else {
                output.add_identifier(action->Reply()->m_strAcctID.Get());
            }
        } else {
            success = proto::RPCRESPONSE_ERROR;
        }
    } else {
        const auto taskid = client->Sync().ScheduleRegisterAccount(
            ownerid, notaryid, unitdefinitionid);
        output.set_task(taskid->str());
        success = proto::RPCRESPONSE_QUEUED;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::create_issuer_account(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    const auto client = get_client(command.session());
    if (nullptr == client) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (false == client->Wallet().IsLocalNym(command.owner())) {
        output.set_success(proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerid = Identifier::Factory(command.owner()),
               notaryid = Identifier::Factory(command.notary()),
               unitdefinitionid = Identifier::Factory(command.unit());

    const auto unitdefinition =
        client->Wallet().UnitDefinition(unitdefinitionid);
    if (false == bool(unitdefinition) ||
        ownerid != unitdefinition->Nym()->ID()) {
        output.set_success(proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto account = client->Wallet().IssuerAccount(unitdefinitionid);
    if (false != bool(account)) {
        output.set_success(proto::RPCRESPONSE_UNNECESSARY);
        return output;
    }

    auto registered =
        client->OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);
    if (false != registered) {
        auto action = client->ServerAction().IssueUnitDefinition(
            ownerid, notaryid, unitdefinition->PublicContract());
        action->Run();
        if (SendResult::VALID_REPLY == action->LastSendResult()) {

            auto reply = action->Reply();

            if (false == bool(reply) || false == reply->m_bSuccess) {
                success = proto::RPCRESPONSE_ERROR;
            } else {
                output.add_identifier(action->Reply()->m_strAcctID.Get());
            }
        } else {
            success = proto::RPCRESPONSE_ERROR;
        }
    } else {
        const auto taskid = client->Sync().ScheduleIssueUnitDefinition(
            ownerid, notaryid, unitdefinitionid);
        output.set_task(taskid->str());
        success = proto::RPCRESPONSE_QUEUED;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::create_nym(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& createnym = command.createnym();

    auto identifier = session.Wallet().CreateNymHD(
        createnym.type(),
        createnym.name(),
        createnym.seedid(),
        createnym.index());

    if (identifier.empty()) {
        success = proto::RPCRESPONSE_ERROR;
    } else {
        if (0 < createnym.claims_size()) {
            auto nymdata =
                session.Wallet().mutable_Nym(Identifier::Factory(identifier));
            for (const auto& addclaim : createnym.claims()) {
                const auto& contactitem = addclaim.item();
                std::set<std::uint32_t> attributes(
                    contactitem.attribute().begin(),
                    contactitem.attribute().end());
                auto claim = Claim(
                    contactitem.id(),
                    addclaim.sectionversion(),
                    contactitem.type(),
                    contactitem.value(),
                    contactitem.start(),
                    contactitem.end(),
                    attributes);
                nymdata.AddClaim(claim);
            }
        }

        output.add_identifier(identifier);
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::create_unit_definition(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& createunit = command.createunit();
    auto unitdefinition = session.Wallet().UnitDefinition(
        command.owner(),
        createunit.primaryunitname(),
        createunit.name(),
        createunit.symbol(),
        createunit.terms(),
        createunit.tla(),
        createunit.power(),
        createunit.fractionalunitname());

    if (false != bool(unitdefinition)) {
        output.add_identifier(unitdefinition->ID()->str());
    } else {
        success = proto::RPCRESPONSE_ERROR;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::delete_claim(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    auto numdeleted = 0;
    auto nymdata =
        session.Wallet().mutable_Nym(Identifier::Factory(command.owner()));
    for (const auto& id : command.identifier()) {
        auto deleted = nymdata.DeleteClaim(Identifier::Factory(id));
        if (deleted) { ++numdeleted; }
    }

    if (0 == numdeleted) {
        success = proto::RPCRESPONSE_NONE;
    } else if (numdeleted < command.identifier_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::get_account_activity(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto client = get_client(command.session());

    for (const auto& id : command.identifier()) {
        const auto accountid = Identifier::Factory(id);
        auto& accountactivity = client->UI().AccountActivity(
            client->Storage().AccountOwner(accountid), accountid);

        auto balanceitem = accountactivity.First();
        while (balanceitem->Valid()) {
            auto accountevent = output.add_accountevent();
            accountevent->set_id(id);
            accountevent->set_type(
                storagebox_to_accounteventtype(balanceitem->Type()));
            if (0 < balanceitem->Contacts().size()) {
                accountevent->set_contact(balanceitem->Contacts().at(0));
            }
            accountevent->set_number(balanceitem->Number());
            accountevent->set_amount(balanceitem->Amount());
            accountevent->set_pendingamount(balanceitem->Amount());
            accountevent->set_timestamp(
                std::chrono::system_clock::to_time_t(balanceitem->Timestamp()));
            accountevent->set_memo(balanceitem->Memo());

            balanceitem = accountactivity.Next();
        }
    }

    if (0 == output.accountevent_size()) {
        success = proto::RPCRESPONSE_NONE;
    } else if (output.accountevent_size() < command.identifier_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::get_account_balance(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto accountid = Identifier::Factory(id);
        const auto account = session.Wallet().Account(accountid);
        if (false != bool(account)) {
            auto accountdata = output.add_balance();
            accountdata->set_version(1);
            accountdata->set_id(id);
            String name;
            account.get().GetName(name);
            accountdata->set_label(name.Get());
            accountdata->set_unit(
                account.get().GetInstrumentDefinitionID().str());
            accountdata->set_owner(
                session.Storage().AccountOwner(accountid)->str());
            accountdata->set_issuer(
                session.Storage().AccountIssuer(accountid)->str());
            accountdata->set_balance(account.get().GetBalance());
            accountdata->set_pendingbalance(account.get().GetBalance());
        }
    }

    if (0 == output.balance_size()) {
        success = proto::RPCRESPONSE_NONE;
    } else if (output.balance_size() < command.identifier_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

ArgList RPC::get_args(const Args& serialized)
{
    ArgList output{};

    for (const auto& arg : serialized) {
        auto& row = output[arg.key()];

        for (const auto& value : arg.value()) { row.emplace(value); }
    }

    return output;
}

const api::client::Manager* RPC::get_client(const std::int32_t instance) const
{
    auto is_server = instance % 2;

    if (is_server) {
        return nullptr;
    } else {
        return &ot_.Client(get_index(instance));
    }
}

std::size_t RPC::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

proto::RPCResponse RPC::get_nyms(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto nymdata =
            session.Wallet().mutable_Nym(Identifier::Factory(id));

        const auto credentialindex = nymdata.asPublicNym();

        auto pcredentialindex = output.add_nym();
        *pcredentialindex = credentialindex;
    }

    if (0 == output.nym_size()) {
        success = proto::RPCRESPONSE_NONE;
    } else if (output.nym_size() < command.identifier_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::get_seeds(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());
    const auto& hdseeds = session.Seeds();

    for (const auto& id : command.identifier()) {
        auto seed = output.add_seed();
        seed->set_id(id);
        seed->set_words(hdseeds.Words(id));
        seed->set_passphrase(hdseeds.Passphrase(id));
    }

    if (0 == output.seed_size()) {
        success = proto::RPCRESPONSE_NONE;
    } else if (output.seed_size() < command.identifier_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

const api::Core& RPC::get_session(const std::int32_t instance) const
{
    auto is_server = instance % 2;

    if (is_server) {
        return ot_.Server(get_index(instance));
    } else {
        return ot_.Client(get_index(instance));
    }
}

proto::RPCResponse RPC::import_seed(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    auto& seed = command.hdseed();
    OTPassword words;
    words.setPassword(seed.words());
    OTPassword passphrase;
    passphrase.setPassword(seed.passphrase());

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());
    const auto identifier = session.Seeds().ImportSeed(words, passphrase);
    if (identifier.empty()) {
        success = proto::RPCRESPONSE_INVALID;
    } else {
        output.add_identifier(identifier);
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::import_server_contract(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto added = 0;
    auto& session = get_session(command.session());
    for (const auto& servercontract : command.server()) {
        auto server = session.Wallet().Server(servercontract);
        if (false != bool(server)) { ++added; }
    }
    if (0 == added) {
        success = proto::RPCRESPONSE_NONE;
    } else if (added < command.server_size()) {
        success = proto::RPCRESPONSE_PARTIAL;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::init(const proto::RPCCommand& command)
{
    proto::RPCResponse output{};
    output.set_version(command.version());
    output.set_cookie(command.cookie());
    output.set_type(command.type());

    return output;
}

proto::RPCResponse RPC::invalid_command(const proto::RPCCommand& command)
{
    auto output = init(command);
    output.set_success(proto::RPCRESPONSE_INVALID);

    return output;
}

bool RPC::is_session_valid(std::int32_t instance) const
{
    auto is_server = instance % 2;

    auto index = get_index(instance);

    if (is_server) {
        return ot_.Servers() > index;
    } else {
        return ot_.Clients() > index;
    }
}

proto::RPCResponse RPC::list_accounts(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& list = session.Storage().AccountList();
    for (const auto& account : list) { output.add_identifier(account.first); }

    if (0 == output.identifier_size()) { success = proto::RPCRESPONSE_NONE; }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_client_sessions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    for (std::size_t i = 0; i < ot_.Clients(); ++i) {
        proto::SessionData* data = output.add_sessions();
        data->set_version(SESSION_DATA_VERSION);
        data->set_instance(ot_.Client(i).Instance());
    }

    if (0 == output.sessions_size()) { success = proto::RPCRESPONSE_NONE; }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_nyms(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& localnyms = session.Wallet().LocalNyms();
    if (0 < localnyms.size()) {
        for (const auto& id : localnyms) { output.add_identifier(id->str()); }
    } else {
        success = proto::RPCRESPONSE_NONE;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_seeds(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& seeds = session.Storage().SeedList();
    if (0 < seeds.size()) {
        for (const auto& seed : seeds) { output.add_identifier(seed.first); }
    } else {
        success = proto::RPCRESPONSE_NONE;
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_server_contracts(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& servers = session.Wallet().ServerList();
    for (const auto& server : servers) { output.add_identifier(server.first); }

    if (0 == output.identifier_size()) { success = proto::RPCRESPONSE_NONE; }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_server_sessions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    for (std::size_t i = 0; i < ot_.Servers(); ++i) {
        auto data = output.add_sessions();
        data->set_version(SESSION_DATA_VERSION);
        data->set_instance(ot_.Server(i).Instance());
    }

    if (0 == output.sessions_size()) { success = proto::RPCRESPONSE_NONE; }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::list_unit_definitions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    if (!is_session_valid(command.session())) {
        output.set_success(proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& unitdefinitions = session.Wallet().UnitDefinitionList();
    for (const auto& unitdefinition : unitdefinitions) {
        output.add_identifier(unitdefinition.first);
    }

    if (0 == output.identifier_size()) { success = proto::RPCRESPONSE_NONE; }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::move_funds(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    const auto client = get_client(command.session());
    if (nullptr == client) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto& movefunds = command.movefunds();

    const auto sourceaccount = Identifier::Factory(movefunds.sourceaccount());

    auto sender = client->Storage().AccountOwner(sourceaccount);

    switch (movefunds.type()) {
        case proto::RPCPAYMENTTYPE_TRANSFER: {
            const auto targetaccount =
                Identifier::Factory(movefunds.destinationaccount());

            const auto notary = client->Storage().AccountServer(sourceaccount);

            auto action = client->ServerAction().SendTransfer(
                sender,
                notary,
                sourceaccount,
                targetaccount,
                movefunds.amount(),
                movefunds.memo());
            action->Run();
            if (SendResult::VALID_REPLY == action->LastSendResult()) {
                auto reply = action->Reply();

                if (false == bool(reply) || false == reply->m_bSuccess) {
                    success = proto::RPCRESPONSE_ERROR;
                }
            } else {
                success = proto::RPCRESPONSE_ERROR;
            }
            break;
        }
        case proto::RPCPAYMENTTYPE_CHEQUE:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_VOUCHER:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_INVOICE:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_BLINDED:
            [[fallthrough]];
        default: {
            output.set_success(proto::RPCRESPONSE_INVALID);
            return output;
        }
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::Process(const proto::RPCCommand& command) const
{
    const auto valid = proto::Validate(command, VERBOSE);

    if (false == valid) { return invalid_command(command); }

    switch (command.type()) {
        case proto::RPCCOMMAND_ADDCLIENTSESSION: {
            return start_client(command);
        } break;
        case proto::RPCCOMMAND_ADDSERVERSESSION: {
            return start_server(command);
        } break;
        case proto::RPCCOMMAND_LISTCLIENTSSESSIONS: {
            return list_client_sessions(command);
        } break;
        case proto::RPCCOMMAND_LISTSERVERSSESSIONS: {
            return list_server_sessions(command);
        } break;
        case proto::RPCCOMMAND_IMPORTHDSEED: {
            return import_seed(command);
        } break;
        case proto::RPCCOMMAND_LISTHDSEEDS: {
            return list_seeds(command);
        } break;
        case proto::RPCCOMMAND_GETHDSEED: {
            return get_seeds(command);
        } break;
        case proto::RPCCOMMAND_CREATENYM: {
            return create_nym(command);
        } break;
        case proto::RPCCOMMAND_LISTNYMS: {
            return list_nyms(command);
        } break;
        case proto::RPCCOMMAND_GETNYM: {
            return get_nyms(command);
        } break;
        case proto::RPCCOMMAND_ADDCLAIM: {
            return add_claim(command);
        } break;
        case proto::RPCCOMMAND_DELETECLAIM: {
            return delete_claim(command);
        } break;
        case proto::RPCCOMMAND_IMPORTSERVERCONTRACT: {
            return import_server_contract(command);
        } break;
        case proto::RPCCOMMAND_LISTSERVERCONTRACTS: {
            return list_server_contracts(command);
        } break;
        case proto::RPCCOMMAND_REGISTERNYM: {
            return register_nym(command);
        } break;
        case proto::RPCCOMMAND_CREATEUNITDEFINITION: {
            return create_unit_definition(command);
        } break;
        case proto::RPCCOMMAND_LISTUNITDEFINITIONS: {
            return list_unit_definitions(command);
        } break;
        case proto::RPCCOMMAND_ISSUEUNITDEFINITION: {
            return create_issuer_account(command);
        } break;
        case proto::RPCCOMMAND_CREATEACCOUNT: {
            return create_account(command);
        } break;
        case proto::RPCCOMMAND_LISTACCOUNTS: {
            return list_accounts(command);
        } break;
        case proto::RPCCOMMAND_GETACCOUNTBALANCE: {
            return get_account_balance(command);
        } break;
        case proto::RPCCOMMAND_GETACCOUNTACTIVITY: {
            return get_account_activity(command);
        } break;
        case proto::RPCCOMMAND_SENDPAYMENT: {
            return send_payment(command);
        } break;
        case proto::RPCCOMMAND_MOVEFUNDS: {
            return move_funds(command);
        } break;
        case proto::RPCCOMMAND_ADDCONTACT:
        case proto::RPCCOMMAND_LISTCONTACTS:
        case proto::RPCCOMMAND_GETCONTACT:
        case proto::RPCCOMMAND_ADDCONTACTCLAIM:
        case proto::RPCCOMMAND_DELETECONTACTCLAIM:
        case proto::RPCCOMMAND_VERIFYCLAIM:
        case proto::RPCCOMMAND_ACCEPTVERIFICATION:
        case proto::RPCCOMMAND_SENDCONTACTMESSAGE:
        case proto::RPCCOMMAND_GETCONTACTACTIVITY:
        case proto::RPCCOMMAND_ERROR:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unsupported command"
                  << std::endl;
        }
    }

    return invalid_command(command);
}

proto::RPCResponse RPC::register_nym(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    const auto client = get_client(command.session());
    if (nullptr == client) {
        success = proto::RPCRESPONSE_INVALID;
    } else {
        const auto ownerid = Identifier::Factory(command.owner()),
                   notaryid = Identifier::Factory(command.notary());

        auto registered =
            client->OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);

        if (false == registered) {
            auto servercontract = client->Wallet().Server(notaryid);

            if (false != bool(servercontract)) {
                auto nym = client->Wallet().mutable_Nym(ownerid);
                const auto server = nym.PreferredOTServer();

                if (server.empty()) {
                    nym.AddPreferredOTServer(command.notary(), true);
                }

                auto action =
                    client->ServerAction().RegisterNym(ownerid, notaryid);
                action->Run();

                if (SendResult::VALID_REPLY == action->LastSendResult()) {
                    auto reply = action->Reply();

                    if (false == bool(reply) || false == reply->m_bSuccess) {
                        success = proto::RPCRESPONSE_ERROR;
                    }
                } else {
                    success = proto::RPCRESPONSE_ERROR;
                }
            } else {
                const auto taskid =
                    client->Sync().ScheduleRegisterNym(ownerid, notaryid);
                //                queued_commands_[taskid] = command;
                output.set_task(taskid->str());
                success = proto::RPCRESPONSE_QUEUED;
            }
        } else {
            success = proto::RPCRESPONSE_UNNECESSARY;
        }
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::send_payment(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    const auto client = get_client(command.session());
    if (nullptr == client) {
        output.set_success(proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto& sendpayment = command.sendpayment();

    const auto contactid = Identifier::Factory(sendpayment.contact()),
               sourceaccountid =
                   Identifier::Factory(sendpayment.sourceaccount());

    auto& contacts = client->Contacts();
    const auto contact = contacts.Contact(contactid);
    if (contact->Nyms().empty()) {
        output.set_success(proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto sender = client->Storage().AccountOwner(sourceaccountid);

    switch (sendpayment.type()) {
        case proto::RPCPAYMENTTYPE_CHEQUE: {
            if (Messagability::READY ==
                client->Sync().CanMessage(sender, contactid)) {
                auto notary = client->Storage().AccountServer(sourceaccountid);

                auto downloaded =
                    client->ServerAction().DownloadNymbox(sender, notary);
                if (!downloaded) {
                    success = proto::RPCRESPONSE_ERROR;
                    break;
                }

                auto gotnumbers = client->ServerAction().GetTransactionNumbers(
                    sender, notary, 10);
                if (!gotnumbers) {
                    success = proto::RPCRESPONSE_ERROR;
                    break;
                }

                const auto contactnym = contact->Nyms().front();

                std::unique_ptr<Cheque> cheque;
                cheque.reset(client->OTAPI().WriteCheque(
                    notary,
                    sendpayment.amount(),
                    std::chrono::system_clock::to_time_t(
                        std::chrono::system_clock::now()),
                    std::chrono::system_clock::to_time_t(
                        std::chrono::system_clock::now() +
                        std::chrono::hours(OT_CHEQUE_HOURS)),
                    sourceaccountid,
                    sender,
                    sendpayment.memo().c_str(),
                    contactnym));

                if (!bool(cheque)) {
                    success = proto::RPCRESPONSE_ERROR;
                    break;
                }

                const auto chequestring = String::Factory(*cheque);

                auto payment = client->Factory().Payment(chequestring.get());

                std::shared_ptr<const OTPayment> ppayment(payment.release());

                auto action = client->ServerAction().SendPayment(
                    sender, notary, contactnym, ppayment);
                action->Run();
                if (SendResult::VALID_REPLY == action->LastSendResult()) {
                    auto reply = action->Reply();

                    if (false == bool(reply) || false == reply->m_bSuccess) {
                        success = proto::RPCRESPONSE_ERROR;
                    }
                } else {
                    success = proto::RPCRESPONSE_ERROR;
                }
            } else {
                const auto taskid = client->Sync().SendCheque(
                    sender,
                    sourceaccountid,
                    contactid,
                    sendpayment.amount(),
                    sendpayment.memo(),
                    std::chrono::system_clock::now(),
                    std::chrono::system_clock::now() +
                        std::chrono::hours(OT_CHEQUE_HOURS));
                //                queued_commands_[taskid] = command;
                output.set_task(taskid->str());
                success = proto::RPCRESPONSE_QUEUED;
            }
            break;
        }
        case proto::RPCPAYMENTTYPE_TRANSFER: {
            const auto targetaccount =
                Identifier::Factory(sendpayment.destinationaccount());

            auto notary = client->Storage().AccountServer(sourceaccountid);

            auto action = client->ServerAction().SendTransfer(
                sender,
                notary,
                sourceaccountid,
                targetaccount,
                sendpayment.amount(),
                sendpayment.memo());
            action->Run();
            if (SendResult::VALID_REPLY == action->LastSendResult()) {
                auto reply = action->Reply();

                if (false == bool(reply) || false == reply->m_bSuccess) {
                    success = proto::RPCRESPONSE_ERROR;
                }
            } else {
                success = proto::RPCRESPONSE_ERROR;
            }
            break;
        }
        case proto::RPCPAYMENTTYPE_VOUCHER:
            // TODO
        case proto::RPCPAYMENTTYPE_INVOICE:
            // TODO
        case proto::RPCPAYMENTTYPE_BLINDED:
            // TODO
        default: {
            output.set_success(proto::RPCRESPONSE_INVALID);
            return output;
        }
    }

    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::start_client(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Clients())};
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    std::uint32_t instance{0};
    try {
        auto& manager = ot_.StartClient(get_args(command.arg()), session);
        instance = manager.Instance();
    } catch (...) {
        success = proto::RPCRESPONSE_INVALID;
    }

    output.set_session(instance);
    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::start_server(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Servers())};
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    std::uint32_t instance{0};
    try {
        auto& manager = ot_.StartServer(get_args(command.arg()), session);
        instance = manager.Instance();
    } catch (...) {
        success = proto::RPCRESPONSE_INVALID;
    }

    output.set_session(instance);
    output.set_success(success);

    return output;
}

proto::AccountEventType RPC::storagebox_to_accounteventtype(
    StorageBox storagebox)
{
    proto::AccountEventType accounteventtype = proto::ACCOUNTEVENT_ERROR;

    switch (storagebox) {
        case StorageBox::INCOMINGCHEQUE:
            accounteventtype = proto::ACCOUNTEVENT_INCOMINGCHEQUE;
            break;
        case StorageBox::OUTGOINGCHEQUE:
            accounteventtype = proto::ACCOUNTEVENT_OUTGOINGCHEQUE;
            break;
            //        case StorageBox::INCOMINGTRANSFER:
            //            accounteventtype =
            //            proto::ACCOUNTEVENT_INCOMINGTRANSFER; break;
            //        case StorageBox::OUTGOINGTRANSFER:
            //            accounteventtype =
            //            proto::ACCOUNTEVENT_OUTGOINGTRANSFER; break;
        default:
            break;
    }

    return accounteventtype;
}

}  // namespace opentxs::rpc::implementation
