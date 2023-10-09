#pragma once
#include <Arduino.h>

enum MenuItemType{
    MENU,
    FUNC,
};

struct MenuItem{
    char* name;
    MenuItemType type;
    void* (*enter)();
    void* (*exit)();
};
struct MenuData{
    u8_t size;
    MenuItem* items;
};

class Menu {
    public:
        u8_t size;
        MenuItem* items;
        Menu(MenuData menuDataConstructor());
        Menu(MenuItem* items);
        ~Menu();
        void populate();
    private:
        bool ownsItems;
        MenuData (*menuDataConstructor)();
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