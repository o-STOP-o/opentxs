// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#define VERSION_POSITION 0
#define REQUEST_ID_POSITION 1
#define DOMAIN_POSITION 2
#define ADDRESS_POSITION 3
#define IDENTITY_POSITION 4
#define MECHANISM_POSITION 5
#define CREDENTIALS_START_POSITION 6

namespace opentxs::network::zeromq::zap::implementation
{
class Request final : virtual zap::Request, zeromq::implementation::Message
{
public:
    std::string Address() const override { return Body_at(ADDRESS_POSITION); }
    const FrameSection Credentials() const override;
    std::string Debug() const override;
    std::string Domain() const override { return Body_at(DOMAIN_POSITION); }
    OTData Identity() const override
    {
        return Data::Factory(Body_at(IDENTITY_POSITION));
    }
    zap::Mechanism Mechanism() const override
    {
        return string_to_mechanism(Body_at(MECHANISM_POSITION));
    }
    OTData RequestID() const override
    {
        return Data::Factory(Body_at(REQUEST_ID_POSITION));
    }
    std::pair<bool, std::string> Validate() const override;
    std::string Version() const override { return Body_at(VERSION_POSITION); }

    Frame& AddCredential(const Data& credential) override
    {
        return AddFrame(credential);
    };

    ~Request() = default;

private:
    friend network::zeromq::zap::Request;

    using MechanismMap = std::map<zap::Mechanism, std::string>;
    using MechanismReverseMap = std::map<std::string, zap::Mechanism>;

    static const std::set<std::string> accept_versions_;
    static const MechanismMap mechanism_map_;
    static const MechanismReverseMap mechanism_reverse_map_;

    static MechanismReverseMap invert_mechanism_map(const MechanismMap& input);
    static std::string mechanism_to_string(const zap::Mechanism in);
    static zap::Mechanism string_to_mechanism(const std::string& in);

    Request* clone() const override { return new Request(*this); }

    Request(
        const std::string& address,
        const std::string& domain,
        const zap::Mechanism mechanism,
        const Data& requestID,
        const Data& identity,
        const std::string& version);
    Request();
    Request(const Request&);
    Request(Request&&) = default;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = default;
};
}  // namespace opentxs::network::zeromq::zap::implementation
