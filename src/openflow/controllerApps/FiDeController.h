
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
    void sendFlowTables(Packet* packet_in_msg, std::vector<uint32_t> outports);

    cXMLElement *configuration = nullptr;
    bool flowConfigRead = false;
    int idleTimeout;
    int hardTimeout;
    oxm_basic_match match;
    void readFlowtableConfiguration();

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
    void log(std::string msg, bool warn=false, bool debug=false);
    std::string getModuleMameByMac(MacAddress mac);
    std::vector<int> getPortsByMac(MacAddress mac);

    std::vector<TrafficEngineering::Link> links;
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
