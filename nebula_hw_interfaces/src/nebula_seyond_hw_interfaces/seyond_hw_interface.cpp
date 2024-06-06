#include "nebula_hw_interfaces/nebula_hw_interfaces_seyond/seyond_hw_interface.hpp"
#include "nebula_hw_interfaces/nebula_hw_interfaces_seyond/seyond_cmd_sender_robin_w.hpp"

#include <sstream>

namespace nebula
{
namespace drivers
{
SeyondHwInterface::SeyondHwInterface()
: cloud_io_context_{new ::drivers::common::IoContext(1)},
  cloud_udp_driver_{new ::drivers::udp_driver::UdpDriver(*cloud_io_context_)},
  sensor_configuration_(std::make_shared<const SeyondSensorConfiguration>()),
  cloud_packet_callback_(nullptr),
  command_sender_(std::make_unique<SeyondCmdSenderBase>())
#if 0
  ctx_(std::make_shared<boost::asio::io_context>(1)),
  drv_(std::make_unique<::drivers::tcp_driver::HttpClientDriver>(ctx_))
#endif
{
}

SeyondHwInterface::~SeyondHwInterface()
{
  if(command_sender_) {
    command_sender_->Close();
  }
}

void SeyondHwInterface::ReceiveSensorPacketCallback(std::vector<uint8_t> & buffer)
{
  cloud_packet_callback_(buffer);
}

Status SeyondHwInterface::SensorInterfaceStart()
{
  if (sensor_configuration_->sensor_model == SensorModel::SEYOND_FALCON_KINETIC) {
    command_sender_ = std::make_unique<SeyondCmdSenderBase>();
  } else if (sensor_configuration_->sensor_model == SensorModel::SEYOND_ROBIN_W) {
    command_sender_ = std::make_unique<SeyondCmdSenderRobinW>();
  } else {
    return Status::INVALID_SENSOR_MODEL;
  }
  if (!command_sender_) {
    return Status::ERROR_1;
  }


#if 0
  // ====================================================================
  // XXX: SIGSEGV happens here
  // --------------------------------------------------------------------
  drv_->init_client(sensor_configuration_->sensor_ip, 8010);

  auto ret_str = drv_->get("/command/?get_udp_ports_ip");
  drv_->client()->close();
  PrintDebug("!)!)!)!)!)!)!)!)!)!)!)): " + ret_str);
  // --------------------------------------------------------------------
  // Expected output: ""!)!)!)!)!)!)!)!)!)!)!)): 8010,8010,8010,0.0.0.0,0.0.0.0,172.168.1.170"
  // Actual: SIGSEGV
  // ====================================================================
#endif


  Status status = command_sender_->InitSensor(sensor_configuration_);
  if (status != Status::OK){
    std::stringstream ss;
    ss << "Failed to Init sensor: " << status;
    PrintError(ss.str());
    return status;
  }
  DisplayCommonVersion();

  try {
    std::cout << "Starting UDP server on: " << *sensor_configuration_ << std::endl;
    cloud_udp_driver_->init_receiver(
        sensor_configuration_->host_ip, sensor_configuration_->data_port);
    cloud_udp_driver_->receiver()->open();
    cloud_udp_driver_->receiver()->bind();
    cloud_udp_driver_->receiver()->asyncReceive(
        std::bind(&SeyondHwInterface::ReceiveSensorPacketCallback, this, std::placeholders::_1));
  } catch (const std::exception & ex) {
    Status status = Status::UDP_CONNECTION_ERROR;
    std::cerr << status << sensor_configuration_->sensor_ip << ", "
              << sensor_configuration_->data_port << std::endl;
    return status;
  }
  return Status::OK;
}

Status SeyondHwInterface::SensorInterfaceStop()
{
  return Status::ERROR_1;
}

Status SeyondHwInterface::SetSensorConfiguration(
  const std::shared_ptr<const SensorConfigurationBase>& sensor_configuration)
{
  if (!(sensor_configuration->sensor_model == SensorModel::SEYOND_FALCON_KINETIC ||
        sensor_configuration->sensor_model == SensorModel::SEYOND_ROBIN_W)) {
    return Status::INVALID_SENSOR_MODEL;
  }

  sensor_configuration_ = std::static_pointer_cast<const SeyondSensorConfiguration>(sensor_configuration);

  return Status::OK;
}

Status SeyondHwInterface::GetSensorConfiguration(
  const SensorConfigurationBase & sensor_configuration)
{
  std::stringstream ss;
  ss << sensor_configuration;
  PrintDebug(ss.str());
  return Status::ERROR_1;
}

Status SeyondHwInterface::GetCalibrationConfiguration(
  const CalibrationConfigurationBase & calibration_configuration)
{
  PrintDebug(calibration_configuration.calibration_file);
  return Status::ERROR_1;
}

Status SeyondHwInterface::RegisterScanCallback(
  std::function<void(std::vector<uint8_t> &)> scan_callback)
{
  cloud_packet_callback_ = std::move(scan_callback);
  return Status::OK;
}

// void SeyondHwInterface::SetTargetModel(nebula::drivers::SensorModel model)
// {
//   target_model_no = static_cast<int>(model);
// }

void SeyondHwInterface::PrintError(std::string error)
{
  if (parent_node_logger_) {
    RCLCPP_ERROR_STREAM((*parent_node_logger_), error);
  } else {
    std::cerr << error << std::endl;
  }
}

void SeyondHwInterface::PrintDebug(std::string debug)
{
  if (parent_node_logger_) {
    RCLCPP_DEBUG_STREAM((*parent_node_logger_), debug);
  } else {
    std::cout << debug << std::endl;
  }
}

void SeyondHwInterface::PrintInfo(std::string info)
{
if (parent_node_logger_) {
    RCLCPP_INFO_STREAM((*parent_node_logger_), info);
  } else {
    std::cout << info << std::endl;
  }
}

void SeyondHwInterface::SetLogger(std::shared_ptr<rclcpp::Logger> logger)
{
  parent_node_logger_ = logger;
}

void SeyondHwInterface::DisplayCommonVersion()
{
  std::stringstream info;
  info << "*************** Innovusion Lidar Version Info ***************" <<std::endl;
  info << "sw_version:" << command_sender_->GetSensorParameter("sw_version") << std::endl;
  info << "sdk_version:" << command_sender_->GetSensorParameter("sdk_version") << std::endl;
  info << "lidar_id:" << command_sender_->GetSensorParameter("lidar_id") << std::endl;
  info << "fw_version:" << command_sender_->GetSensorParameter("fw_version") << std::endl;
  info << "sn:" << command_sender_->GetSensorParameter("sn") << std::endl;
  info << "*************************************************************" << std::endl;
  PrintDebug(info.str());
}

}  // namespace drivers
}  // namespace nebula
