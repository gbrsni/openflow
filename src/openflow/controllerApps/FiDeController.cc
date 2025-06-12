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
#include "openflow/controllerApps/FiDeController.h"
#include "openflow/messages/OFP_Features_Reply_m.h"

#include <algorithms.h>
#include <applications.h>
#include <Link.h>
#include <tools.h>
#include <Topology.h>
#include <Tunnel.h>

Define_Module(FiDeController);


FiDeController::FiDeController() {

}

FiDeController::~FiDeController() {
}

void FiDeController::initialize(int stage){
    AbstractControllerApp::initialize(stage);

    if (!flowConfigRead) {
        configuration = par("config");
        readFlowtableConfiguration();
        flowConfigRead = true;
    }

    // FiDe stuff
    // TE
    const std::vector<std::string> typenames = {"inet.node.inet.StandardHost", "openflow.openflow.switch.Open_Flow_Switch"};
    TrafficEngineering::Topology topology = TrafficEngineering::makeTopologyFromCurrentNetwork(typenames);
    log("TE Topology made");

    std::vector<TrafficEngineering::Tunnel> tunnels;

//    std::string const& sender = "0A-AA-00-00-61";
//    std::vector<std::string> const& receivers = {"0A-AA-00-00-52", "0A-AA-00-00-57"};

    std::string const& sender = "Scenario_DynamicFatTree.fat_tree.client[1]";
    std::vector<std::string> const& receivers = {"Scenario_DynamicFatTree.fat_tree.client[2]", "Scenario_DynamicFatTree.fat_tree.client[3]"};

    TrafficEngineering::MulticastRequest request;
    request.messageLength = 0;
    request.sendInterval = 0;
    request.appOwnerName = sender;
    request.appReceiverNames = receivers;

    TrafficEngineering::Tunnel tunnel = TrafficEngineering::optimization(topology, tunnels, request);

    log("Tunnel links: ");
    std::vector<TrafficEngineering::Link> links = tunnel.getAllLinks();
    for (auto i = links.begin(); i < links.end(); i++) {
        log("localNodeName: " + i->localNodeName);
//        log("localNodeName: " + i->remoteNodeName);
        log("localInterfaceName: " + i->localInterfaceName);
//        log("remoteInterfaceName: " + i->remoteInterfaceName);
    }

    // Mine
    // What this does is getting the MAC address on the control plane for the Open_Flow_Switch modules.
    // This is useful later since that is the ID they use in OF packets they send to the controller
//    const char* NodeType = "openflow.openflow.switch.Open_Flow_Switch";
//    int startNode = 0;
//
//
//    std::vector<std::string> nodeTypes = cStringTokenizer(NodeType).asVector();
    topo_spanntree.extractByNedTypeName(typenames);
    EV << "FiDeController cTopology found " << topo_spanntree.getNumNodes() << "\n";

    nodeInfo.resize(topo_spanntree.getNumNodes());
    for (int i = 0; i < topo_spanntree.getNumNodes(); i++) {
        nodeInfo[i].moduleID = topo_spanntree.getNode(i)->getModuleId();
        nodeInfo[i].treeNeighbors.resize(topo_spanntree.getNumNodes(),0);

        auto module = topo_spanntree.getNode(i)->getModule();
        auto modulePath = module->getFullPath();
        log("Module Path: " + modulePath);

//        int submoduleID = module->findSubmodule("eth[0]");
//        log("Submodule ID: " + submoduleID);
//        auto submoduleVector = module->getSubmoduleVectorNames();
//        log(submoduleVector.at(0));

        // Get eth[0], that is the control plane interface of the Open_Flow_Switch
        auto submodule = module->getSubmodule("eth", 0);
        if (submodule != nullptr) {
            auto submodulePath = submodule->getFullPath();
//            log("SubModule Path: " + submodulePath);

//            auto addressPar = submodule->("Address");

//            auto numpars = submodule->getNumParams();
//            for (int i = 0; i < numpars; i++) {
//                auto parname = submodule->par(i).getName();
//                std::string parstring(parname);
//                log("Par name: " + (parstring));
//            }

            // Get interface's MAC address. This is the same address we'll find in the datapath id for openflow messages
            std::string addressString = submodule->par("address").getValue().str(); // This adds quotation marks to the string! DFQ
            // Remove trailing and leading "
            addressString = addressString.substr(1, addressString.length()-2); // This slicing is inclusive for some reason
            log("Address string: " + addressString);

            MacAddress moduleAddress;
            try {
                auto cstr = addressString.c_str();
                moduleAddress = MacAddress(cstr);
                log("Good MAC address");
                log("MAC address: " + moduleAddress.str());
            } catch (std::exception& e) {
                log("Bad MAC address");
            }



        } else {
            log("No submodule found");
        }
    }
}

