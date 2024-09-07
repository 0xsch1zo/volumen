#pragma once

class tab {
    enum tabs {
        HOMESCREEN,
        MESSAGES,
        ANNOUCEMENTS,
        TIMETABLE
    };

public:
    enum ui_screens {
        MESSAGE_VIEW,
        ANNOUCEMENT_VIEW,
        EXIT
    };
    
    static void display_interface(int synergia_account_i);
};