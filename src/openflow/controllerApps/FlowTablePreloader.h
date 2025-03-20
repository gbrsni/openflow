
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
    Flow_Table flowTable;
    void readFlowtableConfiguration(Flow_Table& flowTable);
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
