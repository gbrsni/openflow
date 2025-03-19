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

#include "openflow/controllerApps/FlowTablePreloader.h"
#include "openflow/messages/OFP_Features_Reply_m.h"

Define_Module(FlowTablePreloader);


FlowTablePreloader::FlowTablePreloader() {

}

FlowTablePreloader::~FlowTablePreloader() {
}

void FlowTablePreloader::initialize(int stage){
    AbstractControllerApp::initialize(stage);
    readFlowtableConfiguration(Flow_Table& flowTable);
}

void FlowTablePreloader::readFlowtableConfiguration(Flow_Table& flowTable) {
    using namespace xmlutils;

    cXMLElementList entryElements = configuration->getChildrenByTagName("entry");

    for (auto& entryElement : entryElements) {
        const char* action_outputAttr = entryElement->getAttribute("action_output"); // I don't like mixing cases like this but trying to follow multiple standards led to this...

        try {
            Matcher outputMatcher(action_outputAttr);
        } catch (std::exception& e) {
            throw cRuntimeError("Error in XML <entry> element at %s: %s", entryElement->getSourceLocation(), e.what());
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
