#include <api/api.hpp>
#include <cassert>
#include <format>
#include <misc/utils.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

api::api(auth &auth, const std::string &picked_login)
    : auth_o(auth), login_(picked_login) {}

void api::check_if_target_contains(
    const char *FUNCTION, const json &data,
    const std::string &target_json_data_structure) {
  static const std::string target_does_not_exist_message =
      "Target json structure not found: \"";
  if (!data.contains(target_json_data_structure))
    throw error::volumen_exception(target_does_not_exist_message +
                                       target_json_data_structure + "\"",
                                   FUNCTION, error::json_parsing_error);
}

// Fetches from an api endpoint
std::string api::fetch(const std::string &endpoint) {
  cpr::Response resp =
      cpr::Get(cpr::Url{LIBRUS_API_URL + endpoint},
               cpr::Bearer{auth_o.get_api_access_token(login_)});

  if (resp.status_code == auth::UNAUTHORIZED_ERR_CODE) {
    auth_o.refresh_api_tokens();
    resp = cpr::Get(cpr::Url{LIBRUS_API_URL + endpoint},
                    cpr::Bearer{auth_o.get_api_access_token(login_)});

    if (resp.status_code >= 400)
      throw error::volumen_exception(endpoint + ": " + resp.status_line,
                                     __FUNCTION__, error::request_failed);
  }

  return resp.text;
}

std::string api::fetch_url(const std::string &url) {
  cpr::Response resp =
      cpr::Get(cpr::Url{url}, cpr::Bearer{auth_o.get_api_access_token(login_)});

  if (resp.status_code == auth::UNAUTHORIZED_ERR_CODE) {
    auth_o.refresh_api_tokens();
    resp = cpr::Get(cpr::Url{url},
                    cpr::Bearer{auth_o.get_api_access_token(login_)});
    if (resp.status_code >= 400)
      throw error::volumen_exception(url + ": " + resp.status_line,
                                     __FUNCTION__, error::request_failed);
  }

  return resp.text;
}

void api::parse_generic_info_by_id(const std::string &response,
                                   const std::string &target,
                                   generic_info_id_map &generic_info_id_map_p) {
  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target);

  for (const auto &element : data[target].items()) {
    generic_info_id_map_p.emplace((int)element.value()["Id"],
                                  element.value()["Name"]);
  }
}

const std::unordered_map<std::string, std::string> *
api::fetch_usernames_by_message_user_id(std::vector<std::string> &&ids) {
  static std::unordered_map<std::string, std::string> users;
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  std::string not_cached;

  for (int i{}; i < ids.size(); i++) {
    const auto &id = ids.at(i);
    if (!users.contains(id)) {
      not_cached.append(i == 0 ? id : "," + id);
    }
  }

  if (!not_cached.empty()) {
    std::string response = fetch(MESSAGE_USERS_ENDPOINT + "/" + not_cached +
                                 "?limit=" + std::to_string(ids.size()));
    json data = json::parse(response);
    if (data.contains("User"))
      users[data["User"]["Id"]] = (std::string)data["User"]["FirstName"] + " " +
                                  (std::string)data["User"]["LastName"];
    else {
      check_if_target_contains(__FUNCTION__, data, "Users");
      for (const auto &user : data["Users"].items()) {
        std::string first_name = user.value()["FirstName"].is_null()
                                     ? "-"
                                     : user.value()["FirstName"];
        std::string last_name =
            user.value()["LastName"].is_null() ? "" : user.value()["LastName"];
        users[user.value()["Id"]] = std::format("{} {}", first_name, last_name);
      }
    }
  }

  return &users;
}

api::annoucements_t api::get_annoucments() {
  annoucements_t annoucments;
  parse_annoucments(fetch(ANNOUCMENT_ENDPOINT), annoucments);
  return annoucments;
}

void api::parse_annoucments(const std::string &response,
                            annoucements_t &annoucments) {
  const std::string target_data_structure = "SchoolNotices";
  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  annoucments.reserve(data[target_data_structure].size());

  for (int i = data[target_data_structure].size() - 1; i >= 0; i--) {
    const auto &annoucment = data[target_data_structure].at(i);
    annoucments.emplace_back(
        /*.start_date     = */ annoucment["StartDate"],
        /*.end_date       = */ annoucment["EndDate"],
        /*.subject        = */ annoucment["Subject"],
        /*.content        = */ annoucment["Content"],
        /*.author         = */ get_username_by_id(annoucment["AddedBy"]["Id"]));
  }
}

