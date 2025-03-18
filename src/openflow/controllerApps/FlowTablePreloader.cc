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

#include "FlowTablePreloader.h"

Define_Module(FlowTablePreloader);


FlowTablePreloader::FlowTablePreloader() {

}

FlowTablePreloader::~FlowTablePreloader() {
}

void FlowTablePreloader::initialize(int stage){
    AbstractControllerApp::initialize(stage);
}

void FlowTablePreloader::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    AbstractControllerApp::receiveSignal(src,id,obj,details);
    Enter_Method("Hub::receiveSignal %s", cComponent::getSignalName(id));
    if(id == PacketInSignalId){
        EV << "Hub::PacketIn" << '\n';
        auto pkt = dynamic_cast<Packet *>(obj);
        if (pkt != nullptr) {
            auto chunk = pkt->peekAtFront<Chunk>();
            auto packet_in_msg = dynamicPtrCast<const OFP_Packet_In>(chunk);
            if (packet_in_msg != nullptr)
                dropPacket(pkt);
        }
    }
}