void FiDeController::readFlowtableConfiguration() {

    using namespace xmlutils;

    match = oxm_basic_match();

    cXMLElementList entryElements = configuration->getChildrenByTagName("entry");

    for (auto& entryElement : entryElements) {
        const char* action_outputAttr = entryElement->getAttribute("action_output"); // I don't like mixing cases like this but trying to follow multiple standards led to this...
        const char* idleTimeoutAttr = entryElement->getAttribute("idleTimeout");
        const char* hardTimeoutAttr = entryElement->getAttribute("hardTimeout");

        try {
            outport = static_cast<uint32_t>(std::stoul(action_outputAttr));
            idleTimeout = static_cast<int>(std::stoul(idleTimeoutAttr));
            hardTimeout = static_cast<int>(std::stoul(hardTimeoutAttr));
        } catch (std::exception& e) {
            throw cRuntimeError("Error in XML <entry> element at %s: %s", entryElement->getSourceLocation(), e.what());
        }

        const char* dscpAttr = entryElement->getAttribute("dscp");

        if (dscpAttr == nullptr) {
            dscp = 0;
        } else {
            try {
                dscp = static_cast<int>(std::stoul(dscpAttr));
            } catch (std::exception& e) {
                // Use default value
                dscp = 0;
            }
        }

//        cXMLElementList matchElements = configuration->getChildrenByTagName("entry/match");
//
//        for (auto& matchElement : matchElements) {
//            local_match = oxm_basic_match();

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

            int* in_port = (int*)malloc(sizeof(int));

            MacAddress* eth_dst;
            MacAddress* eth_src;
            int* eth_type = (int*)malloc(sizeof(int));

            Ipv4Address* ipv4_dst;

            int* arp_op = (int*)malloc(sizeof(int));
            Ipv4Address* arp_spa;
            Ipv4Address* arp_tpa;
            MacAddress* arp_sha;
            MacAddress* arp_tha;

            uint32_t* wildcards = (uint32_t*)malloc(sizeof(uint32_t));;

            try {
                *in_port = static_cast<int>(std::stoul(in_portAttr));
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> in_port element at %s: %s", entryElement->getSourceLocation(), e.what());
                in_port = nullptr;
            }

            MacAddress o_eth_dst;
            try {
                o_eth_dst = MacAddress(eth_dstAttr);
                eth_dst = &o_eth_dst;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> eth_dst element at %s: %s", entryElement->getSourceLocation(), e.what());
                eth_dst = nullptr;
            }

            MacAddress o_eth_src;
            try {
                o_eth_src = MacAddress(eth_srcAttr);
                eth_src = &o_eth_src;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> eth_src element at %s: %s", entryElement->getSourceLocation(), e.what());
                eth_src = nullptr;
            }
            try {
                *eth_type = static_cast<int>(std::stoul(eth_typeAttr));
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> eth_type element at %s: %s", entryElement->getSourceLocation(), e.what());
                eth_type = nullptr;
            }

            Ipv4Address o_ipv4_dst;
            try {
                o_ipv4_dst = Ipv4Address(ipv4_dstAttr);
                ipv4_dst = &o_ipv4_dst;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> ipv4_dst element at %s: %s", entryElement->getSourceLocation(), e.what());
                ipv4_dst = nullptr;
            }

            try {
                *arp_op = static_cast<int>(std::stoul(arp_opAttr));
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> arp_op element at %s: %s", entryElement->getSourceLocation(), e.what());
                arp_op = nullptr;
            }

            Ipv4Address o_arp_spa;
            try {
                o_arp_spa = Ipv4Address(arp_spaAttr);
                arp_spa = &o_arp_spa;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> arp_spa element at %s: %s", entryElement->getSourceLocation(), e.what());
                arp_spa = nullptr;
            }

            Ipv4Address o_arp_tpa;
            try {
                o_arp_tpa = Ipv4Address(arp_tpaAttr);
                arp_tpa = &o_arp_tpa;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> arp_tpa element at %s: %s", entryElement->getSourceLocation(), e.what());
                arp_tpa = nullptr;
            }

            MacAddress o_arp_sha;
            try {
                o_arp_sha = MacAddress(arp_shaAttr);
                arp_sha = &o_arp_sha;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> arp_sha element at %s: %s", entryElement->getSourceLocation(), e.what());
                arp_sha = nullptr;
            }

            MacAddress o_arp_tha;
            try {
                o_arp_tha = MacAddress(arp_thaAttr);
                arp_tha = &o_arp_tha;
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> arp_tha element at %s: %s", entryElement->getSourceLocation(), e.what());
                arp_tha = nullptr;
            }

            try {
                *wildcards = static_cast<uint32_t>(std::stoul(wildcardsAttr));
            } catch (std::exception& e) {
//                throw cRuntimeError("Error in XML <entry> wildcards element at %s: %s", entryElement->getSourceLocation(), e.what());
                wildcards = nullptr;
            }

            if (in_port != nullptr) {
                match.OFB_IN_PORT = *in_port;
            }

            if (eth_dst != nullptr) {
                match.OFB_ETH_DST = *eth_dst;
            }
            if (eth_src != nullptr) {
                match.OFB_ETH_SRC = *eth_src;
            }
            if (eth_type != nullptr) {
                match.OFB_ETH_TYPE = *eth_type;
            }

            if (ipv4_dst != nullptr) {
                match.OFB_IPV4_DST = *ipv4_dst;
            }

            if (arp_op != nullptr) {
                match.OFB_ARP_OP = *arp_op;
            }
            if (arp_spa != nullptr) {
                match.OFB_ARP_SPA = *arp_spa;
            }
            if (arp_tpa != nullptr) {
                match.OFB_ARP_TPA = *arp_tpa;
            }
            if (arp_sha != nullptr) {
                match.OFB_ARP_SHA = *arp_sha;
            }
            if (arp_tha != nullptr) {
                match.OFB_ARP_THA = *arp_tha;
            }

            if (wildcards != nullptr) {
                match.wildcards = *wildcards;
            }
//        }
    }
}

