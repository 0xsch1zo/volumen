#include "api.hpp"
#include "utils.hpp"
#include <unordered_map>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <iostream>
#include <memory>
#include <cassert>

api::api(authorization::synergia_account_t& account) {
    assert(!account.access_token.empty() && !account.student_name.empty());

    synergia_account.student_name = account.student_name;
    synergia_account.access_token = account.access_token;
    auth_header.push_back("Authorization: Bearer " + synergia_account.access_token);
}

// Sets up common options for the request
void api::request_setup(cl::Easy& request, std::ostringstream& stream, const std::string& url) {
    request.setOpt<cl::options::WriteStream>(&stream);
    request.setOpt<cl::options::Url>(url);
    request.setOpt<cl::options::Verbose>(false);
    request.setOpt<cl::options::HttpHeader>(auth_header);
}

void api::check_if_target_contains(const char* FUNCTION, const json& data, const std::string& target_json_data_structure) {
    static const std::string target_does_not_exist_message = "Target json structure not found: \"";
    if(!data.contains(target_json_data_structure))
        throw error::volumen_exception(target_does_not_exist_message + target_json_data_structure + "\"", FUNCTION);
}

std::shared_ptr<std::vector<api::annoucment_t>> api::get_annoucments() {
    std::ostringstream os;
    cl::Easy request;
    const std::string target_data_structure = "SchoolNotices";
    request_setup(request, os, LIBRUS_API_URL + ANNOUCMENT_ENDPOINT);
    request.perform();
    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    std::shared_ptr<std::vector<api::annoucment_t>> annoucments = std::make_shared<std::vector<annoucment_t>>();

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--){
        const auto& annoucment = data[target_data_structure].at(i);
        annoucments->push_back({
            .start_date     = annoucment["StartDate"],
            .end_date       = annoucment["EndDate"],
            .subject        = annoucment["Subject"],
            .content        = annoucment["Content"],
            .author         = *get_username_by_id(annoucment["AddedBy"]["Id"])
        });
    }

    return annoucments;
}

std::shared_ptr<std::string> api::fetch_username_by_message_user_id(const std::string& url_id, cl::Easy& request) {
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
    const std::string target_data_structure = "Timetable";

    // If url is empty  it will query for the current week
    if(next_or_prev_url.empty())
        request_setup(request, os, LIBRUS_API_URL + TIMETABLE_ENDPOINT);
    else
        request_setup(request, os, next_or_prev_url);

    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    std::shared_ptr timetable_struct_p = std::make_shared<timetable_t>();

    std::shared_ptr<std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]> 
    timetable(new std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]);

    for(int i{}; i< DAY_NUM; i++) {
        timetable[i] = std::make_shared<std::vector<api::lesson_t>>();
    }

    timetable_struct_p->prev_url = std::make_shared<std::string>(data["Pages"]["Prev"]);
    timetable_struct_p->next_url = std::make_shared<std::string>(data["Pages"]["Next"]);

    int i{};
    for(const auto& day : data[target_data_structure].items()) {
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

std::string api::get_today() {
    std::ostringstream os;
    cl::Easy request;

    request_setup(request, os, LIBRUS_API_URL + TODAY_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());
    return (std::string)data["Date"];
}

std::shared_ptr<api::messages_t>
api::get_messages() {
    std::ostringstream os;
    cl::Easy request;
    const std::string target_data_structure = "Messages";

    request_setup(request, os, LIBRUS_API_URL + MESSAGE_ENDPOINT);
    request.perform();
    json data = json::parse(os.str());

    messages_t msgs;
    msgs.messages = std::make_shared<std::vector<message_t>>();

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& message = data[target_data_structure].at(i);
        msgs.messages->push_back({
            // Subject and content need to be parsed again because these are double escaped
            .subject    = json::parse((std::string)message["Subject"]),
            .content    = json::parse((std::string)message["Body"]),
            .sender     = *fetch_username_by_message_user_id(message["Sender"]["Url"], request),
            .send_date  = message["SendDate"]
        });
    }

    return std::make_shared<messages_t>(msgs);
}

std::string api::get_subject_by_id(const int& id) {
    std::unordered_map<int, const std::string>* subject_map_p = get_subjects();
    return (*subject_map_p)[id];
}

std::unordered_map<int, const std::string>* api::get_subjects() {
    const std::string target_data_structure = "Subjects";
    static std::unordered_map<int, const std::string> ids_and_subjects;

    if(!ids_and_subjects.empty())
        return &ids_and_subjects;

    std::ostringstream os;
    cl::Easy request;

    request_setup(request, os, LIBRUS_API_URL + SUBJECTS_ENDPOINT + "");

    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(const auto& subject : data[target_data_structure].items()) {
        ids_and_subjects.insert({
            (int)subject.value()["Id"],
            subject.value()["Name"]
        });
    }
    return &ids_and_subjects;
}

std::string api::get_category_by_id(const int& id, api::category_types type) {
    const std::string target_data_structure = "Categories";
    typedef std::unordered_map<int, const std::string> id_cateogry_map;
    static std::vector<id_cateogry_map> ids_and_categories {
        id_cateogry_map(),
        id_cateogry_map()
    };

    if(!ids_and_categories[type].empty())
        return ids_and_categories[type][id];

    std::ostringstream os;
    cl::Easy request;
    
    switch(type) {
        case GRADE:
            request_setup(request, os, LIBRUS_API_URL + GRADE_CATEGORIES_ENDPOINT);
            break;

        case EVENT:
            request_setup(request, os, LIBRUS_API_URL + EVENT_CATEGORIES_ENDPOINT);
            break;
    }

    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(const auto& category : data[target_data_structure].items()) {
        ids_and_categories[type].insert({
            (int)category.value()["Id"],
            category.value()["Name"]
        });
    }

    return ids_and_categories[type][id];
}

