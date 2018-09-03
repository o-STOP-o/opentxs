// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define TEST_NYM_1 "testNym1"
#define TEST_NYM_2 "testNym2"

using namespace opentxs;

namespace
{

class Test_Rpc : public ::testing::Test
{
public:
    Test_Rpc()
        : ot_{opentxs::OT::App()}
    {
        //        if (nullptr == notification_callback_) {
        //            notification_callback_ = new OTZMQListenCallback(
        //                network::zeromq::ListenCallback::Factory(
        //                    [](const network::zeromq::Message& incoming) ->
        //                    void {
        //                        process_notification(incoming);
        //                    }));
        //        }
        //        if (nullptr == notification_socket_) {
        //            notification_socket_ = new OTZMQSubscribeSocket(
        //                ot_.ZMQ().SubscribeSocket(*notification_callback_));
        //        }
    }

protected:
    const opentxs::api::Native& ot_;

    //    static std::atomic_bool send_payment_complete_;
    //    static OTZMQListenCallback* notification_callback_;
    //    static OTZMQSubscribeSocket* notification_socket_;
    //    static std::map<const std::string, proto::RPCCommand>
    //    queued_commands_;

    static std::size_t get_index(const std::int32_t instance);
    static const api::Core& get_session(const std::int32_t instance);
    //    static const std::string find_account(const proto::RPCCommand&
    //    command); static void process_notification(const
    //    network::zeromq::Message& incoming);

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = opentxs::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(1);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }

    bool add_session(proto::RPCCommandType commandtype, ArgList& args)
    {
        auto command = init(commandtype);
        command.set_session(-1);
        for (auto& arg : args) {
            auto apiarg = command.add_arg();
            apiarg->set_version(1);
            apiarg->set_key(arg.first);
            apiarg->add_value(*arg.second.begin());
        }
        auto response = ot_.RPC(command);

        return proto::RPCRESPONSE_SUCCESS == response.success();
    }

    void list(proto::RPCCommandType commandtype, std::int32_t session = -1)
    {
        auto command = init(commandtype);
        command.set_session(session);

        auto response = ot_.RPC(command);

        ASSERT_EQ(1, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        ASSERT_EQ(proto::RPCRESPONSE_NONE, response.success());
    }
};

// std::atomic_bool Test_Rpc::send_payment_complete_{false};
// OTZMQListenCallback* Test_Rpc::notification_callback_{nullptr};
// OTZMQSubscribeSocket* Test_Rpc::notification_socket_{nullptr};
// std::map<const std::string, proto::RPCCommand> Test_Rpc::queued_commands_{};

std::size_t Test_Rpc::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
};

const api::Core& Test_Rpc::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return opentxs::OT::App().Server(get_index(instance));
    } else {
        return opentxs::OT::App().Client(get_index(instance));
    }
};

// const std::string Test_Rpc::find_account(const proto::RPCCommand& command)
//{
//    auto& session = get_session(command.session());
//
//    const auto ownerid = Identifier::Factory(command.owner()),
//               notaryid = Identifier::Factory(command.notary()),
//               unitdefinitionid = Identifier::Factory(command.unit());
//
//    const auto owneraccounts = session.Storage().AccountsByOwner(ownerid);
//    const auto serveraccounts = session.Storage().AccountsByServer(notaryid);
//
//    std::set<OTIdentifier> accounts;
//    std::set_intersection(
//        owneraccounts.begin(),
//        owneraccounts.end(),
//        serveraccounts.begin(),
//        serveraccounts.end(),
//        std::inserter(accounts, accounts.end()));
//
//    auto contactitemtype =
//        session.Wallet().CurrencyTypeBasedOnUnitType(unitdefinitionid);
//    const auto unitaccounts =
//    session.Storage().AccountsByUnit(contactitemtype);
//
//    std::set<OTIdentifier> accounts2;
//    std::set_intersection(
//        accounts.begin(),
//        accounts.end(),
//        unitaccounts.begin(),
//        unitaccounts.end(),
//        std::inserter(accounts2, accounts2.end()));
//
//    if (0 != accounts2.size()) {
//        const auto& account = *accounts2.begin();
//        return account->str();
//    }
//
//    return "";
//};
//
// void Test_Rpc::process_notification(const network::zeromq::Message& incoming)
//{
//    ASSERT_EQ(2, incoming.Body().size());
//
//    const std::string taskid = incoming.Body().at(0);
//    const auto& frame = incoming.Body().at(1);
//    const auto data = Data::Factory(frame.data(), frame.size());
//    const auto completed = static_cast<bool>(data->at(0));
//
//    if (queued_commands_.find(taskid) == queued_commands_.end()) { return; }
//
//    proto::RPCCommand command;
//    try {
//        command = queued_commands_.at(taskid);
//    } catch (const std::out_of_range&) {
//        ASSERT_EQ(command.version(), 1);  // This should fail.
//    }
//
//    if (completed) {
//        if (command.type() == proto::RPCCOMMAND_CREATEACCOUNT/* ||
//            command.type() == proto::RPCCOMMAND_ISSUEUNITDEFINITION*/) {
//            //            accountid_ = find_account(command);
//
//            // if (!accountid.empty()) { output.add_identifier(accountid); }
//        } else if (command.type() == proto::RPCCOMMAND_REGISTERNYM) {
//        } else if (command.type() == proto::RPCCOMMAND_SENDPAYMENT) {
//            send_payment_complete_ = true;
//        }
//    }
//
//    queued_commands_.erase(taskid);
//};

TEST_F(Test_Rpc, List_Client_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTCLIENTSSESSIONS);
}

TEST_F(Test_Rpc, List_Server_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTSERVERSSESSIONS);
}

