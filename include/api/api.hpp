#pragma once
#define NOMINMAX
#include <auth/auth.hpp>
#include <misc/error_handler.hpp>
#include <string>
#include <cpr/cpr.h>
#include <deque>

#ifdef VOLUMEN_TESTING
class benchmarks;
#endif

using json = nlohmann::json;

class api {
    enum category_types {
        GRADE,
        EVENT
    };
    static const int TIMETABLE_DAY_NUM = 7;
    const std::string LIBRUS_API_URL                = "https://api.librus.pl";
    const std::string ANNOUCMENT_ENDPOINT           = "/3.0/SchoolNotices";
    const std::string TIMETABLE_ENDPOINT            = "/3.0/Timetables";
    const std::string TODAY_ENDPOINT                = "/3.0/SystemData";
    const std::string MESSAGE_ENDPOINT              = "/3.0/Messages";
    const std::string SUBJECTS_ENDPOINT             = "/3.0/Subjects";
    const std::string GRADES_ENDPOINT               = "/3.0/Grades";
    const std::string GRADE_COMMENTS_ENDPOINT       = GRADES_ENDPOINT + "/Comments";
    const std::string GRADE_CATEGORIES_ENDPOINT     = GRADES_ENDPOINT +"/Categories";
    const std::string USERS_ENDPOINT                = "/3.0/Users";
    const std::string EVENT_ENDPOINT                = "/3.0/HomeWorks"; // Why for fucks...
    const std::string EVENT_CATEGORIES_ENDPOINT     = EVENT_ENDPOINT + "/Categories";
    const std::string MESSAGE_USERS_ENDPOINT        = MESSAGE_ENDPOINT + "/User";
    const std::string login_;
    auth& auth_o;

public:
    struct content_t {
        std::string subject;
        std::string content;
        std::string author;
    };

    struct annoucement_t : content_t {
        std::string start_date;
        std::string end_date;

        annoucement_t(const std::string& start, const std::string& end, 
                    const std::string& sub, const std::string& cont, const std::string& authr) 
        : start_date(start), end_date(end), content_t{sub, cont, authr} {}
    };

    typedef std::vector<annoucement_t> annoucements_t;

    struct lesson_t {
        std::string subject;
        std::string teacher;
        std::string start;
        std::string end;
        std::string date;
        std::string substitution_note;
        bool is_substitution;
        bool is_canceled;
        bool is_empty;
    };

    struct timetable_t {
        std::array<std::vector<lesson_t>, TIMETABLE_DAY_NUM> timetable;
        std::string prev_url;
        std::string next_url;
    };

    struct message_t : content_t {
        int send_date; // You might ask why did they use a date as a string in the lesson but now they are using unix timestamps. That my friend is a question that I have yet to answer.

        message_t(const std::string& sub, const std::string& cont, const std::string& authr,
                int send_d) 
        : send_date(send_d), content_t{sub, cont, authr} {}
    };

    struct messages_t {
        std::deque<message_t> recieved;
        std::deque<message_t> sent;
    };

    struct grade_t {
        std::string subject;
        std::string grade;
        std::string category;
        std::string added_by;
        std::string date;
        std::optional<std::string> comment;
        int semester;
        bool is_semester;
        bool is_semester_proposition;
        bool is_final;
        bool is_final_proposition;
    };

    struct subject_with_grades_t {
        std::vector<grade_t> grades;
        std::string subject;
    };

    typedef std::unordered_map<int, subject_with_grades_t> grades_t;
    
    typedef std::vector<grade_t> grades_unstructured_t;

    struct event_t {
        std::string description;
        std::string category;
        std::string date;
        std::string created_by;
        std::optional<std::string> subject;
        int lesson_offset;
    };

    typedef std::unordered_map<std::string, std::vector<event_t>> events_t;

    typedef std::vector<event_t> events_unstructured_t;

    typedef std::unordered_map<int, const std::string> generic_info_id_map;
private:
#ifdef VOLUMEN_TESTING
    friend class benchmarks;
#endif
    std::optional<std::string> get_subject_by_id(const int& id);
    std::string get_category_by_id(const int& id, category_types);
    std::string get_comment_by_id(const int& id);
    std::string get_username_by_id(const int& id);
    const std::unordered_map<std::string, std::string>* fetch_usernames_by_message_user_id(std::vector<std::string>&& ids);
    const std::unordered_map<int, const std::string>* get_subjects();
    static void check_if_target_contains(const char* FUNCTION, const json& data, const std::string& target_json_data_structure);

    std::string fetch(const std::string& endpoint);
    std::string fetch_url(const std::string& urli);

    void populate_messages(std::deque<message_t>& messages, bool sent);

    void parse_generic_info_by_id(const std::string& response, const std::string& target, generic_info_id_map& generic_info_id_map_p);
    void parse_username_by_id(const std::string& response,generic_info_id_map& ids_and_usernames);
    void parse_comment_by_id(const std::string& response, generic_info_id_map& ids_and_comments);
    void parse_annoucments(const std::string& response, annoucements_t& annoucments_o);
    void parse_timetable(const std::string& response, timetable_t& timetable_o);
    void parse_messages(const std::string& response, std::deque<api::message_t>& messages_o);
    void parse_grades(const std::string& response, grades_t& grades_o);
    void parse_grades_unstructured(const std::string& response, grades_unstructured_t& grades_o);
    void parse_events(const std::string& response, api::events_t& events_o);
    void parse_events_unstructured(const std::string& response, api::events_unstructured_t& events_o);
public:


    api(auth& auth_o, const std::string& picked_login);

    annoucements_t get_annoucments();

    timetable_t get_timetable(std::string week_start_url = "");

    std::string get_today();

    messages_t get_messages();

    grades_t get_grades();

    grades_unstructured_t get_grades_unstructured();

    events_t get_events();

    events_unstructured_t get_events_unstructured();
};
