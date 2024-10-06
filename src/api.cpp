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

// Fetches from an api endpoint
void api::fetch(const std::string& endpoint, std::ostringstream& os) {
    cl::Easy request;
    spd::error(endpoint);
    request_setup(request, os, LIBRUS_API_URL + endpoint);
    request.perform();
}

void api::fetch_url(const std::string& url, std::ostringstream& os) {
    cl::Easy request;
    request_setup(request, os, url);
    request.perform();
}

void api::parse_generic_info_by_id(const std::ostringstream& os, const std::string& target, generic_info_id_map& generic_info_id_map_p) {
    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target);

    for(const auto& element : data[target].items()) {
        generic_info_id_map_p.emplace(
            (int)element.value()["Id"],
            element.value()["Name"]
        );
    }
}

std::string api::fetch_username_by_message_user_id(const std::string& url_id, cl::Easy& request) {
    // Handle reuse
    std::ostringstream os;
    request.setOpt<cl::options::WriteStream>(&os);
    request.setOpt<cl::options::Url>(url_id);
    request.setOpt<cl::options::HttpHeader>(auth_header);
    request.perform();
    json data = json::parse(os.str());
    return (std::string)data["User"]["FirstName"] + " " + (std::string)data["User"]["LastName"];
}

api::annoucements_t api::get_annoucments() {
    std::ostringstream os;
    annoucements_t annoucments;
    fetch(ANNOUCMENT_ENDPOINT, os);
    parse_annoucments(os, annoucments);
    return annoucments;
}

void api::parse_annoucments(const std::ostringstream& os, annoucements_t& annoucments) {
    const std::string target_data_structure = "SchoolNotices";
    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    annoucments.reserve(data[target_data_structure].size());

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--){
        const auto& annoucment = data[target_data_structure].at(i);
        annoucments.emplace_back(
            /*.start_date     = */annoucment["StartDate"],
            /*.end_date       = */annoucment["EndDate"],
            /*.subject        = */annoucment["Subject"],
            /*.content        = */annoucment["Content"],
            /*.author         = */get_username_by_id(annoucment["AddedBy"]["Id"])
        );
    }
}

api::timetable_t api::get_timetable(std::string week_start_url){
    std::ostringstream os;
    
    // If url is empty  it will query for the current week
    if(week_start_url.empty())
        fetch(TIMETABLE_ENDPOINT, os);
    else
        fetch_url(week_start_url, os);

    timetable_t timetable_o;

    parse_timetable(os, timetable_o);

    return timetable_o;
}

void api::parse_timetable(const std::ostringstream &os, api::timetable_t& timetable_o) {
    const std::string target_data_structure = "Timetable";

    json data = json::parse(os.str());

    timetable_o.prev_url = data["Pages"]["Prev"];
    timetable_o.next_url = data["Pages"]["Next"];

    int i{};
    for(const auto& day : data[target_data_structure].items()) {
        std::string date = day.key();

        timetable_o.timetable[i].reserve(data[target_data_structure].size());
        for(const auto& lesson : day.value()) {
            if(lesson.empty()) {
                timetable_o.timetable[i].emplace_back(
                    /*.subject            = */"",
                    /*.teacher            = */"",
                    /*.start              = */"",
                    /*.end                = */"",
                    /*.date               = */"",
                    /*.substitution_note  = */"",
                    /*.is_substitution    = */false,
                    /*.is_canceled        = */false,
                    /*.is_empty           = */true
                ); 
                continue;
            }

            timetable_o.timetable[i].emplace_back(
                /*.subject            = */lesson[0]["Subject"]["Name"],
                /*.teacher            = */(std::string)lesson[0]["Teacher"]["FirstName"] + (std::string)lesson[0]["Teacher"]["LastName"],
                /*.start              = */lesson[0]["HourFrom"],
                /*.end                = */lesson[0]["HourTo"],
                /*.date               = */date,
                /*.substitution_note  = */lesson[0]["SubstitutionNote"].is_null() ? "" : lesson[0]["SubstitutionNote"],
                /*.is_substitution    = */lesson[0]["IsSubstitutionClass"],
                /*.is_canceled        = */lesson[0]["IsCanceled"],
                /*.is_empty           = */false 
            );
        }
        i++;
    }
}

