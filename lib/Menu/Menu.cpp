#include <Menu.h>

Menu::Menu(MenuData menuDataConstructor()){
    this->menuDataConstructor = menuDataConstructor;
    this->ownsItems = true;
}

Menu::Menu(MenuItem* items){
    this->size = 5;//sizeof(items) / sizeof(items[0]); 
    this->items = items;
    this->ownsItems = false;
}

Menu::~Menu(){
    if(this->ownsItems){
        free(this->items);
    }
}

void Menu::populate(){
    if(this->ownsItems){
        if(this->items != nullptr){
            free(this->items);
        }
        MenuData data = this->menuDataConstructor();
        this->size = data.size;
        this->items = data.items;
    }
}

MenuController::MenuController(Menu* (*mainMenuConstructor)()){
    this->mainMenuConstructor = mainMenuConstructor;
}

MenuController::~MenuController(){
    free(menuStack);
    free(stackPointer);
}

bool MenuController::init(u8_t maxMenuDepth, void (*errorHandler)()) {
    this->menuStack = (Menu**)malloc(sizeof(Menu*) * maxMenuDepth);
    this->stackPointer = (u8_t*)malloc(sizeof(u8_t) * maxMenuDepth);
    if(this->menuStack == nullptr || this->stackPointer == nullptr){
        free(menuStack);
        free(stackPointer);
        if(errorHandler != nullptr){
            errorHandler();
        }
        return false;
    }
    this->menuStack[0] = this->mainMenuConstructor();
    this->menuStack[0]->populate();
    this->stackPointer[0] = 0;
    this->currentStackDepth = 0;
    this->currentMenuContext;
    this->currentMenuContext.numItems = this->menuStack[this->currentStackDepth]->size;
    this->currentMenuContext.itemSelected = this->stackPointer[this->currentStackDepth] + 1;
    this->currentMenuContext.name = this->getMenuItemAtPointer()->name;
    return true;
}
MenuContext MenuController::update(MenuControlSignal signal){
    switch(signal){
        case IDLE:      return this->currentMenuContext;

        case SELECT:    {
                            switch(this->getMenuItemAtPointer()->type){
                                case FUNC:  {
                                                this->getMenuItemAtPointer()->call(this->getMenuItemAtPointer()->name);
                                                this->menuStack[this->currentStackDepth]->populate();
                                                if(this->menuStack[this->currentStackDepth]->size <= this->stackPointer[this->currentStackDepth]){
                                                    this->stackPointer[this->currentStackDepth] = 0;
                                                }
                                                break;
                                            }   

                                case MENU:  {
                                                Menu* newMenu = (Menu*)this->getMenuItemAtPointer()->call(this->getMenuItemAtPointer()->name);
                                                if(newMenu == nullptr){ break; }
                                                this->currentStackDepth++;
                                                this->menuStack[this->currentStackDepth] = newMenu;
                                                this->menuStack[this->currentStackDepth]->populate();
                                                this->stackPointer[this->currentStackDepth] = 0;
                                                break;
                                            }

                                default:    break;
                            }
                            break;
                        }

        case RETRACT:   {
                            if(this->currentStackDepth > 0){
                                if(this->menuStack[this->currentStackDepth]->ownsItems){
                                    delete this->menuStack[this->currentStackDepth];
                                }
                                this->menuStack[this->currentStackDepth] = nullptr;
                                this->stackPointer[this->currentStackDepth] = 0;
                                this->currentStackDepth--;
                                this->menuStack[this->currentStackDepth]->populate();
                                break;
                            }
                            this->stackPointer[0] = 0;
                        }

        case PREVITEM:    {
                            this->stackPointer[this->currentStackDepth] =   
                                this->stackPointer[this->currentStackDepth] == 0 ?
                                this->menuStack[this->currentStackDepth]->size - 1 :
                                this->stackPointer[this->currentStackDepth] - 1;
                            break;
                        }

        case NEXTITEM:  {
                            this->stackPointer[this->currentStackDepth] =   
                                (this->stackPointer[this->currentStackDepth] == (this->menuStack[this->currentStackDepth]->size - 1)) ?
                                0 :
                                this->stackPointer[this->currentStackDepth] + 1;
                            break;
                        }
    }
    this->currentMenuContext.itemSelected = this->stackPointer[this->currentStackDepth] + 1;
    this->currentMenuContext.name = this->menuStack[this->currentStackDepth]->items[this->stackPointer[this->currentStackDepth]].name;
    this->currentMenuContext.numItems = this->menuStack[this->currentStackDepth]->size;
    return this->currentMenuContext;
}

MenuItem* MenuController::getMenuItemAtPointer(){
    return &this->menuStack[this->currentStackDepth]->items[this->stackPointer[this->currentStackDepth]];
}