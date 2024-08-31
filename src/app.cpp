//disclaimer: this is my first project in alia. i'm pretty sure that a lot of things aren't written the way developers of this library intended
//            e.g. i couldn't pack Lesson struct into signal, so i just use it as a raw value. 
//            "if it works, it works" is a sentiment present in most of this program, 
//            so if you for some reason want to learn alia(or anything) from this repo, i would advise you to reconsider.

//also it only works on 24-clock, cause it's easier


//INCLUDES
//-Alia https://alia.dev
#include <alia/html.hpp>
#include <alia/html/bootstrap/dropdowns.hpp>
//-nlohmann::json included from source https://json.nlohmann.me/
#include "nlohmann/json.hpp"
//-standard c++ libraries
#include <string>
#include <vector>
#include <chrono>
//-standard c libraries
#include <ctime>
#include <cmath>

//string defined here, so modification/changing language is easier
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
#define EDIT_BUTTON_LABEL "E"
#define DELETE_BUTTON_LABEL "X"

using namespace alia;
using namespace html;

//struct containing data about one lesson
struct Lesson {
    //number of lesson, by which lessons are sorted
    int key;
    //name of the lesson
    std::string name;
    //hour and minute when lesson begins
    int hour, minute;

    //default constructor
    Lesson(){}
    //parametric constructor
    Lesson(int n_key, std::string n_name, int n_hour, int n_minute) : key(n_key), name(n_name), hour(n_hour), minute(n_minute){}

    //equality operator and function, used when deleting lesson
    bool  isEqual(const Lesson& l) const{
        return key == l.key && name == l.name && hour == l.hour && minute == l.minute;
    }

    bool operator==(const Lesson& rhs)
    {
        return isEqual( rhs );
    }

};

//function used by nlohmann::json when serializing/deserializing Lesson objects to/from json
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

//operator used when sorting Lessons by keys
struct {
    bool operator()(Lesson a, Lesson b) const { return a.key < b.key; }
} lesson_by_key_less_operator;

//main container for all the Lessons, divided into weekdays, empty by default
std::map<std::string, std::vector<Lesson>> lessons = {
    {MONDAY,{}},
    {TUESDAY,{}},
    {WEDNESDAY,{}},
    {THURSDAY,{}},
    {FRIDAY,{}}
};

//simple helper std::map used to convert weekday names to numeric values
std::map<std::string, int> weekday_to_number = {
    {MONDAY,1},
    {TUESDAY,2},
    {WEDNESDAY,3},
    {THURSDAY,4},
    {FRIDAY,5}
};

//function calculating and formatting time left until Lesson starts
std::string
time_left(int x, Lesson processed_lesson, std::string day_of_the_week){
    //Get current time and convert to tm struct
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now);
    struct tm * date = localtime(&t_c);

    //calculate difference between current time and start of Lesson
    int difference = (weekday_to_number[day_of_the_week] * 24 * 3600 + processed_lesson.hour * 3600 + processed_lesson.minute * 60) - (date -> tm_wday * 24 * 3600 + date -> tm_hour * 3600 + date -> tm_min * 60 + date -> tm_sec);
    //calculate days, hours, minutes and seconds left until Lesson and add 0 before if values are smaller than 10(except for days)
    int days_left = std::floor(difference / (24*3600));
    int hours_left = std::floor((difference - (days_left * 24 * 3600)) / 3600);
    std::string hours_left_str = std::to_string(hours_left).length()<2 
            ? ("0" + std::to_string(hours_left)) 
            : std::to_string(hours_left);
    int minutes_left = std::floor(((difference - days_left * 24 * 3600) - hours_left * 3600 ) / 60);
    std::string minutes_left_str = std::to_string(minutes_left).length()<2 
            ? ("0" + std::to_string(minutes_left)) 
            : std::to_string(minutes_left);
    int seconds_left = std::floor(((difference - days_left * 24 * 3600) - hours_left * 3600 ) - minutes_left * 60);
    std::string seconds_left_str = std::to_string(seconds_left).length()<2 
            ? ("0" + std::to_string(seconds_left)) 
            : std::to_string(seconds_left);
    //format those time measurements if lessons didn't already start, otherwise display appropriate message
    std::string message = difference < 0 ? LESSON_ALREADY_BEGAN :  TIME_LEFT + std::to_string(days_left) + ":" + hours_left_str + ":" + minutes_left_str + ":" + seconds_left_str;
    return message;
}