void FiDeController::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    EV << "FiDeController::receiveSignal" << '\n';

    AbstractControllerApp::receiveSignal(src,id,obj,details);
    Enter_Method("FiDeController::receiveSignal %s", cComponent::getSignalName(id));
    if(id == PacketFeatureReplySignalId){
        EV << "FiDeController::FeatureReply" << '\n';
        auto pkt = dynamic_cast<Packet *>(obj);
        if (pkt != nullptr) {
            auto chunk = pkt->peekAtFront<Chunk>();
            auto packet_in_msg = dynamicPtrCast<const OFP_Features_Reply>(chunk);
            if (packet_in_msg != nullptr) {

                std::string datapath_id(packet_in_msg->getDatapath_id()); // The interface's MAC Address!!! Maybe useful
                log("Datapath: " + datapath_id);
                MacAddress datapathMAC;
                try {
                    datapathMAC = MacAddress(datapath_id.c_str());
                    log("Datapath MAC address: " + datapathMAC.str());
                } catch (std::exception& e) {
                    log("Bad MAC address");
                }


                sendFlowTables(pkt);
            }
        }
    }
//    if(id == PacketInSignalId){
//        EV << "FiDeController::PacketIn" << '\n';
//        auto pkt = dynamic_cast<Packet *>(obj);
//        if (pkt != nullptr) {
//            auto chunk = pkt->peekAtFront<Chunk>();
//            auto packet_in_msg = dynamicPtrCast<const OFP_Packet_In>(chunk);
//            if (packet_in_msg != nullptr)
//                dropPacket(pkt);
//        }
//    }
}

void FiDeController::sendFlowTables(Packet* pkt){

    auto socket = controller->findSocketFor(pkt);

    sendFlowModMessage(OFPFC_ADD, match, outport, socket, idleTimeout, hardTimeout, dscp);
}

std::vector<int> getPortsByMac(MacAddress mac) {
    std::vector<int> res;
    return res;
}

void FiDeController::log(std::string msg, bool warn) {
    if (warn) {
        EV_WARN << "FiDeController " << msg << "\n";
    } else {
        EV << "FiDeController " << msg << "\n";
    }
}
