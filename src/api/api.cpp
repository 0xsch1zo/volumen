#include <api/api.hpp>
#include <misc/utils.hpp>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <spdlog/spdlog.h>

api::api(const auth& auth, const std::string& picked_login) : auth_o(auth), login_(picked_login) {
    update_access_token();
}

void api::update_access_token() {
    api_session->SetBearer(auth_o.get_api_access_token(login_));
}

void api::check_if_target_contains(const char* FUNCTION, const json& data, const std::string& target_json_data_structure) {
    static const std::string target_does_not_exist_message = "Target json structure not found: \"";
    if(!data.contains(target_json_data_structure))
        throw error::volumen_exception(target_does_not_exist_message + target_json_data_structure + "\"", FUNCTION);
}

// Fetches from an api endpoint
std::string api::fetch(const std::string& endpoint) {
    std::lock_guard api_session_lock{api_session_mutex};
    api_session->SetUrl(LIBRUS_API_URL + endpoint);

    cpr::Response resp = api_session->Get();

    if(resp.status_code == auth::UNAUTHORIZED_ERR_CODE) {
        auth_o.refresh_api_tokens();
        update_access_token();
        resp = api_session->Get();
        if(resp.status_code >= 400)
            throw error::volumen_exception( 
                "Unexected error while accessing: " + endpoint + 
                "\n Status code:" + std::to_string(resp.status_code),
                __FUNCTION__
            );
        
    }

    return resp.text;
}

std::string api::fetch_url(const std::string& url) {
    std::lock_guard api_session_lock{api_session_mutex};
    api_session->SetUrl(url);

    cpr::Response resp = api_session->Get();

    if(resp.status_code == auth::UNAUTHORIZED_ERR_CODE) {
        auth_o.refresh_api_tokens();
        update_access_token();
        resp = api_session->Get();
        if(resp.status_code >= 400)
            throw error::volumen_exception( 
                "Unexected error while accessing: " + url +
                "\n Status code:" + std::to_string(resp.status_code),
                __FUNCTION__
            );
    }

    return resp.text;
}

void api::parse_generic_info_by_id(const std::string& response, const std::string& target, generic_info_id_map& generic_info_id_map_p) {
    json data = json::parse(response);

    check_if_target_contains(__FUNCTION__, data, target);

    for(const auto& element : data[target].items()) {
        generic_info_id_map_p.emplace(
            (int)element.value()["Id"],
            element.value()["Name"]
        );
    }
}

std::string api::fetch_username_by_message_user_id(const std::string& url_id) {
    // Handle reuse
    json data = json::parse(fetch_url(url_id));
    return (std::string)data["User"]["FirstName"] + " " + (std::string)data["User"]["LastName"];
}

api::annoucements_t api::get_annoucments() {
    annoucements_t annoucments;
    parse_annoucments(fetch(ANNOUCMENT_ENDPOINT), annoucments);
    return annoucments;
}

void api::parse_annoucments(const std::string& response, annoucements_t& annoucments) {
    const std::string target_data_structure = "SchoolNotices";
    json data = json::parse(response);

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    annoucments.reserve(data[target_data_structure].size());

    for(int i = data[target_data_structure].size() - 1; i >= 0; i--) {
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
    timetable_t timetable_o;
    
    // If url is empty  it will query for the current week
    if(week_start_url.empty())
        parse_timetable(fetch(TIMETABLE_ENDPOINT), timetable_o);
    else
        parse_timetable(fetch_url(week_start_url), timetable_o);

    return timetable_o;
}

void api::parse_timetable(const std::string& response, api::timetable_t& timetable_o) {
    const std::string target_data_structure = "Timetable";

    json data = json::parse(response);

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
                /*.teacher            = */(std::string)lesson[0]["Teacher"]["FirstName"] + " " + (std::string)lesson[0]["Teacher"]["LastName"],
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
    json data = json::parse(fetch(TODAY_ENDPOINT));
    return (std::string)data["Date"];
}

api::messages_t api::get_messages() {
    const int page_message_limit = 300;
    messages_t messages_o;
    int page{1};
    do {
        parse_messages(
            fetch(MESSAGE_ENDPOINT + 
            "?limit=" + std::to_string(page_message_limit) +
            "&page=" + std::to_string(page++)), 
            messages_o
        );
    } while(messages_o.size() == page_message_limit);

    return messages_o; 
}

void api::parse_messages(const std::string& response, api::messages_t& messages_o) {
    const std::string target_data_structure = "Messages";
    json data = json::parse(response);

    check_if_target_contains(__FUNCTION__, data, target_data_structure);

    for(int i{}; i < data[target_data_structure].size(); i++){
        const auto& message = data[target_data_structure].at(i);
        messages_o.emplace_front(
            // Subject and content need to be parsed again because these are double escaped
            /*.subject    = */json::parse((std::string)message["Subject"]),
            /*.content    = */json::parse((std::string)message["Body"]),
            /*.author     = */fetch_username_by_message_user_id(message["Sender"]["Url"]),
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

    // Populate
    parse_generic_info_by_id(fetch(SUBJECTS_ENDPOINT), target_data_structure, ids_and_subjects);

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

    
    switch(type) {
        case GRADE:
            parse_generic_info_by_id(fetch(GRADE_CATEGORIES_ENDPOINT), target_data_structure, ids_and_categories[type]);
            break;

        case EVENT:
            parse_generic_info_by_id(fetch(EVENT_CATEGORIES_ENDPOINT), target_data_structure, ids_and_categories[type]);
            break;
    }

    return ids_and_categories[type][id];
}

std::string api::get_username_by_id(const int& id) {
    static std::unordered_map<int, const std::string> ids_and_usernames;
    if(!ids_and_usernames.empty())
        return ids_and_usernames[id];

    parse_username_by_id(fetch(USERS_ENDPOINT), ids_and_usernames);
    return ids_and_usernames[id];
}

void api::parse_username_by_id(const std::string& response, api::generic_info_id_map& ids_and_usernames) {
    const std::string delimiter = " ";
    const std::string target_data_structure = "Users";

    json data = json::parse(response);

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

    parse_comment_by_id(fetch(GRADE_COMMENTS_ENDPOINT), ids_and_comments);
    return ids_and_comments[id];
}

void api::parse_comment_by_id(const std::string& response, generic_info_id_map& ids_and_comments) {
    const std::string target_data_structure = "Comments";

    json data = json::parse(response);

    check_if_target_contains(__FUNCTION__, data, target_data_structure);
    ids_and_comments.reserve(data[target_data_structure].size());

    for(const auto& comment : data[target_data_structure].items())
        ids_and_comments.emplace( comment.value()["Id"], comment.value()["Text"] );
}

api::grades_t api::get_grades() {
    grades_t grades_o;

    parse_grades(fetch(GRADES_ENDPOINT), grades_o);
    return grades_o;
}

void api::parse_grades(const std::string& response, grades_t& grades_o) {
    const std::string target_data_structure = "Grades";
    json data = json::parse(response);

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
    recent_grades_t grades_o;
	
    parse_recent_grades(fetch(GRADES_ENDPOINT), grades_o);
    return grades_o;
}

void api::parse_recent_grades(const std::string& response, recent_grades_t& grades_o) {
    const std::string target_data_structure = "Grades";
	const int MAX_VECTOR_SIZE = 4;
 
    json data = json::parse(response);

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
    events_t events_o;
	
    parse_events(fetch(EVENT_ENDPOINT), events_o);
    return events_o;
}

void api::parse_events(const std::string& response, api::events_t& events_o) {
    const std::string target_data_structure = "HomeWorks";

    json data = json::parse(response);

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