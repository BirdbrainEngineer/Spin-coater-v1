#include <Arduino.h>

struct MenuData{
    u8_t size;
    MenuItem* items;
};

class Menu {
    public:
        u8_t size;
        MenuItem* items;
        Menu(MenuData* menuDataConstructor());
        Menu(u8_t size, MenuItem* items);
        ~Menu();
        void populate();
    private:
        bool ownsItems;
        MenuData* (*menuDataConstructor)();
};

// struct Menu{
//     u8_t size;
//     MenuItem items[];
//     Menu* 
// };


struct MenuItem{
    char* name;
    void* enter;
    void* exit;
};

class MenuController{
    public:
        MenuController(Menu* mainMenu);
        ~MenuController();
    private:
        Menu* mainMenu;
        Menu* currentMenu;
        Menu* menuStack[sizeof(u8_t)];
        u8_t stackPointer[sizeof(u8_t)];
        u8_t stackDepth;
};