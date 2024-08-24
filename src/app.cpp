#include <alia/html.hpp>
#include <alia/html/bootstrap/dropdowns.hpp>
#include <string>
#include <vector>
#include <ranges>

using namespace alia;
using namespace html;

struct Lesson
{
    int key;
    std::string name;
    int hour, minute;

    Lesson(int n_key, std::string n_name, int n_hour, int n_minute) : key(n_key), name(n_name), hour(n_hour), minute(n_minute){}

    bool  isEqual(const Lesson& l) const{
        return key == l.key && name == l.name && hour == l.hour && minute == l.minute;
    }

    bool operator==(const Lesson& rhs)
    {
        return isEqual( rhs );
    }

};


std::map<std::string, std::vector<Lesson>> lessons = 
    {
        {"Poniedziałek",{{1, "Matma", 12, 5},
                        {2, "Polski", 12, 50},
                        {3, "Wf", 12, 5},
                        {4, "Ang", 12, 50},
                        {5, "Niem", 12, 5},
                        {6, "Inf", 12, 50}}
        },
        {"Wtorek",{}},
        {"Środa",{}},
        {"Czwartek",{}},
        {"Piątek",{}}
    }
    ;

void
odliczanie(html::context ctx, Lesson processed_lesson, std::string day_of_the_week){
    auto is_edit_open = get_state(ctx, false);
    auto new_key = get_state(ctx, processed_lesson.key);
    auto new_name = get_state(ctx, processed_lesson.name);
    auto new_hour = get_state(ctx, processed_lesson.hour);
    auto new_minute = get_state(ctx, processed_lesson.minute);

    auto delete_lesson = alia::callback([&]() { 
        auto itr = std::remove_if(lessons[day_of_the_week].begin(),lessons[day_of_the_week].end(), [&](Lesson a){return a == processed_lesson;});
        lessons[day_of_the_week].erase(itr,lessons[day_of_the_week].end());

    });

    auto edit_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute) {std::replace(lessons[day_of_the_week].begin(), lessons[day_of_the_week].end(), processed_lesson, Lesson(n_key, n_name, n_hour, n_minute));});

    std::string formatted_minute = std::to_string(processed_lesson.minute).length()<2 ? ("0" + std::to_string(processed_lesson.minute)) : std::to_string(processed_lesson.minute);
    
    html::div(ctx).class_("lesson").content([&] {
        html::div(ctx).class_("lesson_current").content([&] {
            html::text(ctx, (std::to_string(processed_lesson.key) + ". "));
            html::element(ctx, "strong").text((std::to_string(processed_lesson.hour) + ":" + formatted_minute + " "));
            html::text(ctx, processed_lesson.name);
            html::div(ctx).class_("operations").content([&] {
                html::button(ctx, "X", delete_lesson);
                html::button(ctx, "E", is_edit_open <<= true);
            });
        });

        alia_if(is_edit_open)
        {
            html::div(ctx).class_("lesson_edit").content([&] {
            html::button(ctx, "Close", is_edit_open <<= false);
            html::p(ctx, "Key:");
            html::input(ctx, new_key);
            html::p(ctx, "Name:");
            html::input(ctx, new_name);
            html::p(ctx, "Hour:");
            html::input(ctx, new_hour);
            html::p(ctx, "Minute");
            html::input(ctx, new_minute);
            html::button(ctx, "Edit lesson", edit_lesson << new_key.read() << new_name.read() << new_hour.read() << new_minute.read()
                (
                    edit_lesson << alia::mask(new_key.read(), new_key > 0) 
                                << alia::mask(new_name.read(), !new_name.read().empty()) 
                                << alia::mask(new_hour.read(), new_hour > 0 && new_hour <= 24) 
                                << alia::mask(new_minute.read(), new_minute > 0 && new_minute < 60),
                    new_key <<= 0,
                    new_name <<= "",
                    new_hour <<= 0,
                    new_minute <<= 0,
                )
            );
        });
        }
        alia_end
        
    });
}

// Here's the main UI function for our app.
void
app_ui(html::context ctx)
{
    auto active_weekday = get_state(ctx, std::string("Poniedziałek"));
    auto new_weekday = get_state(ctx, std::string("Poniedziałek"));
    auto new_key = get_state(ctx, empty<int>());
    auto new_name = get_state(ctx, std::string());
    auto new_hour = get_state(ctx, empty<int>());
    auto new_minute = get_state(ctx, empty<int>());

    auto add_lesson = alia::callback(
    [&](int n_key, std::string n_name, int n_hour, int n_minute, std::string day_of_the_week) { lessons[day_of_the_week].push_back(Lesson(n_key, n_name, n_hour, n_minute)); });

    document_title(ctx, "Timetable");

    placeholder_root(ctx, "app", [&] {
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", "Weekday", [&](auto& menu) {
            menu.heading("Options");
            menu.option("Poniedziałek", active_weekday <<= "Poniedziałek");
            menu.option("Wtorek", active_weekday <<= "Wtorek");
            menu.option("Środa", active_weekday <<= "Środa");
            menu.option("Czwartek", active_weekday <<= "Czwartek");
            menu.option("Piątek", active_weekday <<= "Piątek");
        });
        alia::for_each(ctx, lessons[active_weekday.read()],
        [&](auto each_lesson) {
            odliczanie(ctx, each_lesson, active_weekday.read());
        });

        html::p(ctx, "Weekday:");
        alia::html::bootstrap::dropdown_button(ctx, "btn-primary", "Weekday", [&](auto& menu) {
            menu.heading("Options");
            menu.option("Poniedziałek", new_weekday <<= "Poniedziałek");
            menu.option("Wtorek", new_weekday <<= "Wtorek");
            menu.option("Środa", new_weekday <<= "Środa");
            menu.option("Czwartek", new_weekday <<= "Czwartek");
            menu.option("Piątek", new_weekday <<= "Piątek");
        });
        html::p(ctx, "Key:");
        html::input(ctx, new_key);
        html::p(ctx, "Name:");
        html::input(ctx, new_name);
        html::p(ctx, "Hour:");
        html::input(ctx, new_hour);
        html::p(ctx, "Minute");
        html::input(ctx, new_minute);
        html::button(ctx, "Add lesson", 
            (
                add_lesson << alia::mask(new_key.read(), new_key > 0) 
                            << alia::mask(new_name.read(), !new_name.read().empty()) 
                            << alia::mask(new_hour.read(), new_hour > 0 && new_hour <= 24) 
                            << alia::mask(new_minute.read(), new_minute > 0 && new_minute < 60) 
                            << new_weekday.read(),
                new_key <<= 0,
                new_name <<= "",
                new_hour <<= 0,
                new_minute <<= 0,
                new_weekday <<= "Poniedziałek"
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
