#pragma once
#include "authorization.hpp"
#include <string>
#include <curlpp/Easy.hpp>

//namespace spd = spdlog;
namespace cl = cURLpp;
using json = nlohmann::json;

class api {
    enum category_types {
        GRADE,
        EVENT
    };
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
    authorization::synergia_account_t synergia_account; // Gets set once during init than stays the same
    std::list<std::string> auth_header;
    std::string get_subject_by_id(const int& id);
    std::string get_category_by_id(const int& id, category_types);
    std::string get_comment_by_id(const int& id);
    std::shared_ptr<std::string> get_username_by_id(const int& id);
    std::shared_ptr<std::string> fetch_username_by_message_user_id(const std::string& url_id, cl::Easy& request);
    std::unordered_map<int, const std::string>* get_subjects();

public:
    static const int RECENT_GRADES_SIZE             = 10;
public:
    // TODO: return const
    struct annoucment_t {
        std::string start_date;
        std::string end_date;
        std::string subject;
        std::string content;
        std::string author;
    };

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
        std::shared_ptr<std::shared_ptr<std::vector<lesson_t>>[]> timetable;
        std::string prev_url;
        std::string next_url;
    };

    struct message_t {
        std::string subject;
        std::string content;
        std::string sender;
        int send_date; // You might ask why did they use a date as a string in the lesson but now they are using unix timestamps. That my friend is a question that I have yet to answer.
    };

    struct messages_t {
        std::shared_ptr<std::vector<message_t>> messages;
    };

    struct grade_t {
        std::string subject;
        std::string grade;
        std::string category;
        std::string added_by;
        std::string date;
        std::string comment;
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

    struct event_t {
        std::string description;
        std::string category;
        std::string date;
        std::string created_by;
        int lesson_offset;
    };

    typedef std::unordered_map<std::string, std::vector<api::event_t>> events_t;

    api(authorization::synergia_account_t& account);

    void request_setup(cl::Easy& request, std::ostringstream& stream, const std::string& url);

    std::shared_ptr<std::vector<annoucment_t>> get_annoucments();

    std::shared_ptr<timetable_t>
    get_timetable(std::string week_start = "");

    std::string get_today();

    std::shared_ptr<messages_t>
    get_messages();

    std::shared_ptr<grades_t> get_grades();

    std::shared_ptr<std::vector<grade_t>>
    get_recent_grades();

    std::shared_ptr<events_t> get_events();
};


