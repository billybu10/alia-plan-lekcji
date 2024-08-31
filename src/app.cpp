//disclaimer: this is my first project in alia. i'm pretty sure that a lot of things aren't written the way developers of this library intended
//            e.g. i couldn't pack Lesson struct into signal, so i just use it as a raw value. 
//            "if it works, it works" is a sentiment present in most of this program, 
//            so if you for some reason want to learn alia(or anything) from this repo, i would advise you to reconsider.
#include <alia/html.hpp>
#include <alia/html/bootstrap/dropdowns.hpp>
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <cmath>

#define MONDAY "Monday"
#define TUESDAY "Tuesday"
#define WEDNESDAY "Wednesday"
#define THURSDAY "Thursday"
#define FRIDAY "Friday"
#define CLOSE "Close"
#define KEY "Key: "
#define NAME "Name: "
#define HOUR "Hour: "
#define MINUTE "Minute: "
#define EDIT_LESSON "Edit lesson"
#define ADD_LESSON "Add lesson"
#define TIMETABLE "Timetable"
#define WEEKDAY "Weekday"
#define TIME_LEFT " Time left: "
#define LESSON_ALREADY_BEGAN " Lesson already began. See you next week!"

using namespace alia;
using namespace html;


struct Lesson {
    int key;
    std::string name;
    int hour, minute;

    Lesson(){}
    Lesson(int n_key, std::string n_name, int n_hour, int n_minute) : key(n_key), name(n_name), hour(n_hour), minute(n_minute){}

    bool  isEqual(const Lesson& l) const{
        return key == l.key && name == l.name && hour == l.hour && minute == l.minute;
    }

    bool operator==(const Lesson& rhs)
    {
        return isEqual( rhs );
    }

};

void from_json(const nlohmann::json& j, Lesson& val)
{
    j.at("key").get_to(val.key);
    j.at("name").get_to(val.name);
    j.at("hour").get_to(val.hour);
    j.at("minute").get_to(val.minute);
}

void to_json(nlohmann::json& j, const Lesson& val)
{
    j["key"] = val.key;
    j["name"] = val.name;
    j["hour"] = val.hour;
    j["minute"] = val.minute;
}

struct {
    bool operator()(Lesson a, Lesson b) const { return a.key < b.key; }
} lesson_by_key_less_operator;


std::map<std::string, std::vector<Lesson>> lessons = {
    {MONDAY,{}},
    {TUESDAY,{}},
    {WEDNESDAY,{}},
    {THURSDAY,{}},
    {FRIDAY,{}}
};

std::map<std::string, int> weekday_to_number = {
    {MONDAY,1},
    {TUESDAY,2},
    {WEDNESDAY,3},
    {THURSDAY,4},
    {FRIDAY,5}
};

std::string
time_left(int x, Lesson processed_lesson, std::string day_of_the_week){
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now);
    struct tm * date = localtime(&t_c);
    int difference = (weekday_to_number[day_of_the_week] * 24 * 3600 + processed_lesson.hour * 3600 + processed_lesson.minute * 60) - (date -> tm_wday * 24 * 3600 + date -> tm_hour * 3600 + date -> tm_min * 60 + date -> tm_sec);
    int days_left = std::floor(difference / (24*3600));
    int hours_left = std::floor((difference - (days_left * 24 * 3600)) / 3600);
    int minutes_left = std::floor(((difference - days_left * 24 * 3600) - hours_left * 3600 ) / 60);
    int seconds_left = std::floor(((difference - days_left * 24 * 3600) - hours_left * 3600 ) - minutes_left * 60);
    std::string message = difference < 0 ? LESSON_ALREADY_BEGAN :  TIME_LEFT + ::to_string(days_left) + ":" + std::to_string(hours_left) + ":" + std::to_string(minutes_left) + ":" + std::to_string(seconds_left);
    return message;
}

