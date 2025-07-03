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

#include <omnetpp.h>

Define_Module(FiDeController);


FiDeController::FiDeController() {

}

FiDeController::~FiDeController() {
}

void FiDeController::initialize(int stage){
    AbstractControllerApp::initialize(stage);

    const std::vector<std::string> typenames = {"inet.node.inet.StandardHost", "openflow.openflow.switch.Open_Flow_Switch"};

    if (stage == INITSTAGE_APPLICATION_LAYER) {
        // Object parameters
        cValueArray* fideGroupArray = check_and_cast<cValueArray*>(par("fideGroups").objectValue());
        for (int i = 0; i < fideGroupArray->size(); ++i) {
            auto fideGroup = check_and_cast<cValueMap*>(fideGroupArray->get(i).objectValue()); // Don't you just love broken manual examples? :D Thankfully the INET source code is avaiable...

            std::string sender = (*fideGroup)["sender"].stringValue();
            std::vector<std::string> receivers = parseReceivers((*fideGroup)["receivers"].stringValue());
            Ipv4Address multicastGroup = Ipv4Address((*fideGroup)["multicastGroup"].stringValue());

            log("Current FiDe group:", DEBUG);
            log("Sender:", DEBUG);
            log(sender, DEBUG);
            log("Receivers:", DEBUG);
            for (auto i = receivers.begin(); i < receivers.end(); i++) {
                log(*i, DEBUG);
            }
            log("Multicast group:", DEBUG);
            log(multicastGroup.str(), DEBUG);

            // FiDe stuff
            // TE
            TrafficEngineering::Topology topology = TrafficEngineering::makeTopologyFromCurrentNetwork(typenames);
            log("TE Topology made");

            TrafficEngineering::MulticastRequest request;
            request.messageLength = 0; // TODO: Make these into parameters
            request.sendInterval = 0; //
            request.appOwnerName = sender;
            request.appReceiverNames = receivers;

            TrafficEngineering::Tunnel tunnel = TrafficEngineering::optimization(topology, tunnels, request);
            tunnels.push_back(tunnel);

            auto result = adjustment(tunnels);
            float minLatency = result[0].minDelay;
            float maxJitter = result[0].maxDelay - result[0].minDelay;
            log("Result", WARN);
            log("minLatency: " + std::to_string(minLatency), WARN);
            log("maxJitter: " + std::to_string(maxJitter) , WARN);

            log("Tunnel links: ", DEBUG);
            links.emplace(multicastGroup, tunnel.getAllLinks());
            for (auto i = links[multicastGroup].begin(); i < links[multicastGroup].end(); i++) {
                log("localNodeName: " + i->localNodeName, DEBUG);
                log("localNodeName: " + i->remoteNodeName, DEBUG);
                log("localInterfaceName: " + i->localInterfaceName, DEBUG);
                log("remoteInterfaceName: " + i->remoteInterfaceName, DEBUG);
            }
        }

        // Get Open_Flow_Switch MACs
        // What this does is getting the MAC address on the control plane for the Open_Flow_Switch modules.
        // This is useful later since that is the ID they use in OF packets they send to the controller
        topo_SwitchMACs.extractByNedTypeName(typenames);
        log("FiDeController cTopology found " + std::to_string(topo_SwitchMACs.getNumNodes()));

        nodeInfo.resize(topo_SwitchMACs.getNumNodes());
        for (int i = 0; i < topo_SwitchMACs.getNumNodes(); i++) {
            nodeInfo[i].moduleID = topo_SwitchMACs.getNode(i)->getModuleId();
            nodeInfo[i].treeNeighbors.resize(topo_SwitchMACs.getNumNodes(),0);

            auto module = topo_SwitchMACs.getNode(i)->getModule();
            auto modulePath = module->getFullPath();
            log("Module Path: " + modulePath, DEBUG);

            // Get eth[0], that is the control plane interface of the Open_Flow_Switch
            auto submodule = module->getSubmodule("eth", 0);
            if (submodule != nullptr) {
                auto submodulePath = submodule->getFullPath();
                log("SubModule Path: " + submodulePath, TRACE);

                // Get interface's MAC address. This is the same address we'll find in the datapath id for openflow messages
                std::string addressString = submodule->par("address").getValue().str(); // This adds quotation marks to the string! DFQ
                // Remove trailing and leading "
                addressString = addressString.substr(1, addressString.length()-2);
                log("Address string: " + addressString, DEBUG);

                MacAddress moduleAddress;
                try {
                    auto cstr = addressString.c_str();
                    moduleAddress = MacAddress(cstr);
                    log("Good MAC address", DEBUG);
                    log("MAC address: " + moduleAddress.str(), DEBUG);
                } catch (std::exception& e) {
                    log("Bad MAC address", DEBUG);
                }

            } else {
                log("No submodule found");
            }
        }
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

                for (auto i = links.begin(); i != links.end(); i++) {
                    Ipv4Address multicastGroup = i->first;
                    log("Current group: " + multicastGroup.str());

                    std::vector<uint32_t> ports = getPortsByMacAndGroup(datapathMAC, multicastGroup);

                    log("Ports:");
                    for (auto i = ports.begin(); i < ports.end(); i++) {
                        log("port: " + std::to_string(*i));
                    }

                    sendFlowTables(pkt, ports, multicastGroup);
                }
            }
        }
    }
}

