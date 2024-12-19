#include "space_station_eclss/co2_scrubber.h"

Co2Scrubber::Co2Scrubber()
    :Node("baking_process"),
    co2_level_(declare_parameter<double>("initial_co2_level",400.0)),
    increase_rate_(declare_parameter<double>("increase_rate",5.0)),
    critical_threshold_(declare_parameter<double>("critical_threhold",650.0)),
    bake_reduction_(declare_parameter<double>("bake_reduction",300.0)),
    scrubber_efficiency_(declare_parameter<double>("scrubber_efficiency",0.9)),
    temperature_(declare_parameter<double>("station_temperature",22.0)), //degree celcius
    humidity_(declare_parameter<double>("station_humidity",50.0)) //%

{
    efficiency_service_=this->create_service<std_srvs::srv::Trigger>("/check_efficiency",
                                                                std::bind(
                &Co2Scrubber::handle_zeolite_efficiency, this, std::placeholders::_1, std::placeholders::_2));

}

void Co2Scrubber::handle_zeolite_efficiency( const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    const std::shared_ptr<std_srvs::srv::Trigger::Response> res){

    bool scrubber_success=(rand()%10) < 8 ;


    if (req == nullptr)
    {
        RCLCPP_WARN(this->get_logger(), "Received an invalid request!");
        res->success = false;
        res->message = "Invalid request received.";
        return;
    }

    if (scrubber_success){

        double initial_co2=co2_level_;
        co2_level_*=0.7; //reducing the co2 to simulate the baking scenario


        res->success=true;
        res->message=
            "Baking CO2 successful. Initial CO2 level: " + std::to_string(initial_co2) +
            " ppm. Reduced to: " + std::to_string(co2_level_) +
            " ppm with efficiency: " + std::to_string(scrubber_efficiency_ * 100) + "%.";


        RCLCPP_INFO(this->get_logger(),"Bake request handed successfully, CO2 Reduced from %.2f ppm to %.2f ppm",
        initial_co2,co2_level_);

    }
    else{
        res->success=false;
        res->message="Baking failed due to malfunction.Please check system ASAP";


        RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 3000,  // 3 seconds
                              "Bake request failed due to scrubber malfunction. CO2 level CRITICAL: %.2f", co2_level_);
    }
}

int main(int argc,char *argv[])
{
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<Co2Scrubber>());
    rclcpp::shutdown();
    return 0;
}