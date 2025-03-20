//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/common/XMLUtils.h"
#include "openflow/controllerApps/FlowTablePreloader.h"
#include "openflow/messages/OFP_Features_Reply_m.h"

Define_Module(FlowTablePreloader);


FlowTablePreloader::FlowTablePreloader() {

}

FlowTablePreloader::~FlowTablePreloader() {
}

void FlowTablePreloader::initialize(int stage){
    AbstractControllerApp::initialize(stage);
    configuration = par("config");
    readFlowtableConfiguration();
}

void FlowTablePreloader::readFlowtableConfiguration() {
    using namespace xmlutils;

    cXMLElementList entryElements = configuration->getChildrenByTagName("entry");

    for (auto& entryElement : entryElements) {
        const char* action_outputAttr = entryElement->getAttribute("action_output"); // I don't like mixing cases like this but trying to follow multiple standards led to this...
        const char* idleTimeoutAttr = entryElement->getAttribute("idleTimeout");
        const char* hardTimeoutAttr = entryElement->getAttribute("hardTimeout");

        try {
            outport = static_cast<uint32_t>(std::stoul(action_outputAttr));
            idleTimeout = static_cast<uint32_t>(std::stoul(action_outputAttr));
            hardTimeout = static_cast<uint32_t>(std::stoul(action_outputAttr));
        } catch (std::exception& e) {
            throw cRuntimeError("Error in XML <entry> element at %s: %s", entryElement->getSourceLocation(), e.what());
        }

        cXMLElementList matchElements = configuration->getChildrenByTagName("match");

        for (auto& matchElement : matchElements) {
            match = oxm_basic_match();

            const char* in_portAttr = entryElement->getAttribute("in_port");

            const char* eth_dstAttr = entryElement->getAttribute("eth_dst");
            const char* eth_srcAttr = entryElement->getAttribute("eth_src");
            const char* eth_typeAttr = entryElement->getAttribute("eth_type");

            const char* ipv4_dstAttr = entryElement->getAttribute("ipv4_dst");

            const char* arp_opAttr = entryElement->getAttribute("arp_op");
            const char* arp_spaAttr = entryElement->getAttribute("arp_spa");
            const char* arp_tpaAttr = entryElement->getAttribute("arp_tpa");
            const char* arp_shaAttr = entryElement->getAttribute("arp_sha");
            const char* arp_thaAttr = entryElement->getAttribute("arp_tha");

            const char* wildcardsAttr = entryElement->getAttribute("wildcards");

            int in_port;

            MacAddress eth_dst;
            MacAddress eth_src;
            int eth_type;

            Ipv4Address ipv4_dst;

            int arp_op;
            Ipv4Address arp_spa;
            Ipv4Address arp_tpa;
            MacAddress arp_sha;
            MacAddress arp_tha;

            try {
                in_port = static_cast<int>(std::stoul(action_outputAttr));

                eth_dst = MacAddress(eth_dstAttr);
                eth_src = MacAddress(eth_srcAttr);
                eth_type = static_cast<int>(std::stoul(eth_typeAttr));

                ipv4_dst = Ipv4Address(ipv4_dstAttr);
            } catch (std::exception& e) {
                throw cRuntimeError("Error in XML <match> element at %s: %s", matchElement->getSourceLocation(), e.what());
            }

        }
    }
}

void FlowTablePreloader::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    EV << "FlowTablePreloader::receiveSignal" << '\n';
    AbstractControllerApp::receiveSignal(src,id,obj,details);
    Enter_Method("FlowTablePreloader::receiveSignal %s", cComponent::getSignalName(id));
    if(id == PacketFeatureReplySignalId){
        EV << "FlowTablePreloader::FeatureReply" << '\n';
        auto pkt = dynamic_cast<Packet *>(obj);
        if (pkt != nullptr) {
            auto chunk = pkt->peekAtFront<Chunk>();
            auto packet_in_msg = dynamicPtrCast<const OFP_Features_Reply>(chunk);
            if (packet_in_msg != nullptr)
                sendFlowTables(pkt);
        }
    }
//    if(id == PacketInSignalId){
//        EV << "FlowTablePreloader::PacketIn" << '\n';
//        auto pkt = dynamic_cast<Packet *>(obj);
//        if (pkt != nullptr) {
//            auto chunk = pkt->peekAtFront<Chunk>();
//            auto packet_in_msg = dynamicPtrCast<const OFP_Packet_In>(chunk);
//            if (packet_in_msg != nullptr)
//                dropPacket(pkt);
//        }
//    }
}

void FlowTablePreloader::sendFlowTables(Packet* pkt){
    oxm_basic_match match = oxm_basic_match();

    match.wildcards= 0;
    match.wildcards |= OFPFW_ALL;

    uint32_t outport = OFPP_FLOOD;

    auto socket = controller->findSocketFor(pkt);

    int idleTimeout = -1;
    int hardTimeout = -1;

    sendFlowModMessage(OFPFC_ADD, match, outport, socket, idleTimeout, hardTimeout);
}
