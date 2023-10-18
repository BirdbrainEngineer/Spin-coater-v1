#pragma once
#include <Arduino.h>

enum MenuItemType{
    MENU,
    FUNC,
};

enum MenuControlSignal{
    IDLE,
    SELECT,
    RETRACT,
    PREVITEM,
    NEXTITEM,
};

struct MenuItem{
    char* name;
    MenuItemType type;
    void* (*call)(char* caller);
};
struct MenuData{
    u8_t size;
    MenuItem* items;
};

struct MenuContext{
    u8_t numItems;
    u8_t itemSelected;
    char* name;
};

class Menu {
    public:
        u8_t size;
        MenuItem* items;
        bool ownsItems;
        Menu(MenuData (*menuDataConstructor)());
        Menu(MenuItem* items);
        ~Menu();
        void populate();
    private:
        MenuData (*menuDataConstructor)();
};

class MenuController{
    public:
        MenuContext currentMenuContext;
        MenuController(Menu* (*mainMenuConstructor)());
        ~MenuController();
        bool init(u8_t maxMenuDepth, void (*errorHandler)() = nullptr);
        MenuContext update(MenuControlSignal signal);
    private:
        Menu* (*mainMenuConstructor)();
        Menu** menuStack;
        u8_t* stackPointer;
        u8_t currentStackDepth;
        MenuItem* getMenuItemAtPointer();
};