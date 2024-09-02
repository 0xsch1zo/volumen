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

// Sets up common options for the request
void api::request_setup(cl::Easy& request, std::ostringstream& stream, const std::string& endpoint) {
    request.setOpt<cl::options::WriteStream>(&stream);
    request.setOpt<cl::options::Url>(endpoint);
    request.setOpt<cl::options::Verbose>(false);
    request.setOpt<cl::options::HttpHeader>(auth_header);
}

std::shared_ptr<std::vector<api::event_t>> api::get_events() {
    std::ostringstream os;
    cl::Easy request;
    request_setup(request, os, LIBRUS_API_URL + EVENT_ENDPOINT);
    request.perform();
    json data = json::parse(os.str());
    std::shared_ptr<std::vector<api::event_t>> events = std::make_shared<std::vector<api::event_t>>();

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

std::shared_ptr<api::timetable_t>
api::get_timetable(std::string next_or_prev_url){
    std::ostringstream os;
    cl::Easy request;
    const int DAY_NUM = 7;

    // If url is empty  it will query for the current week
    if(next_or_prev_url.empty())
        request_setup(request, os, LIBRUS_API_URL + TIMETABLE_ENDPOINT);
    else
        request_setup(request, os, next_or_prev_url);

    request.perform();

    json data = json::parse(os.str());

    std::shared_ptr timetable_struct_p = std::make_shared<timetable_t>();

    std::shared_ptr<std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]> 
    timetable(new std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]);

    for(int i{}; i< DAY_NUM; i++) {
        timetable[i] = std::make_shared<std::vector<api::lesson_t>>();
    }

    timetable_struct_p->prev_url = data["Pages"]["Prev"];
    timetable_struct_p->next_url = data["Pages"]["Next"];

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
    
    timetable_struct_p->timetable = timetable;

    return timetable_struct_p;
}