std::string api::get_today() {
    std::ostringstream os;
    cl::Easy request;

    request_setup(request, os, LIBRUS_API_URL + TODAY_ENDPOINT);
    request.perform();

    json data = json::parse(os.str());
    return (std::string)data["Date"];
}

api::messages_t api::get_messages() {
    std::ostringstream os;
    messages_t messages_o;
    messages_o;

    fetch(MESSAGE_ENDPOINT, os);
    parse_messages(os, messages_o);
    return messages_o; 
}

void api::parse_messages(const std::ostringstream& os, api::messages_t& messages_o) {
    cl::Easy get_user_request;
    const std::string target_data_structure = "Messages";
    json data = json::parse(os.str());


    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    messages_o.reserve(data[target_data_structure].size());

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& message = data[target_data_structure].at(i);
        messages_o.emplace_back(
            // Subject and content need to be parsed again because these are double escaped
            /*.subject    = */json::parse((std::string)message["Subject"]),
            /*.content    = */json::parse((std::string)message["Body"]),
            /*.sender     = */fetch_username_by_message_user_id(message["Sender"]["Url"], get_user_request),
            /*.send_date  = */message["SendDate"]
        );
    }
}

std::string api::get_subject_by_id(const int& id) {
    const std::unordered_map<int, const std::string>* subject_map_p = get_subjects();
    return subject_map_p->at(id);
}

const std::unordered_map<int, const std::string>* api::get_subjects() {
    static std::unordered_map<int, const std::string> ids_and_subjects;
    const std::string target_data_structure = "Subjects";
    if(!ids_and_subjects.empty())
        return &ids_and_subjects;

    std::ostringstream os;

    fetch(SUBJECTS_ENDPOINT, os);
    // Populate
    parse_generic_info_by_id(os, target_data_structure, ids_and_subjects);

    return &ids_and_subjects;
}

std::string api::get_category_by_id(const int& id, api::category_types type) {
    const std::string target_data_structure = "Categories";
    
    static std::vector<generic_info_id_map> ids_and_categories {
        generic_info_id_map(),
        generic_info_id_map()
    };

    if(!ids_and_categories[type].empty())
        return ids_and_categories[type][id];

    
    std::ostringstream os;
    
    switch(type) {
        case GRADE:
            fetch(GRADE_CATEGORIES_ENDPOINT, os);
            break;

        case EVENT:
            fetch(EVENT_CATEGORIES_ENDPOINT, os);
            break;
    }

    parse_generic_info_by_id(os, target_data_structure, ids_and_categories[type]);
    return ids_and_categories[type][id];
}

std::string api::get_username_by_id(const int& id) {
    static std::unordered_map<int, const std::string> ids_and_usernames;
    if(!ids_and_usernames.empty())
        return ids_and_usernames[id];

    std::ostringstream os;
    cl::Easy request;

    fetch(USERS_ENDPOINT, os);
    parse_username_by_id(os, ids_and_usernames);
    return ids_and_usernames[id];
}

void api::parse_username_by_id(const std::ostringstream& os, api::generic_info_id_map& ids_and_usernames) {
    const std::string delimiter = " ";
    const std::string target_data_structure = "Users";

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    ids_and_usernames.reserve(data[target_data_structure].size());

    for(const auto& username : data[target_data_structure].items()) {
        ids_and_usernames.emplace(
            username.value()["Id"],
            (std::string)username.value()["FirstName"] + delimiter + (std::string)username.value()["LastName"]
        );
    }
}

std::string api::get_comment_by_id(const int& id) {
    static std::unordered_map<int, const std::string> ids_and_comments;

    if(!ids_and_comments.empty())
        return ids_and_comments[id];

    std::ostringstream os;
    fetch(GRADE_COMMENTS_ENDPOINT, os);
    parse_comment_by_id(os, ids_and_comments);
    return ids_and_comments[id];
}

void api::parse_comment_by_id(const std::ostringstream& os, generic_info_id_map& ids_and_comments) {
    const std::string target_data_structure = "Comments";

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    ids_and_comments.reserve(data[target_data_structure].size());

    for(const auto& comment : data[target_data_structure].items())
        ids_and_comments.emplace( comment.value()["Id"], comment.value()["Text"] );
}

api::grades_t api::get_grades() {
    std::ostringstream os;
    grades_t grades_o;

    fetch(GRADES_ENDPOINT, os);
    parse_grades(os, grades_o);
    return grades_o;
}

