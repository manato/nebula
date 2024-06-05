#ifndef NEBULA_SEYOND_CMD_SENDER_ROBIN_W_H
#define NEBULA_SEYOND_CMD_SENDER_ROBIN_W_H

#include "nebula_hw_interfaces/nebula_hw_interfaces_seyond/seyond_cmd_sender_base.hpp"

namespace nebula
{
namespace drivers
{
/// @brief Derived class to send configuration commands to Seyond Robin W via HTTP
class SeyondCmdSenderRobinW : public SeyondCmdSenderBase
{
 protected:
  /// @brief Update destination IP and port that pointcloud data to be sent
  /// @param unicast_ip Current IP that sensor sends data to
  /// @param unicast_port Current port that sensor sends data to
  /// @param local_ip local (Host) IP address to receive data
  /// @param local_port The local (Host) port to receive data
  virtual void UpdateUnicastIpPort(const std::string& unicast_ip, const uint16_t& unicast_port,
                                   const std::string& local_ip, const uint16_t& local_port) override;
};
}  // namespace drivers
}  // namespace nebula

#endif  // NEBULA_SEYOND_CMD_SENDER_ROBIN_W_H