api::timetable_t api::get_timetable(std::string week_start_url) {
  timetable_t timetable_o;

  // If url is empty  it will query for the current week
  if (week_start_url.empty())
    parse_timetable(fetch(TIMETABLE_ENDPOINT), timetable_o);
  else
    parse_timetable(fetch_url(week_start_url), timetable_o);

  return timetable_o;
}

void api::parse_timetable(const std::string &response,
                          api::timetable_t &timetable_o) {
  const std::string target_data_structure = "Timetable";

  json data = json::parse(response);

  timetable_o.prev_url = data["Pages"]["Prev"];
  timetable_o.next_url = data["Pages"]["Next"];

  int i{};
  for (const auto &day : data[target_data_structure].items()) {
    std::string date = day.key();

    timetable_o.timetable[i].reserve(data[target_data_structure].size());
    for (const auto &lesson : day.value()) {
      if (lesson.empty()) {
        timetable_o.timetable[i].emplace_back(
            /*.subject            = */ "",
            /*.teacher            = */ "",
            /*.start              = */ "",
            /*.end                = */ "",
            /*.date               = */ "",
            /*.substitution_note  = */ "",
            /*.is_substitution    = */ false,
            /*.is_canceled        = */ false,
            /*.is_empty           = */ true);
        continue;
      }

      timetable_o.timetable[i].emplace_back(
          /*.subject            = */ lesson[0]["Subject"]["Name"],
          /*.teacher            = */
          (std::string)lesson[0]["Teacher"]["FirstName"] + " " +
              (std::string)lesson[0]["Teacher"]["LastName"],
          /*.start              = */ lesson[0]["HourFrom"],
          /*.end                = */ lesson[0]["HourTo"],
          /*.date               = */ date,
          /*.substitution_note  = */ lesson[0]["SubstitutionNote"].is_null()
              ? ""
              : lesson[0]["SubstitutionNote"],
          /*.is_substitution    = */ lesson[0]["IsSubstitutionClass"],
          /*.is_canceled        = */ lesson[0]["IsCanceled"],
          /*.is_empty           = */ false);
    }
    i++;
  }
}

std::string api::get_today() {
  json data = json::parse(fetch(TODAY_ENDPOINT));
  return (std::string)data["Date"];
}

api::messages_t api::get_messages() {
  messages_t messages_o;
  int page{1};
  populate_messages(messages_o.recieved, false);
  populate_messages(messages_o.sent, true);
  return messages_o;
}

void api::populate_messages(std::deque<message_t> &messages, bool sent) {
  const int page_message_limit = 300;
  int page{1};
  do {
    parse_messages(fetch(MESSAGE_ENDPOINT +
                         "?limit=" + std::to_string(page_message_limit) +
                         "&page=" + std::to_string(page++) +
                         (sent ? "&inSended=true" : "")),
                   messages);
  } while (messages.size() == page_message_limit);
}

void api::parse_messages(const std::string &response,
                         std::deque<api::message_t> &messages_o) {
  const std::string target_data_structure = "Messages";
  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);

  std::vector<std::string> user_ids;
  user_ids.reserve(data[target_data_structure].size());
  for (int i{}; i < data[target_data_structure].size(); i++) {
    const auto &message = data[target_data_structure].at(i);
    messages_o.emplace_front(
        // Subject and content need to be parsed again because these are double
        // escaped
        /*.subject    = */ json::parse((std::string)message["Subject"]),
        /*.content    = */ json::parse((std::string)message["Body"]),
        /*.author     = */ message["Sender"]["Id"],
        /*.send_date  = */ message["SendDate"]);

    user_ids.emplace_back(message["Sender"]["Id"]);
  }

  auto user_id_map = fetch_usernames_by_message_user_id(std::move(user_ids));
  for (auto &message : messages_o) {
    message.author = user_id_map->at(message.author);
  }
}

std::optional<std::string> api::get_subject_by_id(const int &id) {
  const std::unordered_map<int, const std::string> *subject_map_p =
      get_subjects();
  // It turns out that the librus api can give wrong ids for the subject.
  return subject_map_p->contains(id)
             ? std::optional<std::string>(subject_map_p->at(id))
             : std::nullopt;
}