void api::parse_grades(const std::ostringstream& os, grades_t& grades_o) {
    const std::string target_data_structure = "Grades";
    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    const auto& subjects = get_subjects();
    grades_o.reserve(data[target_data_structure].size());

    for(const auto subject : *subjects)
        grades_o.emplace(
			subject.first, 
			subject_with_grades_t { 
				.grades = std::vector<grade_t>(), 
				.subject = subject.second
			}
		);

    // Populate
    for(const auto& grade : data[target_data_structure].items()) {
        /*grades_o.try_emplace(
			grade.value()["Subject"]["Id"], 
			subject_with_grades_t { 
				.grades = std::vector<grade_t>(), 
				.subject = ""
			}
        );*/

        grades_o[grade.value()["Subject"]["Id"]].grades.emplace_back(
            /*.subject                    =*/ get_subject_by_id(grade.value()["Subject"]["Id"]),
            /*.grade                      =*/ grade.value()["Grade"],
            /*.category                   =*/ get_category_by_id(grade.value()["Category"]["Id"], GRADE),
            /*.added_by                   =*/ get_username_by_id(grade.value()["AddedBy"]["Id"]),
            /*.date                       =*/ grade.value()["Date"],
            // Why would a grade have multiple comments
            /*.comment                    =*/ grade.value().contains("Comments") ? get_comment_by_id(grade.value()["Comments"][0]["Id"]) : "N/A",
            /*.semester                   =*/ grade.value()["Semester"],
            /*.is_semester                =*/ grade.value()["IsSemester"],
            /*.is_semester_proposition    =*/ grade.value()["IsSemesterProposition"],
            /*.is_final                   =*/ grade.value()["IsFinal"],
            /*.is_final_proposition       =*/ grade.value()["IsFinalProposition"]
        );
    }
}

api::recent_grades_t api::get_recent_grades() {
    std::ostringstream os;
    recent_grades_t grades_o;
	
    fetch(GRADES_ENDPOINT, os);
    parse_recent_grades(os, grades_o);
    return grades_o;
}

void api::parse_recent_grades(const std::ostringstream& os, recent_grades_t& grades_o) {
    const std::string target_data_structure = "Grades";
	const int MAX_VECTOR_SIZE = 4;
 
    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    grades_o.reserve(data[target_data_structure].size());

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& grade = data[target_data_structure].at(i);
        grades_o.emplace_back(
            /*.subject                    =*/ get_subject_by_id(grade["Subject"]["Id"]),
            /*.grade                      =*/ grade["Grade"],
            /*.category                   =*/ get_category_by_id(grade["Category"]["Id"], GRADE),
            /*.added_by                   =*/ get_username_by_id(grade["AddedBy"]["Id"]),
            /*.date                       =*/ grade["Date"],
            /*.comment                    =*/ grade.contains("Comments") ? get_comment_by_id(grade["Comments"][0]["Id"]) : "N/A",
            /*.semester                   =*/ grade["Semester"],
            /*.is_semester                =*/ grade["IsSemester"],
            /*.is_semester_proposition    =*/ grade["IsSemesterProposition"],
            /*.is_final                   =*/ grade["IsFinal"],
            /*.is_final_proposition       =*/ grade["IsFinalProposition"]
        );

        if(grades_o.size() >= MAX_VECTOR_SIZE)
            break;
    }   
}

api::events_t api::get_events() {
    std::ostringstream os;
    events_t events_o;
	
    fetch(EVENT_ENDPOINT, os);
    parse_events(os, events_o);
    return events_o;
}

void api::parse_events(const std::ostringstream& os, api::events_t& events_o) {
    const std::string target_data_structure = "HomeWorks";

    json data = json::parse(os.str());

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    events_o.reserve(data[target_data_structure].size());

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
        const auto& event = data[target_data_structure].at(i);
        const std::string date = event["Date"];

        events_o.try_emplace(date, std::vector<event_t>());
        events_o.at(date).emplace_back(
            /*.description =    */    event["Content"],
            /*.category =       */    get_category_by_id(event["Category"]["Id"], EVENT),
            /*.date =           */    event["Date"],
            /*.created_by =     */    get_username_by_id(event["CreatedBy"]["Id"]),
            /*.lesson_offset =  */    event["LessonNo"].is_null() ? 0 : std::stoi((std::string)event["LessonNo"])
        );
    }
}