void
present_lesson(html::context ctx, Lesson processed_lesson, std::string day_of_the_week, storage_signal timetable){
    auto is_edit_open = get_state(ctx, false);
    auto new_key = get_state(ctx, processed_lesson.key);
    auto new_name = get_state(ctx, processed_lesson.name);
    auto new_hour = get_state(ctx, processed_lesson.hour);
    auto new_minute = get_state(ctx, processed_lesson.minute);

    auto delete_lesson = alia::callback([&]() { 
        auto itr = std::remove_if(lessons[day_of_the_week].begin(),lessons[day_of_the_week].end(), [&](Lesson a){return a == processed_lesson;});
        lessons[day_of_the_week].erase(itr,lessons[day_of_the_week].end());
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        timetable.write(ss.str());
    });

    auto edit_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute) {
        std::replace(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), processed_lesson, Lesson(n_key, n_name, n_hour, n_minute));
        std::sort(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), lesson_by_key_less_operator);
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        timetable.write(ss.str());
    });

    std::string formatted_minute = std::to_string(processed_lesson.minute).length()<2 ? ("0" + std::to_string(processed_lesson.minute)) : std::to_string(processed_lesson.minute);
    
    html::div(ctx).class_("lesson").content([&] {
        html::div(ctx).class_("lesson_current").content([&] {
            html::text(ctx, (std::to_string(processed_lesson.key) + ". "));
            html::element(ctx, "strong").text((std::to_string(processed_lesson.hour) + ":" + formatted_minute + " "));
            html::text(ctx, processed_lesson.name);
            html::text(ctx, time_left(get_raw_animation_tick_count(ctx) / 1000, processed_lesson, day_of_the_week));
            html::div(ctx).class_("operations").content([&] {
                html::button(ctx, "X", delete_lesson);
                html::button(ctx, "E", is_edit_open <<= true);
            });
        });

        alia_if(is_edit_open)
        {
            html::div(ctx).class_("lesson_edit").content([&] {
            html::button(ctx, CLOSE, is_edit_open <<= false);
            html::p(ctx, KEY);
            html::input(ctx, new_key);
            html::p(ctx, NAME);
            html::input(ctx, new_name);
            html::p(ctx, HOUR);
            html::input(ctx, new_hour);
            html::p(ctx, MINUTE);
            html::input(ctx, new_minute);
            html::button(ctx, EDIT_LESSON,
                (
                    edit_lesson << alia::mask(new_key.read(), new_key > 0) 
                                << alia::mask(new_name.read(), !new_name.read().empty()) 
                                << alia::mask(new_hour.read(), new_hour > 0 && new_hour <= 24) 
                                << alia::mask(new_minute.read(), (new_minute > -1 && new_minute < 60) || new_minute == 0),
                    new_key <<= 0,
                    new_name <<= "",
                    new_hour <<= 0,
                    new_minute <<= -1
                )
            );
        });
        }
        alia_end
        
    });
}


void
app_ui(html::context ctx)
{
    auto active_weekday = get_state(ctx, std::string(MONDAY));
    auto new_weekday = get_state(ctx, std::string(MONDAY));
    auto new_key = get_state(ctx, empty<int>());
    auto new_name = get_state(ctx, std::string());
    auto new_hour = get_state(ctx, empty<int>());
    auto new_minute = get_state(ctx, empty<int>());
    auto timetable = get_local_state(ctx, "timetable");
    //TODO: READ
    if(!timetable.read().empty()){
        nlohmann::json j2 = nlohmann::json::parse(timetable.read());
        lessons = j2.get<std::map<std::string, std::vector<Lesson>>>();
    }

    auto add_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute, std::string day_of_the_week) {
        lessons[day_of_the_week].push_back(Lesson(n_key, n_name, n_hour, n_minute)); 
        std::sort(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), lesson_by_key_less_operator);
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        timetable.write(ss.str());
    });

    document_title(ctx, TIMETABLE);

    placeholder_root(ctx, "app", [&] {
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", WEEKDAY, [&](auto& menu) {
            menu.option(MONDAY, active_weekday <<= MONDAY);
            menu.option(TUESDAY, active_weekday <<= TUESDAY);
            menu.option(WEDNESDAY, active_weekday <<= WEDNESDAY);
            menu.option(THURSDAY, active_weekday <<= THURSDAY);
            menu.option(FRIDAY, active_weekday <<= FRIDAY);
        });
        html::text(ctx, " " + active_weekday);
        alia::for_each(ctx, lessons[active_weekday.read()],
        [&](auto each_lesson) {
            present_lesson(ctx, each_lesson, active_weekday.read(), timetable);
        });

        html::p(ctx, "\n");
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", WEEKDAY, [&](auto& menu) {
            menu.option(MONDAY, new_weekday <<= MONDAY);
            menu.option(TUESDAY, new_weekday <<= TUESDAY);
            menu.option(WEDNESDAY, new_weekday <<= WEDNESDAY);
            menu.option(THURSDAY, new_weekday <<= THURSDAY);
            menu.option(FRIDAY, new_weekday <<= FRIDAY);
        });
        html::text(ctx, " " + new_weekday);
        html::p(ctx, KEY);
        html::input(ctx, new_key);
        html::p(ctx, NAME);
        html::input(ctx, new_name);
        html::p(ctx, HOUR);
        html::input(ctx, new_hour);
        html::p(ctx, MINUTE);
        html::input(ctx, new_minute);
        html::button(ctx, ADD_LESSON, 
            (
                add_lesson << alia::mask(new_key.read(), new_key > 0) 
                            << alia::mask(new_name.read(), !new_name.read().empty()) 
                            << alia::mask(new_hour.read(), new_hour > 0 && new_hour <= 24) 
                            << alia::mask(new_minute.read(), new_minute >= 0 && new_minute < 60) 
                            << new_weekday.read(),
                new_key <<= 0,
                new_name <<= "",
                new_hour <<= 0,
                new_minute <<= 0,
                new_weekday <<= MONDAY
            )
        );
    });
}

int
main()
{
    static html::system the_sys;
    initialize(the_sys, app_ui);
};