std::shared_ptr<std::string> api::get_username_by_id(const int& id) {
    static std::unordered_map<int, const std::string> ids_and_usernames;
    if(!ids_and_usernames.empty())
        return std::make_shared<std::string>(ids_and_usernames[id]);

    std::ostringstream os;
    cl::Easy request;
    const std::string delimiter = " ";
    const std::string target_data_structure = "Users";

    request_setup(request, os, LIBRUS_API_URL + USERS_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(const auto& username : data[target_data_structure].items()) {
        ids_and_usernames.emplace(
            username.value()["Id"],
            (std::string)username.value()["FirstName"] + delimiter + (std::string)username.value()["LastName"]
        );
    }

    return std::make_shared<std::string>(ids_and_usernames[id]);
}

std::string api::get_comment_by_id(const int& id) {
    static std::unordered_map<int, const std::string> ids_and_comments;
    const std::string target_data_structure = "Comments";

    if(!ids_and_comments.empty())
        return ids_and_comments[id];

    std::ostringstream os;
    cl::Easy request;

    request_setup(request, os, LIBRUS_API_URL + GRADE_COMMENTS_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(const auto& comment : data[target_data_structure].items()) {
        ids_and_comments.insert({
            (int)comment.value()["Id"],
            (std::string)comment.value()["Text"]
        });
    }

    return ids_and_comments[id];
}

std::shared_ptr<api::grades_t> api::get_grades() {
    std::ostringstream os;
    cl::Easy request;
    const std::string target_data_structure = "Grades";

    request_setup(request, os, LIBRUS_API_URL + GRADES_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    // Init
    std::shared_ptr<grades_t> grades = std::make_shared<grades_t>();

    for(const auto subject : *get_subjects())
        grades->emplace(
			subject.first, 
			subject_with_grades_t { 
				.grades = std::vector<grade_t>(), 
				.subject = subject.second
			}
		);

    // Populate
    for(const auto& grade : data[target_data_structure].items()) {
        (*grades)[grade.value()["Subject"]["Id"]].grades.push_back({
            .subject                    = get_subject_by_id(grade.value()["Subject"]["Id"]),
            .grade                      = grade.value()["Grade"],
            .category                   = get_category_by_id(grade.value()["Category"]["Id"], GRADE),
            .added_by                   = *get_username_by_id(grade.value()["AddedBy"]["Id"]),
            .date                       = grade.value()["Date"],
            // Why would a grade have multiple comments
            .comment                    = grade.value().contains("Comments") ? get_comment_by_id(grade.value()["Comments"][0]["Id"]) : "N/A",
            .semester                   = grade.value()["Semester"],
            .is_semester                = grade.value()["IsSemester"],
            .is_semester_proposition    = grade.value()["IsSemesterProposition"],
            .is_final                   = grade.value()["IsFinal"],
            .is_final_proposition       = grade.value()["IsFinalProposition"]
        });
    }

    return grades;
}

std::shared_ptr<std::vector<api::grade_t>> 
api::get_recent_grades() {
    std::ostringstream os;
    cl::Easy request;
	const int MAX_VECTOR_SIZE = 4;
    const std::string target_data_structure = "Grades";
	
    request_setup(request, os, LIBRUS_API_URL + GRADES_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());

    auto grades = std::make_shared<std::vector<grade_t>>();

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& grade = data[target_data_structure].at(i);
        grades->push_back({
            .subject                    = get_subject_by_id(grade["Subject"]["Id"]),
            .grade                      = grade["Grade"],
            .category                   = get_category_by_id(grade["Category"]["Id"], GRADE),
            .added_by                   = *get_username_by_id(grade["AddedBy"]["Id"]),
            .date                       = grade["Date"],
            .comment                    = grade.contains("Comments") ? get_comment_by_id(grade["Comments"][0]["Id"]) : "N/A",
            .semester                   = grade["Semester"],
            .is_semester                = grade["IsSemester"],
            .is_semester_proposition    = grade["IsSemesterProposition"],
            .is_final                   = grade["IsFinal"],
            .is_final_proposition       = grade["IsFinalProposition"]
        });

        if(grades->size() >= MAX_VECTOR_SIZE)
            break;
    }

    return grades;
}

std::shared_ptr<api::events_t> api::get_events() {
    std::ostringstream os;
    cl::Easy request;
    const std::string target_data_structure = "HomeWorks";
	
    request_setup(request, os, LIBRUS_API_URL + EVENT_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());

    std::shared_ptr<events_t> events = std::make_shared<events_t>();

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& event = data[target_data_structure].at(i);
        const std::string date = event["Date"];

        events->try_emplace(date, std::vector<event_t>());
        events->at(date).push_back({
            .description = event["Content"],
            .category = get_category_by_id(event["Category"]["Id"], EVENT),
            .date = event["Date"],
            .created_by = *get_username_by_id(event["CreatedBy"]["Id"]),
            .lesson_offset = event["LessonNo"].is_null() ? 0 : std::stoi((std::string)event["LessonNo"])
        });
    }

    return events;
}