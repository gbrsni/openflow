
#ifndef OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_
#define OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_

#include <omnetpp.h>
#include "openflow/controllerApps/AbstractControllerApp.h"

class FlowTablePreloader: public AbstractControllerApp {
public:
    FlowTablePreloader();
    ~FlowTablePreloader();

protected:
    void initialize(int stage) override;
};

#endif /* OPENFLOW_CONTROLLERAPPS_FLOWTABLEPRELOADER_H_ */