// The client created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Client_Session)
{
    auto command = init(proto::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(0, response.session());

    //    auto& manager = get_session(response.session());
    //    auto started =
    //        notification_socket_->get().Start(manager.Endpoints().TaskComplete());
    //
    //    ASSERT_TRUE(started);
}

// The server created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Server_Session)
{
    auto command = init(proto::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.session());

    auto& manager = get_session(response.session());
    //    auto started =
    //        notification_socket_->get().Start(manager.Endpoints().TaskComplete());
    //
    //    ASSERT_TRUE(started);

    // Register the server on the client.
    auto& servermanager = dynamic_cast<const api::server::Manager&>(manager);
    auto servercontract = servermanager.Wallet().Server(servermanager.ID());

    auto& client = get_session(0);
    auto& clientmanager = dynamic_cast<const api::client::Manager&>(client);
    auto clientservercontract =
        clientmanager.Wallet().Server(servercontract->PublicContract());

    // Make the server the introduction server.
    clientmanager.Sync().SetIntroductionServer(*clientservercontract);
}

TEST_F(Test_Rpc, List_Client_Sessions)
{
    ArgList args;
    auto added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    ASSERT_TRUE(added);

    added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    ASSERT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTCLIENTSSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(3, response.sessions_size());

    for (auto& session : response.sessions()) {
        ASSERT_EQ(1, session.version());
        ASSERT_TRUE(
            0 == session.instance() || 2 == session.instance() ||
            4 == session.instance());
    }
}

TEST_F(Test_Rpc, List_Server_Sessions)
{
    // These args never make it to the server.  Uncomment these lines and the
    // check for the number of sessions (below) when this issue is resolved.

    //    ArgList args{{OPENTXS_ARG_COMMANDPORT, {"7086"}}};
    //
    //    auto added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    //    ASSERT_TRUE(added);
    //
    //    auto& values = args[OPENTXS_ARG_COMMANDPORT];
    //    values.clear();
    //    values.emplace("7087");
    //
    //    added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    //    ASSERT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTSERVERSSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    //    ASSERT_EQ(3, response.sessions_size());
    ASSERT_EQ(1, response.sessions_size());

    for (auto& session : response.sessions()) {
        ASSERT_EQ(1, session.version());
        ASSERT_TRUE(
            1 == session.instance() || 3 == session.instance() ||
            5 == session.instance());
    }
}

// The nym created in this test is used in subsequent tests.
TEST_F(Test_Rpc, Create_Nym)
{
    // Add tests for specifying the seedid and index (not -1).
    // Add tests for adding claims.

    auto command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    auto createnym = command.mutable_createnym();

    ASSERT_NE(nullptr, createnym);

    createnym->set_version(1);
    createnym->set_type(proto::CITEMTYPE_INDIVIDUAL);
    createnym->set_name(TEST_NYM_1);
    createnym->set_index(-1);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 != response.identifier_size());

    // Now create a second nym for later tests.
    createnym->set_name(TEST_NYM_2);

    response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());

    ASSERT_TRUE(0 != response.identifier_size());
}

TEST_F(Test_Rpc, List_Unit_Definitions_None)
{
    list(proto::RPCCOMMAND_LISTUNITDEFINITIONS, 0);
}

TEST_F(Test_Rpc, Create_Unit_Definition)
{
    auto command = init(proto::RPCCOMMAND_CREATEUNITDEFINITION);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto def = command.mutable_createunit();

    ASSERT_NE(nullptr, def);

    def->set_version(1);
    def->set_name("GoogleTestDollar");
    def->set_symbol("G");
    def->set_primaryunitname("gdollar");
    def->set_fractionalunitname("gcent");
    def->set_tla("GTD");
    def->set_power(2);
    def->set_terms("Google Test Dollars");
    def->set_unitofaccount(proto::CITEMTYPE_USD);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 != response.identifier_size());
}

TEST_F(Test_Rpc, List_Unit_Definitions)
{
    auto command = init(proto::RPCCOMMAND_LISTUNITDEFINITIONS);
    command.set_session(0);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, RegisterNym)
{
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 == response.identifier_size());

    // Register the second nym.
    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_2);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 == response.identifier_size());
}

TEST_F(Test_Rpc, List_Accounts_None)
{
    list(proto::RPCCOMMAND_LISTACCOUNTS, 0);
}

TEST_F(Test_Rpc, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    const auto unitdefinitionlist = manager.Wallet().UnitDefinitionList();
    ASSERT_TRUE(!unitdefinitionlist.empty());

    auto& unitid = unitdefinitionlist.front().first;
    command.set_unit(unitid);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    const auto unitdefinitionlist = manager.Wallet().UnitDefinitionList();
    ASSERT_TRUE(!unitdefinitionlist.empty());

    auto& unitid = unitdefinitionlist.front().first;
    command.set_unit(unitid);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(1 == response.identifier_size());

    // Create an account for the second nym.
    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_2);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(1 == response.identifier_size());
}

TEST_F(Test_Rpc, Send_Payment_Cheque)
{
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);

    auto nym2 = manager.Wallet().NymByIDPartialMatch(TEST_NYM_2);

    ASSERT_TRUE(bool(nym2));

    auto& contacts = manager.Contacts();
    const auto contactid = contacts.ContactID(nym2->ID());

    ASSERT_FALSE(contactid->empty());

    sendpayment->set_contact(contactid->str());

    const auto accounts = manager.Storage().AccountsByOwner(nym->ID());

    ASSERT_TRUE(!accounts.empty());
    auto sourceaccountid = *accounts.cbegin();
    sendpayment->set_sourceaccount(sourceaccountid->str());

    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
}

}  // namespace
