
#ifndef OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_
#define OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_

#include <omnetpp.h>
#include "openflow/controllerApps/AbstractControllerApp.h"
#include "openflow/openflow/switch/Flow_Table.h"

class FlowTablePreloader: public AbstractControllerApp {
public:
    FlowTablePreloader();
    ~FlowTablePreloader();

protected:
    simsignal_t flowModOut;
    int flowModCounter = 0;

    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;
    void initialize(int stage) override;
    void sendFlowTables(Packet* packet_in_msg);

    cXMLElement *configuration = nullptr;
    bool flowConfigRead = false;
    uint32_t outport;
    int idleTimeout;
    int hardTimeout;
    oxm_basic_match match;
    void readFlowtableConfiguration();
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
