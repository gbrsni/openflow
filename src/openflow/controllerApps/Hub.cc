#include "openflow/controllerApps/Hub.h"

Define_Module(Hub);

Hub::Hub(){

}

Hub::~Hub(){

}

void Hub::initialize(int stage){
    AbstractControllerApp::initialize(stage);
}

void Hub::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    AbstractControllerApp::receiveSignal(src,id,obj,details);
    Enter_Method("Hub::receiveSignal %s", cComponent::getSignalName(id));
    if(id == PacketInSignalId){
        EV << "Hub::PacketIn" << '\n';
        auto pkt = dynamic_cast<Packet *>(obj);
        if (pkt != nullptr) {
            auto chunk = pkt->peekAtFront<Chunk>();
            auto packet_in_msg = dynamicPtrCast<const OFP_Packet_In>(chunk);
            if (packet_in_msg != nullptr)
                doHubbing(pkt);
        }
    }
//
//    if(id == PacketInSignalId){
//        EV << "Hub::PacketIn" << '\n';
//        if (dynamic_cast<OFP_Packet_In *>(obj) != NULL) {
//            OFP_Packet_In *packet_in = (OFP_Packet_In *) obj;
//            floodPacket(packet_in);
//        }
//    }
}

void Hub::doHubbing(Packet *packet_in_msg){
    auto socket = controller->findSocketFor(packet_in_msg);
    sendFlowModFlood(socket);
    floodPacket(packet_in_msg);
}

void Hub::sendFlowModFlood(TcpSocket* socket){
    //OFP_Flow_Mod *flow_mod_msg = new OFP_Flow_Mod("flow_mod");
    auto flow_mod_msg = makeShared<OFP_Flow_Mod>();
    auto pkt = new Packet("flow_mod");

    oxm_basic_match match = oxm_basic_match();

    match.wildcards= 0;
    match.wildcards |= OFPFW_ALL;

    flow_mod_msg->getHeaderForUpdate().version = OFP_VERSION;
    flow_mod_msg->getHeaderForUpdate().type = OFPT_FLOW_MOD;
    flow_mod_msg->setCommand(OFPFC_ADD);
    flow_mod_msg->setMatch(match);
    flow_mod_msg->setChunkLength(B(56));
    flow_mod_msg->setHard_timeout(100);
    flow_mod_msg->setIdle_timeout(100);
    ofp_action_output *action_output = new ofp_action_output();
    action_output->port = OFPP_FLOOD;
    flow_mod_msg->setActionsArraySize(1);
    flow_mod_msg->setActions(0, *action_output);
    pkt->insertAtFront(flow_mod_msg);

    pkt->setKind(TCP_C_SEND);

    controller->sendPacket(socket, pkt);
}
