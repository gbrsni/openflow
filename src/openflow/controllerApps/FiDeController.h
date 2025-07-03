
#ifndef OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_
#define OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_

#include <omnetpp.h>
#include "openflow/controllerApps/AbstractControllerApp.h"
#include "openflow/openflow/switch/Flow_Table.h"

#include <Link.h>

class FiDeController: public AbstractControllerApp {
public:
    FiDeController();
    ~FiDeController();

protected:
    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;
    void initialize(int stage) override;
    void sendFlowTables(Packet* packet_in_msg, std::vector<uint32_t> outports, Ipv4Address multicastGroup);

    cXMLElement *configuration = nullptr;
    int idleTimeout = -1;
    int hardTimeout = -1;

    int dscp;

    // Tree stuff
    struct NodeInfo {
        NodeInfo() {isInTree=false;isProcessed=false;}
        bool isInTree;
        bool isProcessed;
        int moduleID;
        std::vector<int> ports;
        std::vector<int> treeNeighbors;
    };

    typedef std::vector<NodeInfo> NodeInfoVector;
    cTopology topo_spanntree;
    NodeInfoVector nodeInfo;

private:
    std::vector<std::string> parseReceivers(std::string str);
    // For the log function
    enum loglevel {
        TRACE,
        DEBUG,
//        DETAIL,
        INFO,
        WARN,
//        ERROR,
//        FATAL,
    };

    void log(std::string msg, int loglevel=INFO);
    std::string getModuleMameByMac(MacAddress mac);
    std::vector<uint32_t> getPortsByMacAndGroup(MacAddress mac, Ipv4Address multicastGroup);

    // TODO: Make this a map of multicast group to vector of links?
//    std::vector<TrafficEngineering::Link> links;
    std::map<Ipv4Address, std::vector<TrafficEngineering::Link>> links;
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