const std::unordered_map<int, const std::string> *api::get_subjects() {
  static std::unordered_map<int, const std::string> ids_and_subjects;
  const std::string target_data_structure = "Subjects";
  if (!ids_and_subjects.empty())
    return &ids_and_subjects;

  // Populate
  parse_generic_info_by_id(fetch(SUBJECTS_ENDPOINT), target_data_structure,
                           ids_and_subjects);

  return &ids_and_subjects;
}

std::string api::get_category_by_id(const int &id, api::category_types type) {
  const std::string target_data_structure = "Categories";

  static std::vector<generic_info_id_map> ids_and_categories{
      generic_info_id_map(), generic_info_id_map()};

  if (!ids_and_categories[type].empty())
    return ids_and_categories[type][id];

  switch (type) {
  case GRADE:
    parse_generic_info_by_id(fetch(GRADE_CATEGORIES_ENDPOINT),
                             target_data_structure, ids_and_categories[type]);
    break;

  case EVENT:
    parse_generic_info_by_id(fetch(EVENT_CATEGORIES_ENDPOINT),
                             target_data_structure, ids_and_categories[type]);
    break;
  }

  return ids_and_categories[type][id];
}

std::string api::get_username_by_id(const int &id) {
  static std::unordered_map<int, const std::string> ids_and_usernames;
  if (!ids_and_usernames.empty())
    return ids_and_usernames[id];

  parse_username_by_id(fetch(USERS_ENDPOINT), ids_and_usernames);
  return ids_and_usernames[id];
}

void api::parse_username_by_id(const std::string &response,
                               api::generic_info_id_map &ids_and_usernames) {
  const std::string delimiter = " ";
  const std::string target_data_structure = "Users";

  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  ids_and_usernames.reserve(data[target_data_structure].size());

  for (const auto &username : data[target_data_structure].items()) {
    ids_and_usernames.emplace(username.value()["Id"],
                              (std::string)username.value()["FirstName"] +
                                  delimiter +
                                  (std::string)username.value()["LastName"]);
  }
}

std::string api::get_comment_by_id(const int &id) {
  static std::unordered_map<int, const std::string> ids_and_comments;

  if (!ids_and_comments.empty())
    return ids_and_comments[id];

  parse_comment_by_id(fetch(GRADE_COMMENTS_ENDPOINT), ids_and_comments);
  return ids_and_comments[id];
}

void api::parse_comment_by_id(const std::string &response,
                              generic_info_id_map &ids_and_comments) {
  const std::string target_data_structure = "Comments";

  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  ids_and_comments.reserve(data[target_data_structure].size());

  for (const auto &comment : data[target_data_structure].items())
    ids_and_comments.emplace(comment.value()["Id"], comment.value()["Text"]);
}

api::grades_t api::get_grades() {
  grades_t grades_o;

  parse_grades(fetch(GRADES_ENDPOINT), grades_o);
  return grades_o;
}

void api::parse_grades(const std::string &response, grades_t &grades_o) {
  const std::string target_data_structure = "Grades";
  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  const auto &subjects = get_subjects();
  grades_o.reserve(data[target_data_structure].size());

  for (const auto subject : *subjects)
    grades_o.emplace(subject.first,
                     subject_with_grades_t{.grades = std::vector<grade_t>(),
                                           .subject = subject.second});

  // Populate
  for (const auto &grade : data[target_data_structure].items()) {
    /*grades_o.try_emplace(
                    grade.value()["Subject"]["Id"],
                    subject_with_grades_t {
                            .grades = std::vector<grade_t>(),
                            .subject = ""
                    }
    );*/

    grades_o[grade.value()["Subject"]["Id"]].grades.emplace_back(
        /*.subject                    =*/*get_subject_by_id(
            grade.value()["Subject"]["Id"]), // The api gives invalid subject
                                             // ids only in events so we don't
                                             // have to worry about that here
        /*.grade                      =*/grade.value()["Grade"],
        /*.category                   =*/
        get_category_by_id(grade.value()["Category"]["Id"], GRADE),
        /*.added_by                   =*/
        get_username_by_id(grade.value()["AddedBy"]["Id"]),
        /*.date                       =*/grade.value()["Date"],
        // Why would a grade have multiple comments
        /*.comment                    =*/grade.value().contains("Comments")
            ? std::optional<std::string>(
                  get_comment_by_id(grade.value()["Comments"][0]["Id"]))
            : std::nullopt,
        /*.semester                   =*/grade.value()["Semester"],
        /*.is_semester                =*/grade.value()["IsSemester"],
        /*.is_semester_proposition    =*/grade.value()["IsSemesterProposition"],
        /*.is_final                   =*/grade.value()["IsFinal"],
        /*.is_final_proposition       =*/grade.value()["IsFinalProposition"]);
  }
}