void FiDeController::sendFlowTables(Packet* pkt, std::vector<uint32_t> outports, Ipv4Address multicastGroup){
    log("sendFlowTables", DEBUG);

    oxm_basic_match match = oxm_basic_match();
    match.OFB_IPV4_DST = multicastGroup;

    match.wildcards= 0;
    match.wildcards |= OFPFW_ALL;
    match.wildcards ^=  OFPFW_NW_DST_ALL; // Wildcard all but IPV4 destination

    auto socket = controller->findSocketFor(pkt);

    uint32_t outport = -1;

    sendFlowModMessage(OFPFC_ADD, match, outport, socket, idleTimeout, hardTimeout, -1, outports);
}

std::vector<std::string> FiDeController::parseReceivers(std::string str) {
    std::vector<std::string> res = {};
    std::stringstream ss(str);

    std::string tmp;

    char del = ',';

    while (getline(ss, tmp, del)) {
        res.push_back(tmp);
    }

    return res;
}

std::string FiDeController::getModuleMameByMac(MacAddress mac) {
    std::string res = "";

    for (int i = 0; i < topo_SwitchMACs.getNumNodes(); i++) {
        nodeInfo[i].moduleID = topo_SwitchMACs.getNode(i)->getModuleId();
        nodeInfo[i].treeNeighbors.resize(topo_SwitchMACs.getNumNodes(),0);

        auto module = topo_SwitchMACs.getNode(i)->getModule();
        auto modulePath = module->getFullPath();
        log("Module Path: " + modulePath, DEBUG);

        // Get eth[0], that is the control plane interface of the Open_Flow_Switch
        auto submodule = module->getSubmodule("eth", 0);
        if (submodule != nullptr) {
            auto submodulePath = submodule->getFullPath();
            log("SubModule Path: " + submodulePath, DEBUG);

            // Get interface's MAC address. This is the same address we'll find in the datapath id for openflow messages
            std::string addressString = submodule->par("address").getValue().str(); // This adds quotation marks to the string! DFQ
            // Remove trailing and leading "
            addressString = addressString.substr(1, addressString.length()-2);
            log("Address string: " + addressString, TRACE);

            MacAddress moduleAddress;
            try {
                auto cstr = addressString.c_str();
                moduleAddress = MacAddress(cstr);
                log("Good MAC address", DEBUG);
                log("MAC address: " + moduleAddress.str(), DEBUG);
            } catch (std::exception& e) {
                log("Bad MAC address", DEBUG);
            }

            if (mac == moduleAddress) {
                return modulePath;
            }


        } else {
            log("No eth[0] submodule found", DEBUG);
        }
    }

    log("Couldn't get module name by MAC!", WARN);

    return res;
}

std::vector<uint32_t> FiDeController::getPortsByMacAndGroup(MacAddress mac, Ipv4Address multicastGroup) {
    log("getPortsByMacAndGroup");
    std::vector<uint32_t> res;

    std::string modulePath = getModuleMameByMac(mac);
    log("Module path: " + modulePath);


    // TODO: Make this into a map instead of iterating every time
    log("Tunnel links: ");
    std::vector<std::string> interfaceNames;
    for (auto i = links[multicastGroup].begin(); i < links[multicastGroup].end(); i++) {
        log("localNodeName: " + i->localNodeName, DEBUG);
        log("remoteNodeName: " + i->remoteNodeName, DEBUG);
        log("localInterfaceName: " + i->localInterfaceName, DEBUG);
        log("remoteInterfaceName: " + i->remoteInterfaceName, DEBUG);
        EV << "\n";
        if (modulePath.compare(i->localNodeName) == 0) {
            log("Found a match", DEBUG);
            interfaceNames.push_back(i->localInterfaceName);
        }
    }

    log("FiDe interfaces:");
    for (auto i = interfaceNames.begin(); i < interfaceNames.end(); i++) {
        log("Interface: " + *i, DEBUG);
        std::string portIDString = i->substr(i->length()-1, 1);
        log("portIDString: " + portIDString, DEBUG);
        int portID = std::stoi(portIDString) + 101;  // Ports are numbered from 101
        log("portID: " + std::to_string(portID) + " for " + modulePath, INFO);
        res.push_back(portID);
    }


    return res;
}

void FiDeController::log(std::string msg, int loglevel) {
    if (loglevel == WARN) {
        EV_WARN << "FiDeController " << msg << "\n";
    } else if (loglevel == DEBUG) {
        EV_DEBUG << "FiDeController " << msg << "\n";
    } else if (loglevel == TRACE) {
        EV_TRACE << "FiDeController " << msg << "\n";
    } else if (loglevel == INFO) {
        EV << "FiDeController " << msg << "\n";
    }
}
