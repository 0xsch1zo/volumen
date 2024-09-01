#include "api.hpp"
#include "utils.hpp"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <iostream>
#include <memory>
#include <cassert>

const std::string api::LIBRUS_API_URL       = "https://api.librus.pl";
const std::string api::EVENT_ENDPOINT       = "/3.0/SchoolNotices";
const std::string api::TIMETABLE_ENDPOINT   = "/3.0/Timetables";
authorization::synergia_account_t api::synergia_account;
std::list<std::string> api::auth_header;

api::api(authorization::synergia_account_t& account) {
    assert(account.access_token.empty() || account.student_name.empty());

    synergia_account.student_name = account.student_name;
    synergia_account.access_token = account.access_token;
    auth_header.push_back("Authorization: Bearer " + synergia_account.access_token);
}

std::shared_ptr<std::vector<api::event_t>> api::get_events() {
    std::ostringstream os;
    cl::Easy request;
    request.setOpt<cl::options::WriteStream>(&os);
    request.setOpt<cl::options::Url>(LIBRUS_API_URL + EVENT_ENDPOINT);
    request.setOpt<cl::options::Verbose>(false);
    request.setOpt<cl::options::HttpHeader>(auth_header);
    request.perform();
    json data = json::parse(os.str());
    std::shared_ptr<std::vector<api::event_t>> events = std::make_shared<std::vector<api::event_t>>();

    //spd::info(os.str());
    for(const auto& event : data["SchoolNotices"]){
        events->push_back({
            .start_date     = event["StartDate"],
            .end_date       = event["EndDate"],
            .subject        = event["Subject"],
            .content        = event["Content"],
            .author         = *fetch_id(event["AddedBy"]["Url"], request)
        });
    }
    request.reset();
    return events;
}

std::shared_ptr<std::string> api::fetch_id(const std::string& url_id, cl::Easy& request) {
    // Handle reuse
    std::ostringstream os;
    request.setOpt<cl::options::WriteStream>(&os);
    request.setOpt<cl::options::Url>(url_id);
    request.setOpt<cl::options::HttpHeader>(auth_header);
    request.perform();
    json data = json::parse(os.str());
    return std::make_shared<std::string>((std::string)data["User"]["FirstName"] + " " + (std::string)data["User"]["LastName"]);
}

std::shared_ptr<std::shared_ptr<std::vector<api::lesson_t>>[]>
// std::array<std::shared_ptr<std::vector<api::lesson_t>>, 7>
api::get_timetable(std::string week_start){
    std::ostringstream os;
    cl::Easy request;
    const int DAY_NUM = 7;
    request.setOpt<cl::options::WriteStream>(&os);
    request.setOpt<cl::options::Url>(LIBRUS_API_URL + TIMETABLE_ENDPOINT + "?weekStart=" + week_start);
    request.setOpt<cl::options::Verbose>(false);
    request.setOpt<cl::options::HttpHeader>(auth_header);
    request.perform();
    json data = json::parse(os.str());

    std::shared_ptr<std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]> 
    timetable(new std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]);

    for(int i{}; i< DAY_NUM; i++) {
        timetable[i] = std::make_shared<std::vector<api::lesson_t>>();
    }

    int i{};
    for(const auto& day : data["Timetable"].items()) {
        std::string date = day.key();

        for(const auto& lesson : day.value()) {
            if(lesson.empty()) {
                timetable[i]->push_back({
                    .subject            = "",
                    .teacher            = "",
                    .start              = "",
                    .end                = "",
                    .date               = "",
                    .substitution_note  = "",
                    .is_substitution    = false,
                    .is_canceled        = false,
                    .is_empty           = true
                }); 
                continue;
            }

            timetable[i]->push_back({
                .subject            = lesson[0]["Subject"]["Name"],
                .teacher            = (std::string)lesson[0]["Teacher"]["FirstName"] + (std::string)lesson[0]["Teacher"]["LastName"],
                .start              = lesson[0]["HourFrom"],
                .end                = lesson[0]["HourTo"],
                .date               = date,
                .substitution_note  = lesson[0]["SubstitutionNote"].is_null() ? "" : lesson[0]["SubstitutionNote"],
                .is_substitution    = lesson[0]["IsSubstitutionClass"],
                .is_canceled        = lesson[0]["IsCanceled"],
                .is_empty           = false 
            });
        }
        i++;
    }

    return timetable;
}