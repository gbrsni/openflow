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
        string sender;
        string receivers;
        xml config = default(xml("<config><entry action_output='4294967291' idleTimeout='-1' hardTimeout='-1' priority='0' in_port='' eth_dst='' eth_src='' eth_type='' ipv4_dst='' arp_op='' arp_spa='' arp_tpa='' arp_sha='' arp_tha='' wildcards='4194303'/></config>"));
}
