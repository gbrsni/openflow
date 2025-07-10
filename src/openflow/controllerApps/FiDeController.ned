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

package openflow.controllerApps;

simple FiDeController like AbstractControllerApp
{
    parameters:
        @display("i=block/app");
        
        object fideGroups;
        // Expects values formatted like this:
//        [
//        	{
//        	    sender: "Scenario_DynamicFatTree.fat_tree.client[1]",
//        	    receivers: "Scenario_DynamicFatTree.fat_tree.client[2],Scenario_DynamicFatTree.fat_tree.client[3],Scenario_DynamicFatTree.fat_tree.client[10]",
//        	    multicastGroup: "224.0.1.3",
//				messageLength: 1024,
//				sendInterval: 1
//        	},
//        	{
//        	    sender: "Scenario_DynamicFatTree.fat_tree.client[0]",
//        	    receivers: "Scenario_DynamicFatTree.fat_tree.client[4],Scenario_DynamicFatTree.fat_tree.client[5],Scenario_DynamicFatTree.fat_tree.client[15]",
//        	    multicastGroup: "224.0.1.4",
//				messageLength: 1024,
//				sendInterval: 1
//        	},
//        ]
		// Array of objects, each representing a FiDe group
		// messageLength is in bits, sendInterval in seconds
}
