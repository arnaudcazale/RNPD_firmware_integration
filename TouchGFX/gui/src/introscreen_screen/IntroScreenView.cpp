#include <gui/introscreen_screen/IntroScreenView.hpp>

extern "C"
{
#include "RND_Main.h"
extern QueueHandle_t 	printQueue;
}

IntroScreenView::IntroScreenView()
{

}

void IntroScreenView::setupScreen()
{
	IntroScreenViewBase::setupScreen();

	/* display version string */
	uint8_t *p = (uint8_t *)RND_GetVersionString();
    Unicode::fromUTF8(p, this->textArea1Buffer, this->TEXTAREA1_SIZE);
    textArea1.invalidate();

    RND_SetDisplay_Backlight( TRUE);
}

void IntroScreenView::tearDownScreen()
{
    IntroScreenViewBase::tearDownScreen();
}

void IntroScreenView::printText()
{
	char *p;

	if( printQueue)
	{
		if( xQueueReceive( printQueue, &p, 1) == pdPASS)
		{
			if( !strcmp( p,"main"))
				application().gotoMainScreenScreenNoTransition();
		}
	}
}
