#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

double getWavDuration(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command),
             "ffmpeg -i \"%s\" 2>&1 | grep Duration | cut -d ' ' -f 4 | sed s/,//",
             filename);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to run command\n");
        return -1;
    }

    char buffer[128];
    char result[128] = {0};

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strcat(result, buffer);
    }

    pclose(pipe);

    int hours = 0, minutes = 0;
    float seconds = 0.0f;
    sscanf(result, "%d:%d:%f", &hours, &minutes, &seconds);

    return hours * 3600 + minutes * 60 + seconds;
}

int main(int argc, char *argv[])
{
    InitWindow(800,560,"ray-whisper");

    // Custom file dialog
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    
    char fileNameToLoad[512] = { 0 };

    while(!WindowShouldClose())
    {

        if (fileDialogState.SelectFilePressed)
        {
            // Load image file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".png"))
            {
                // strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                // UnloadTexture(texture);
                // texture = LoadTexture(fileNameToLoad);
            }

            fileDialogState.SelectFilePressed = false;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            if (fileDialogState.windowActive) GuiLock();

            if (GuiButton((Rectangle){ 20, 20, 140, 30 }, GuiIconText(ICON_FILE_OPEN, "Open Image"))) fileDialogState.windowActive = true;

            GuiUnlock();

            // GUI: Dialog Window
            //--------------------------------------------------------------------------------
            GuiWindowFileDialog(&fileDialogState);
            //--------------------------------------------------------------------------------

        EndDrawing();
    }

    CloseWindow();
    
    return EXIT_SUCCESS;
}
