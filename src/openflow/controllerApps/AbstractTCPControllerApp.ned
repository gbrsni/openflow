//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package openflow.controllerApps;

import inet.applications.contract.IApp;

//
// AbstractTCPControllerApp is a base class for OpenFlow controller applications that need
// to communicate over TCP in addition to interacting with the OpenFlow controller.
//
// This module extends AbstractControllerApp (which provides the basic controller application
// functionality) and implements the INET IApp interface (which provides TCP socket functionality).
// It serves as a bridge between the OpenFlow controller architecture and the INET TCP/IP stack.
//
// Key features of AbstractTCPControllerApp:
//
// 1. TCP Socket Management: Provides functionality for creating and managing TCP sockets,
//    allowing controller applications to establish connections with external entities.
//
// 2. Message Queue Processing: Implements a message queue with configurable service time
//    to simulate processing delay for incoming messages. Messages are processed sequentially
//    with the specified service time between them.
//
// 3. Statistics Collection: Collects and records statistics about queue size, waiting time,
//    and packets processed per second, which can be used for performance analysis.
//
// This class is used as a base for controller applications that need to communicate with
// external systems, such as the HyperFlowAgent which connects to a HyperFlowSynchronizer
// to enable distributed controller functionality.
//
simple AbstractTCPControllerApp like IControllerApp, IApp
{
    parameters:
        @class(openflow::AbstractTCPControllerApp);
        @display("i=block/app");
        @signal[queueSize](type="unsigned long");
        @statistic[queueSize](title="QueueSize"; record=vector?,stats; interpolationmode=none);
        @signal[waitingTime](type="simtime_t");
        @statistic[waitingTime](title="WaitingTime"; record=vector?,stats?; interpolationmode=none);

        // Processing time for each message (used to simulate processing delay).
        // Messages are processed sequentially with this delay between them.
        double serviceTime @unit("s") = default(0s);

        // Priority of flow entries installed by this application (higher values are matched first)
        int priority = default(1);

    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
