#include "nebula_hw_interfaces/nebula_hw_interfaces_seyond/seyond_cmd_sender_robin_w.hpp"
#include <cstdio>
#include <algorithm>
#include <iterator>

namespace nebula
{
namespace drivers
{
void SeyondCmdSenderRobinW::UpdateUnicastIpPort(
    const std::string& unicast_ip, const uint16_t& unicast_port,
    const std::string& local_ip, const uint16_t& local_port)
{
  if (unicast_ip == local_ip && unicast_port == local_port) {
    // No update is needed
    return;
  }

  // XXX: Currently, one port is used to receive data, status, and message
  std::string config = "{\"ip\":\"" + local_ip + "\",\"port_data\":" + std::to_string(local_port)
                       + ",\"port_message\":" + std::to_string(local_port)
                       + ",\"port_status\":" +  std::to_string(local_port) + "}";

  HttpPost("/v1/lidar/force_udp_ports_ip", config);
}


}  // namespace drivers
}  // namespace nebula
