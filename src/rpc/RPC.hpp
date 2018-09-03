// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::rpc::implementation
{
class RPC final : virtual public rpc::internal::RPC, Lockable
{
public:
    proto::RPCResponse Process(const proto::RPCCommand& command) const override;

    ~RPC() = default;

private:
    friend opentxs::Factory;

    using Args = const ::google::protobuf::RepeatedPtrField<
        ::opentxs::proto::APIArgument>;

    const api::Native& ot_;

    static ArgList get_args(const Args& serialized);
    static std::size_t get_index(std::int32_t instance);
    static proto::RPCResponse init(const proto::RPCCommand& command);
    static proto::RPCResponse invalid_command(const proto::RPCCommand& command);
    static proto::AccountEventType storagebox_to_accounteventtype(
        StorageBox storagebox);

    proto::RPCResponse add_claim(const proto::RPCCommand& command) const;
    proto::RPCResponse create_account(const proto::RPCCommand& command) const;
    proto::RPCResponse create_issuer_account(
        const proto::RPCCommand& command) const;
    proto::RPCResponse create_nym(const proto::RPCCommand& command) const;
    proto::RPCResponse create_unit_definition(
        const proto::RPCCommand& command) const;
    proto::RPCResponse delete_claim(const proto::RPCCommand& command) const;
    const api::client::Manager* get_client(std::int32_t instance) const;
    proto::RPCResponse get_account_activity(
        const proto::RPCCommand& command) const;
    proto::RPCResponse get_account_balance(
        const proto::RPCCommand& command) const;
    proto::RPCResponse get_nyms(const proto::RPCCommand& command) const;
    proto::RPCResponse get_seeds(const proto::RPCCommand& command) const;
    const api::Core& get_session(std::int32_t instance) const;
    proto::RPCResponse import_seed(const proto::RPCCommand& command) const;
    proto::RPCResponse import_server_contract(
        const proto::RPCCommand& command) const;
    bool is_session_valid(std::int32_t instance) const;
    proto::RPCResponse list_accounts(const proto::RPCCommand& command) const;
    proto::RPCResponse list_client_sessions(
        const proto::RPCCommand& command) const;
    proto::RPCResponse list_seeds(const proto::RPCCommand& command) const;
    proto::RPCResponse list_nyms(const proto::RPCCommand& command) const;
    proto::RPCResponse list_server_contracts(
        const proto::RPCCommand& command) const;
    proto::RPCResponse list_server_sessions(
        const proto::RPCCommand& command) const;
    proto::RPCResponse list_unit_definitions(
        const proto::RPCCommand& command) const;
    proto::RPCResponse move_funds(const proto::RPCCommand& command) const;
    proto::RPCResponse register_nym(const proto::RPCCommand& command) const;
    proto::RPCResponse send_payment(const proto::RPCCommand& command) const;
    proto::RPCResponse start_client(const proto::RPCCommand& command) const;
    proto::RPCResponse start_server(const proto::RPCCommand& command) const;

    RPC(const api::Native& native);
    RPC() = delete;
    RPC(const RPC&) = delete;
    RPC(RPC&&) = delete;
    RPC& operator=(const RPC&) = delete;
    RPC& operator=(RPC&&) = delete;
};
}  // namespace opentxs::rpc::implementation
