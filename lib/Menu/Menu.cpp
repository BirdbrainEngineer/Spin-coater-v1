#include <Menu.h>

Menu::Menu(MenuData* menuDataConstructor()){
    this->menuDataConstructor = menuDataConstructor;
    ownsItems = true;
}

Menu::Menu(MenuItem* items){
    this->size = (sizeof items) / (sizeof *items); //(sizeof *items) gives size of only a single element
    this->items = items;
    ownsItems = false;
}

Menu::~Menu(){
    if(ownsItems){
        free(items);
    }
}

void Menu::populate(){
    if(ownsItems){
        if(items != nullptr){
            free(items);
        }
        MenuData data = *(menuDataConstructor)();
        this->size = data.size;
        this->items = data.items;
    }
}