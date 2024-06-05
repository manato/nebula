#ifndef NEBULA_SEYOND_CMD_SENDER_BASE_H
#define NEBULA_SEYOND_CMD_SENDER_BASE_H
// Have to define macros to silence warnings about deprecated headers being used by
// boost/property_tree/ in some versions of boost.
// See: https://github.com/boostorg/property_tree/issues/51
#include <boost/version.hpp>
#if (BOOST_VERSION / 100 >= 1073 && BOOST_VERSION / 100 <= 1076)  // Boost 1.73 - 1.76
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#if (BOOST_VERSION / 100 == 1074)  // Boost 1.74
#define BOOST_ALLOW_DEPRECATED_HEADERS
#endif

#include "nebula_common/seyond/seyond_common.hpp"
#include "boost_tcp_driver/http_client_driver.hpp"

#include <memory>
#include <string>

namespace nebula
{
namespace drivers
{
/// @brief Base class to send configuration commands to Seyond LiDARs via HTTP
class SeyondCmdSenderBase
{
 protected:
  std::shared_ptr<boost::asio::io_context> command_io_ctx_;
  std::unique_ptr<::drivers::tcp_driver::HttpClientDriver> command_http_driver_;

  /// @bried Perform HTTP "get" operation
  /// @param command command string to be sent
  /// @return result string
  virtual std::string HttpGet(const std::string& command);

  /// @bried Perform HTTP "post" operation
  /// @param command command string to be sent
  /// @return result string
  virtual std::string HttpPost(const std::string command, const std::string body);

  /// @brief Check IP address ends with 255 to see broadcast one
  /// @param ip IP address to be checked
  /// @return return true if IP is broadcast
  bool IsBroadcast(std::string ip);

  /// @brief Check IP address starts with the value within 224--239 to see multicast one
  /// @param ip IP address to be checked
  /// @return return true if IP is multicast
  bool IsMulticast(std::string ip);

  /// @brief Add senseor to multicast group (Since multicasting is not supported so far, it will falls back to unicast)
  /// @param multicast_ip The IP address of multicast
  /// @param multicast_port The port of multicast
  /// @param local_ip The IP address of local
  /// @param local_port The port of local
  virtual void AddToMulticastGroup(const std::string& multicast_ip, const uint16_t& multicast_port,
                                   const std::string& local_ip, const uint16_t& local_port);

  /// @brief Update destination IP and port that pointcloud data to be sent
  /// @param unicast_ip Current IP that sensor sends data to
  /// @param unicast_port Current port that sensor sends data to
  /// @param local_ip local (Host) IP address to receive data
  /// @param local_port The local (Host) port to receive data
  virtual void UpdateUnicastIpPort(const std::string& unicast_ip, const uint16_t& unicast_port,
                                   const std::string& local_ip, const uint16_t& local_port);

 public:
  /// @brief Constructor
  SeyondCmdSenderBase();

  /// @brief Acquire sensor parameter via HTTP
  /// @param key parameter name to be get
  /// @return Acquisition result
  virtual std::string GetSensorParameter(const std::string& key);

  /// @brief Set sensor parameter via HTTP
  /// @param key parameter name to be set
  /// @param value parameter value to be set
  /// @return result from sensor
  virtual std::string SetSensorParameter(const std::string& key, const std::string& value);

  /// @brief Initialize HTTP driver to send commands to the sensors
  /// @param sensor_config structure to store sensor configuration
  /// @return Resulting status
  virtual Status InitSensor(const std::shared_ptr<const SeyondSensorConfiguration>& sensor_config);

  /// @brief Close HTTP driver
  virtual void Close();

};
}  // namespace drivers
}  // namespace nebula

#endif  // NEBULA_SEYOND_CMD_SENDER_BASE_H