//component representing one lesson
void
present_lesson(html::context ctx, Lesson processed_lesson, std::string day_of_the_week, storage_signal timetable){
    //set local state variables concerning editing Lesson to current values of Lesson
    auto is_edit_open = get_state(ctx, false);
    auto new_key = get_state(ctx, processed_lesson.key);
    auto new_name = get_state(ctx, processed_lesson.name);
    auto new_hour = get_state(ctx, processed_lesson.hour);
    auto new_minute = get_state(ctx, processed_lesson.minute);

    //callback called when delete button is pressed
    auto delete_lesson = alia::callback([&]() { 
        //find the Lesson to delete in lessons container
        auto itr = std::remove_if(lessons[day_of_the_week].begin(),lessons[day_of_the_week].end(), [&](Lesson a){return a == processed_lesson;});
        //delete Lesson from container
        lessons[day_of_the_week].erase(itr,lessons[day_of_the_week].end());
        //parse lessons container into json
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        //save json in browser memory
        timetable.write(ss.str());
    });

    //callback called when edit button is pressed
    auto edit_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute) {
        //replace old Lesson with new version
        std::replace(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), processed_lesson, Lesson(n_key, n_name, n_hour, n_minute));
        //sort container by key values
        std::sort(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), lesson_by_key_less_operator);
        //parse lessons container into json
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        //save json in browser memory
        timetable.write(ss.str());
    });

    //add 0 in front of number of minutes when it's less than 10
    std::string formatted_minute = std::to_string(processed_lesson.minute).length()<2 ? ("0" + std::to_string(processed_lesson.minute)) : std::to_string(processed_lesson.minute);
    
    //rendered elements
    html::div(ctx).class_("lesson").content([&] {
        //div containing data about currently processed lesson and edit and delete buttons
        html::div(ctx).class_("lesson_current").content([&] {
            html::text(ctx, (std::to_string(processed_lesson.key) + ". "));
            html::element(ctx, "strong").text((std::to_string(processed_lesson.hour) + ":" + formatted_minute + " "));
            html::text(ctx, processed_lesson.name);
            html::text(ctx, time_left(get_raw_animation_tick_count(ctx) / 1000, processed_lesson, day_of_the_week));
            html::div(ctx).class_("operations").content([&] {
                html::button(ctx, DELETE_BUTTON_LABEL, delete_lesson);
                html::button(ctx, EDIT_BUTTON_LABEL, is_edit_open <<= true);
            });
        });

        //opens menu for editing lesson if editing button was clicked
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

//function rendering main component
void
app_ui(html::context ctx)
{
    //local state variable storing currently displayed weekday
    auto active_weekday = get_state(ctx, std::string(MONDAY));
    //local state variables storing values provided in new lesson form
    auto new_weekday = get_state(ctx, std::string(MONDAY));
    auto new_key = get_state(ctx, empty<int>());
    auto new_name = get_state(ctx, std::string());
    auto new_hour = get_state(ctx, empty<int>());
    auto new_minute = get_state(ctx, empty<int>());

    //variable storing our timetable data in browser memory
    auto timetable = get_local_state(ctx, "timetable");

    //retrieve data from browser memory and load them into lessons container if variable in browser memory isn't empty
    if(!timetable.read().empty()){
        nlohmann::json j2 = nlohmann::json::parse(timetable.read());
        lessons = j2.get<std::map<std::string, std::vector<Lesson>>>();
    }

    //callback called when delete button is pressed
    auto add_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute, std::string day_of_the_week) {
        //add new Lesson with values provided in new lesson form
        lessons[day_of_the_week].push_back(Lesson(n_key, n_name, n_hour, n_minute)); 
        //sort container by key values
        std::sort(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), lesson_by_key_less_operator);
        //parse lessons container into json
        std::stringstream ss;
        nlohmann::json j = lessons;
        ss << std::setw(4) << j;
        //parse lessons container into json
        timetable.write(ss.str());
    });

    //set page title
    document_title(ctx, TIMETABLE);

    //rendering of main element
    placeholder_root(ctx, "app", [&] {
        //dropdown to select currently browsed weekday
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", WEEKDAY, [&](auto& menu) {
            menu.option(MONDAY, active_weekday <<= MONDAY);
            menu.option(TUESDAY, active_weekday <<= TUESDAY);
            menu.option(WEDNESDAY, active_weekday <<= WEDNESDAY);
            menu.option(THURSDAY, active_weekday <<= THURSDAY);
            menu.option(FRIDAY, active_weekday <<= FRIDAY);
        });
        //which weekday was selected
        html::text(ctx, " " + active_weekday);

        //render element for each Lesson
        alia::for_each(ctx, lessons[active_weekday.read()],
        [&](auto each_lesson) {
            present_lesson(ctx, each_lesson, active_weekday.read(), timetable);
        });

        //new lesson form
        html::p(ctx, "\n");
        //check on which day Lesson occurs
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", WEEKDAY, [&](auto& menu) {
            menu.option(MONDAY, new_weekday <<= MONDAY);
            menu.option(TUESDAY, new_weekday <<= TUESDAY);
            menu.option(WEDNESDAY, new_weekday <<= WEDNESDAY);
            menu.option(THURSDAY, new_weekday <<= THURSDAY);
            menu.option(FRIDAY, new_weekday <<= FRIDAY);
        });
        //enter name, key, hour and minute
        html::text(ctx, " " + new_weekday);
        html::p(ctx, KEY);
        html::input(ctx, new_key);
        html::p(ctx, NAME);
        html::input(ctx, new_name);
        html::p(ctx, HOUR);
        html::input(ctx, new_hour);
        html::p(ctx, MINUTE);
        html::input(ctx, new_minute);
        //add Lesson button
        //is enabled only when provided data is correct
        //after it's clicked it resets variables
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


//main function that initializes the alia library and calls main element
int
main()
{
    static html::system the_sys;
    initialize(the_sys, app_ui);
};
