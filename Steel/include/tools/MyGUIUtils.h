#ifndef STEEL_MYGUIUTILS_H
#define STEEL_MYGUIUTILS_H

namespace MyGUI
{
    class Widget;
}

namespace Steel
{

    class MyGUIUtils
    {
    public:
        /// **EXPANDS** means that all given directions go **outward** (ie left is substracted and width is added)
        static void expandWidgetCoord(MyGUI::Widget *const widget, int left, int top, int width, int height);
    };
}

#endif // STEEL_MYGUIUTILS_H
