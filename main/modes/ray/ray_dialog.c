//==============================================================================
// Includes
//==============================================================================

#include "fill.h"
#include "ray_dialog.h"
#include "esp_wifi.h"

//==============================================================================
// Defines
//==============================================================================

#define DIALOG_START_Y  (TFT_HEIGHT / 2)
#define DIALOG_MARGIN   16
#define TEXT_MARGIN     8
#define PORTRAIT_OFFSET 4

#define DIALOG_BG_COLOR   c000
#define DIALOG_TEXT_COLOR c240

//==============================================================================
// Constant text
//==============================================================================

const char* const macPuzzleText[] = {
    "WARNING DATA CORRUPTED. Seek other bounty hunters to reconstruct data.\n\n4! is too excited, subtract 12 from "
    "it\n[=/+._?@\n}+&)(/&?\n+[:&${)}",
    "WARNING DATA CORRUPTED. Seek other bounty hunters to reconstruct data.\n\n.?%?-}{;\nTake the next answer, like a "
    "tree find the square\n&,?:)/&:\n:/{#_!&,",
    "WARNING DATA CORRUPTED. Seek other bounty hunters to reconstruct data.\n\n:#:$/{)]\n{?$-}*].\nArgon's number is "
    "nice, but in two it must split\n/}%-%?}%",
    "WARNING DATA CORRUPTED. Seek other bounty hunters to reconstruct data.\n\n{@#??_[!\n#,;{(;-{\n]#_!]=,.\nAverage "
    "two prior, that's it, almost there",
};

//==============================================================================
// Functions
//==============================================================================

/**
 * @brief Show a dialog box
 *
 * @param ray The entire game state
 * @param dialogText The dialog text to show
 * @param dialogPortrait The dialog portrait to draw
 */
void rayShowDialog(ray_t* ray, const char* dialogText, wsg_t* dialogPortrait)
{
    ray->screen = RAY_DIALOG;
    if (0 == strcmp("MAC_PZL", dialogText))
    {
        uint8_t macAddr[6];
        esp_wifi_get_mac(WIFI_IF_STA, macAddr);
        ray->dialogText = macPuzzleText[macAddr[5] % 4];
    }
    else
    {
        ray->dialogText = dialogText;
    }
    ray->dialogPortrait = dialogPortrait;
    ray->btnLockoutUs   = 2000000;
}

/**
 * @brief Check for button input while showing dialog and either show the next part of the dialog or return to the game
 * loop
 *
 * @param ray The entire game state
 */
void rayDialogCheckButtons(ray_t* ray)
{
    // Check the button queue
    buttonEvt_t evt;
    while (checkButtonQueueWrapper(&evt))
    {
        if (0 == ray->btnLockoutUs)
        {
            // If A was pressed
            if (PB_A == evt.button && evt.down)
            {
                // If there is more dialog
                if (NULL != ray->nextDialogText)
                {
                    // Show the next part of the dialog
                    ray->dialogText = ray->nextDialogText;
                }
                else
                {
                    // Dialog over, return to game
                    ray->screen = RAY_GAME;
                }
            }
        }
    }
}

/**
 * @brief Render the current dialog box
 *
 * @param ray The entire game state
 * @param elapsedUs The elapsed time since this function was last called
 */
void rayDialogRender(ray_t* ray, uint32_t elapsedUs)
{
    // Draw the background text box, outline
    drawRect(DIALOG_MARGIN,              //
             DIALOG_START_Y,             //
             TFT_WIDTH - DIALOG_MARGIN,  //
             TFT_HEIGHT - DIALOG_MARGIN, //
             DIALOG_TEXT_COLOR);
    fillDisplayArea(DIALOG_MARGIN + 1,                //
                    DIALOG_START_Y + 1,               //
                    (TFT_WIDTH - DIALOG_MARGIN) - 1,  //
                    (TFT_HEIGHT - DIALOG_MARGIN) - 1, //
                    DIALOG_BG_COLOR);

    // Find the square size for the portrait
    int16_t portraitSquareSize = MAX(ray->dialogPortrait->w, ray->dialogPortrait->h);

    // Draw the background portrait box, outline
    drawRect(TFT_WIDTH - DIALOG_MARGIN - PORTRAIT_OFFSET - portraitSquareSize, //
             DIALOG_START_Y + PORTRAIT_OFFSET - portraitSquareSize,            //
             TFT_WIDTH - PORTRAIT_OFFSET - DIALOG_MARGIN,                      //
             DIALOG_START_Y + PORTRAIT_OFFSET,                                 //
             DIALOG_TEXT_COLOR);
    fillDisplayArea(TFT_WIDTH - DIALOG_MARGIN - PORTRAIT_OFFSET - portraitSquareSize + 1, //
                    DIALOG_START_Y + PORTRAIT_OFFSET - portraitSquareSize + 1,            //
                    TFT_WIDTH - PORTRAIT_OFFSET - DIALOG_MARGIN - 1,                      //
                    DIALOG_START_Y + PORTRAIT_OFFSET - 1,                                 //
                    DIALOG_BG_COLOR);

    // Draw the portrait
    drawWsgSimple(ray->dialogPortrait,                                                                             //
                  TFT_WIDTH - DIALOG_MARGIN - PORTRAIT_OFFSET - (portraitSquareSize + ray->dialogPortrait->w) / 2, //
                  DIALOG_START_Y + PORTRAIT_OFFSET - (portraitSquareSize + ray->dialogPortrait->h) / 2);

    // Draw the text
    int16_t xOff = DIALOG_MARGIN + TEXT_MARGIN;
    int16_t yOff = DIALOG_START_Y + TEXT_MARGIN;
    // Save what the next part of the text will be
    ray->nextDialogText = drawTextWordWrap(&ray->ibm, DIALOG_TEXT_COLOR, ray->dialogText,
                                           &xOff,                                   //
                                           &yOff,                                   //
                                           TFT_WIDTH - DIALOG_MARGIN - TEXT_MARGIN, //
                                           TFT_HEIGHT - DIALOG_MARGIN - TEXT_MARGIN);

    // Blink an arrow to show there's more dialog
    if (ray->blink && 0 == ray->btnLockoutUs)
    {
        drawTriangleOutlined(TFT_WIDTH - DIALOG_MARGIN - 16, TFT_HEIGHT - DIALOG_MARGIN - 4,
                             TFT_WIDTH - DIALOG_MARGIN - 4, TFT_HEIGHT - DIALOG_MARGIN - 10,
                             TFT_WIDTH - DIALOG_MARGIN - 16, TFT_HEIGHT - DIALOG_MARGIN - 16, DIALOG_BG_COLOR,
                             DIALOG_TEXT_COLOR);
    }
}