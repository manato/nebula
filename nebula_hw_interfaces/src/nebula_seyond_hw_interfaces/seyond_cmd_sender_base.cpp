#include "nebula_hw_interfaces/nebula_hw_interfaces_seyond/seyond_cmd_sender_base.hpp"
#include <cstdio>
#include <algorithm>
#include <iterator>

namespace nebula
{
namespace drivers
{
SeyondCmdSenderBase::SeyondCmdSenderBase()
    : command_io_ctx_(std::make_shared<boost::asio::io_context>(1)),
      command_http_driver_(std::make_unique<::drivers::tcp_driver::HttpClientDriver>(command_io_ctx_))
{
}

Status SeyondCmdSenderBase::InitSensor(const std::shared_ptr<const SeyondSensorConfiguration>& sensor_config)
{
  constexpr uint16_t kCommandDefaultPort = 8010;
  try {
    command_http_driver_->init_client(sensor_config->sensor_ip, kCommandDefaultPort);
  } catch (const std::exception & ex) {
    Status status = Status::HTTP_CONNECTION_ERROR;
    std::cerr << "SeyondCmdSenderBase::InitSensor: " << status
              << sensor_config->sensor_ip << ", " << kCommandDefaultPort << std::endl;
    return Status::HTTP_CONNECTION_ERROR;
  }

  std::string udp_info = GetSensorParameter("udp_ports_ip");
  // Parse only necessary fields from the response
  auto parse_csv_line = [](const std::string& csv_text) -> std::vector<std::string>
  {
    char delimiter = ',';
    std::vector<std::string> values;
    std::stringstream ss(csv_text);
    std::string w;
    while (std::getline(ss, w, delimiter)) {values.emplace_back(w);}
    return values;
  };

  std::vector<std::string> parse_result = parse_csv_line(udp_info);
  if (parse_result.size() < 4) {
    // udp_info should have contents like:
    // 8010,8010,8010,0.0.0.0,0.0.0.0,172.168.1.170 (6 comma separated values )
    return Status::HTTP_CONNECTION_ERROR;
  }
  uint16_t data_port = std::stoi(parse_result[0]);
  uint16_t status_port = std::stoi(parse_result[1]);
  uint16_t message_port = std::stoi(parse_result[2]);
  std::string destination_ip{parse_result[3]};

  if (IsBroadcast(destination_ip)) {
    // broadcast IP, do nothing
    //sensor_config->host_ip = "0.0.0.0";
  } else if (IsMulticast(destination_ip)) {
    // multicast IP
    AddToMulticastGroup(destination_ip, data_port, sensor_config->host_ip, sensor_config->data_port);
  } else {
    // unicast IP
    UpdateUnicastIpPort(destination_ip, data_port, sensor_config->host_ip, sensor_config->data_port);
  }

  return Status::OK;
}

void SeyondCmdSenderBase::Close()
{
  command_http_driver_->client()->close();
}

std::string SeyondCmdSenderBase::HttpGet(const std::string& command)
{
  auto ret = command_http_driver_->get(command);
  command_http_driver_->client()->close();
  return ret;
}

std::string SeyondCmdSenderBase::HttpPost(const std::string command, const std::string body)
{
  auto ret = command_http_driver_->post(command, body);
  command_http_driver_->client()->close();
  return ret;
}

std::string SeyondCmdSenderBase::GetSensorParameter(const std::string& key)
{
  // return HttpGet("/command/?get_" + key);
  auto rt = command_http_driver_->get("/command/?get_" + key);
  command_http_driver_->client()->close();
  return rt;
}

std::string SeyondCmdSenderBase::SetSensorParameter(const std::string& key, const std::string& value)
{
  return HttpGet("/command/?set_" + key + "=" + value);
}

bool SeyondCmdSenderBase::IsBroadcast(std::string ip)
{
  constexpr std::string_view kBroadCastPostfix = ".255";
  if (ip.size() < kBroadCastPostfix.size()) {
    return false;
  }
  return std::equal(std::rbegin(kBroadCastPostfix), std::rend(kBroadCastPostfix),
                    std::rbegin(ip));
}

bool SeyondCmdSenderBase::IsMulticast(std::string ip)
{
  uint16_t first_octet;
  if (std::sscanf(ip.c_str(), "%hu", &first_octet) != 1) {
    return false;
  }
  return (224 <= first_octet && first_octet <= 239);
}

void SeyondCmdSenderBase::AddToMulticastGroup(
    const std::string& multicast_ip, const uint16_t& multicast_port,
    const std::string& local_ip, const uint16_t& local_port)
{
  /// XXX: multicast access to point cloud data is not supported yet
  std::cout << "Attention!!!!!!!!! Currently, multicast access to point cloud data is not supported!"
            << " We shall change to unicast!" << std::endl;
  UpdateUnicastIpPort(multicast_ip, multicast_port, local_ip, local_port);
}

void SeyondCmdSenderBase::UpdateUnicastIpPort(
    const std::string& unicast_ip, const uint16_t& unicast_port,
    const std::string& local_ip, const uint16_t& local_port)
{
  if (unicast_ip == local_ip && unicast_port == local_port) {
    // No update is needed
    return;
  }

  // XXX: Currently, one port is used to receive data, status, and message
  std::string config = std::to_string(local_port) + "," + std::to_string(local_port) + ","
                       + std::to_string(local_port);
  SetSensorParameter("udp_ports_ip", config);
}


}  // namespace drivers
}  // namespace nebula
