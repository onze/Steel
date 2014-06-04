
#include "tools/MyGUIUtils.h"

#include "steeltypes.h"
#include <MyGUI.h>
#include <MyGUI_Widget.h>

namespace Steel
{
    void MyGUIUtils::expandWidgetCoord(MyGUI::Widget *const widget, int left, int top, int width, int height)
    {
        MyGUI::IntCoord coords = widget->getCoord();
        coords.set(coords.left - left, coords.top - top, coords.width + width, coords.height + height);
        widget->setCoord(coords);
    }
}
