/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/mainscreen_screen/MainScreenViewBase.hpp>
#include "BitmapDatabase.hpp"
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>

MainScreenViewBase::MainScreenViewBase()
{

    image1.setXY(0, 0);
    image1.setBitmap(touchgfx::Bitmap(BITMAP_MYBGBLACK_ID));

    textArea.setPosition(8, 9, 780, 461);
    textArea.setColor(touchgfx::Color::getColorFrom24BitRGB(0, 255, 222));
    textArea.setLinespacing(0);
    textAreaBuffer[0] = 0;
    textArea.setWildcard(textAreaBuffer);
    textArea.setTypedText(touchgfx::TypedText(T_SINGLEUSEID2));

    add(image1);
    add(textArea);
}

void MainScreenViewBase::setupScreen()
{

}
