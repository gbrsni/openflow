
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
    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;
    void initialize(int stage) override;
    void sendFlowTables(Packet* packet_in_msg);

    cXMLElement *configuration = nullptr;
    uint32_t outport;
    int idleTimeout;
    int hardTimeout;
    oxm_basic_match match;
    uint32_t debug;
    void readFlowtableConfiguration();
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
