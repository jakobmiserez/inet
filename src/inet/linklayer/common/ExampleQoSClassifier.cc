//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2010 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "ExampleQoSClassifier.h"
#include "UserPriority.h"

#ifdef WITH_IPv4
#  include "inet/networklayer/ipv4/IPv4Header.h"
#  include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif
#ifdef WITH_IPv6
#  include "inet/networklayer/ipv6/IPv6Datagram.h"
#  include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif
#ifdef WITH_UDP
#  include "inet/transportlayer/udp/UdpHeader.h"
#endif
#ifdef WITH_TCP_COMMON
#  include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif

namespace inet {

Define_Module(ExampleQoSClassifier);

void ExampleQoSClassifier::initialize()
{
    //TODO parameters
}

void ExampleQoSClassifier::handleMessage(cMessage *msg)
{
    msg->ensureTag<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int ExampleQoSClassifier::getUserPriority(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    int ipProtocol = -1;
    bit ipHeaderLength = bit(-1);

#ifdef WITH_IPv4
    if (packet->getMandatoryTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv4) {
        const auto& ipv4Header = packet->peekHeader<IPv4Header>();
        if (ipv4Header->getTransportProtocol() == IP_PROT_ICMP)
            return UP_BE; // ICMP class
        ipProtocol = ipv4Header->getTransportProtocol();
        ipHeaderLength = ipv4Header->getChunkLength();
    }
#endif

#ifdef WITH_IPv6
    if (packet->getMandatoryTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv6) {
        const auto& ipv6Header = packet->peekHeader<IPv6Datagram>();
        if (ipv6Header->getTransportProtocol() == IP_PROT_IPv6_ICMP)
            return UP_BE; // ICMPv6 class
        ipProtocol = ipv6Header->getTransportProtocol();
        ipHeaderLength = ipv6Header->getChunkLength();
    }
#endif

    if (ipProtocol == -1)
        return UP_BE;

#ifdef WITH_UDP
    if (ipProtocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipHeaderLength);
        unsigned int srcPort = udpHeader->getSourcePort();
        unsigned int destPort = udpHeader->getDestinationPort();
        if (destPort == 21 || srcPort == 21)
            return UP_BK;
        if (destPort == 80 || srcPort == 80)
            return UP_BE;
        if (destPort == 4000 || srcPort == 4000)
            return UP_VI;
        if (destPort == 5000 || srcPort == 5000)
            return UP_VO;
        if (destPort == 6000 || srcPort == 6000) // not classified
            return -1;
    }
#endif

#ifdef WITH_TCP_COMMON
    if (ipProtocol == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipHeaderLength);
        unsigned int srcPort = tcpHeader->getSourcePort();
        unsigned int destPort = tcpHeader->getDestinationPort();
        if (destPort == 21 || srcPort == 21)
            return UP_BK;
        if (destPort == 80 || srcPort == 80)
            return UP_BE;
        if (destPort == 4000 || srcPort == 4000)
            return UP_VI;
        if (destPort == 5000 || srcPort == 5000)
            return UP_VO;
        if (tcp->getDestinationPort() == 6000 || tcp->getSourcePort() == 6000) // not classified
            return -1;
    }
#endif

    return UP_BE;
}

} // namespace inet