api::grades_unstructured_t api::get_grades_unstructured() {
  grades_unstructured_t grades;

  parse_grades_unstructured(fetch(GRADES_ENDPOINT), grades);
  return grades;
}

void api::parse_grades_unstructured(const std::string &response,
                                    grades_unstructured_t &grades_o) {
  const std::string target_data_structure = "Grades";

  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  grades_o.reserve(data[target_data_structure].size());

  for (int i = data[target_data_structure].size() - 1; i >= 0; i--) {
    const auto &grade = data[target_data_structure].at(i);
    grades_o.emplace_back(
        /*.subject                    =*/*get_subject_by_id(
            grade["Subject"]
                 ["Id"]), // The api gives invalid subject ids only in events so
                          // we don't have to worry about that here
        /*.grade                      =*/grade["Grade"],
        /*.category                   =*/
        get_category_by_id(grade["Category"]["Id"], GRADE),
        /*.added_by                   =*/
        get_username_by_id(grade["AddedBy"]["Id"]),
        /*.date                       =*/grade["Date"],
        /*.comment                    =*/grade.contains("Comments")
            ? std::optional<std::string>(
                  get_comment_by_id(grade["Comments"][0]["Id"]))
            : std::nullopt,
        /*.semester                   =*/grade["Semester"],
        /*.is_semester                =*/grade["IsSemester"],
        /*.is_semester_proposition    =*/grade["IsSemesterProposition"],
        /*.is_final                   =*/grade["IsFinal"],
        /*.is_final_proposition       =*/grade["IsFinalProposition"]);
  }
}

api::events_t api::get_events() {
  events_t events;

  parse_events(fetch(EVENT_ENDPOINT), events);
  return events;
}

void api::parse_events(const std::string &response, api::events_t &events_o) {
  const std::string target_data_structure = "HomeWorks";

  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  events_o.reserve(data[target_data_structure].size());

  for (int i = data[target_data_structure].size() - 1; i >= 0; i--) {
    const auto &event = data[target_data_structure].at(i);
    const std::string date = event["Date"];

    events_o.try_emplace(date, std::vector<event_t>());
    events_o.at(date).emplace_back(
        /*.description =    */ event["Content"],
        /*.category =       */
        get_category_by_id(event["Category"]["Id"], EVENT),
        /*.date =           */ event["Date"],
        /*.created_by =     */ get_username_by_id(event["CreatedBy"]["Id"]),
        /*.subject =        */ event.contains("Subject")
            ? get_subject_by_id(event["Subject"]["Id"])
            : std::nullopt,
        /*.lesson_offset =  */ event["LessonNo"].is_null()
            ? 0
            : std::stoi((std::string)event["LessonNo"]));
  }
}

api::events_unstructured_t api::get_events_unstructured() {
  events_unstructured_t events;

  parse_events_unstructured(fetch(EVENT_ENDPOINT), events);
  return events;
}

void api::parse_events_unstructured(const std::string &response,
                                    api::events_unstructured_t &events) {
  const std::string target_data_structure = "HomeWorks";

  json data = json::parse(response);

  check_if_target_contains(__FUNCTION__, data, target_data_structure);
  events.reserve(data[target_data_structure].size());

  for (int i = data[target_data_structure].size() - 1; i >= 0; i--) {
    const auto &event = data[target_data_structure].at(i);
    const std::string date = event["Date"];

    events.emplace_back(
        /*.description =    */ event["Content"],
        /*.category =       */
        get_category_by_id(event["Category"]["Id"], EVENT),
        /*.date =           */ event["Date"],
        /*.created_by =     */ get_username_by_id(event["CreatedBy"]["Id"]),
        /*.subject =        */ event.contains("Subject")
            ? get_subject_by_id(event["Subject"]["Id"])
            : std::nullopt,
        /*.lesson_offset =  */ event["LessonNo"].is_null()
            ? 0
            : std::stoi((std::string)event["LessonNo"]));
  }
}
