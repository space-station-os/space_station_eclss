#include "space_station_eclss/co2_scrubber.h"
#include <algorithm>

Co2Scrubber::Co2Scrubber()
    :Node("ars_system"),
    co2_level_(declare_parameter<double>("initial_co2_level",400.0)),
    increase_rate_(declare_parameter<double>("increase_rate",5.0)),
    critical_threshold_(declare_parameter<double>("critical_threhold",650.0)),
    bake_reduction_(declare_parameter<double>("bake_reduction",300.0)),
    scrubber_efficiency_(declare_parameter<double>("scrubber_efficiency",0.9)),
    temperature_(declare_parameter<double>("station_temperature",22.0)), //degree celcius
    humidity_(declare_parameter<double>("station_humidity",50.0)) //% 



{
    ars_data_pub_=this->create_publisher<space_station_eclss::msg::ARS>("/ars_system",10);

    
    timer_=this->create_wall_timer(1s,std::bind(&Co2Scrubber::simulate_ars,this));


    bakery_ = this->create_client<std_srvs::srv::Trigger>("/check_efficiency");

    RCLCPP_INFO(this->get_logger(),"AIR REVITALIZATION SYSTEM INTIALIZED");


}


void Co2Scrubber::simulate_ars()
{
    // Simulate CO2, temperature, and humidity
    co2_level_ += increase_rate_;
    temperature_ += ((rand() % 100) / 1000.0) - 0.05; 
    humidity_ += ((rand() % 100) / 1000.0) - 0.05; 

    // Trigger baking process if critical threshold is reached
    if (co2_level_ >= critical_threshold_)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 3000,  // 3 seconds
                              "Critical CO2 level reached: %.2f ppm. Immediate action required!", co2_level_);

        // Call the baking process
        bake_gas();
    }

    // Create and publish the ARS data message
    auto msg = space_station_eclss::msg::ARS();
    msg.co2 = co2_level_;

    // Temperature
    msg.temperature.temperature = temperature_;
    msg.temperature.header.stamp = this->now();  
    msg.temperature.header.frame_id = "ISS_Temp"; 
    msg.temperature.variance = 0.01;  

    // Humidity
    msg.humidity.relative_humidity = humidity_;
    msg.humidity.header.stamp = this->now(); 
    msg.humidity.header.frame_id = "ISS_Humidity"; 
    msg.humidity.variance = 0.01;  

    ars_data_pub_->publish(msg);

    // Log the data being published
    RCLCPP_INFO(this->get_logger(), "Co2: %.2f ppm \n Temperature: %.2f °C \n Humidity: %.2f",
                co2_level_, temperature_, humidity_);
}


void Co2Scrubber::bake_gas()
{
    if (!bakery_->wait_for_service(5s))
    {
        RCLCPP_ERROR(this->get_logger(), "Bake service not available. Make sure the server is running.");
        return;
    }

    RCLCPP_INFO(this->get_logger(), "Sending request to bake CO2.");

    auto request = std::make_shared<std_srvs::srv::Trigger::Request>();

    // Send the request asynchronously and handle the response in a callback
    auto result_future = bakery_->async_send_request(
        request,
        [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture future_response) {
            try
            {
                auto res = future_response.get();
                if (res->success)
                {
                    double previous_co2 = co2_level_;
                    co2_level_ *= 0.7;  // Simulate a 30% reduction in CO2
                    RCLCPP_INFO(this->get_logger(),
                                "CO2 reduced from %.2f ppm to %.2f ppm: %s",
                                previous_co2, co2_level_, res->message.c_str());
                }
                else
                {
                    RCLCPP_WARN(this->get_logger(), "Bake process failed: %s", res->message.c_str());
                }
            }
            catch (const std::exception &e)
            {
                RCLCPP_ERROR(this->get_logger(), "Exception while calling bake service: %s", e.what());
            }
        });
}





int main(int argc,char *argv[])
{
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<Co2Scrubber>());
    rclcpp::shutdown();
    return 0;
}