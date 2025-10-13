#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#include <regex.h>
#include <stdbool.h>

#define BUFFER_SIZE 128

static char whisperPath[512] = "~/whisper.cpp/build/bin/whisper-cli";

// Variables globales o parte de alguna estructura
bool PROC_SHOULD_RUN = true;
int progress_ = 0;

void runCommand(char modelPath[512], char inputPath[512]) {
    char fullCommand[512];
    snprintf(fullCommand, sizeof(fullCommand),
             "%s -l es -m %s -f %s",
             whisperPath, modelPath, inputPath);

    FILE* pipe = popen(fullCommand, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to run command.\n");
        return;
    }

    char buffer[BUFFER_SIZE];

    // Expresión regular POSIX
    regex_t regex;
    const char* pattern = "\\[([0-9]+):([0-9]+):([0-9]+)\\.([0-9]+) --> ([0-9]+):([0-9]+):([0-9]+)\\.([0-9]+)\\]";
    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        fprintf(stderr, "Failed to compile regex\n");
        pclose(pipe);
        return;
    }

    regmatch_t matches[9];

    while (fgets(buffer, sizeof(buffer), pipe) != NULL && PROC_SHOULD_RUN) {
        if (regexec(&regex, buffer, 9, matches, 0) == 0) {
            // Extraer los tiempos del grupo 5 al 8 (hora, minuto, segundo, milisegundo)
            char temp[8];

            strncpy(temp, buffer + matches[5].rm_so, matches[5].rm_eo - matches[5].rm_so);
            temp[matches[5].rm_eo - matches[5].rm_so] = '\0';
            int endHours = atoi(temp);

            strncpy(temp, buffer + matches[6].rm_so, matches[6].rm_eo - matches[6].rm_so);
            temp[matches[6].rm_eo - matches[6].rm_so] = '\0';
            int endMinutes = atoi(temp);

            strncpy(temp, buffer + matches[7].rm_so, matches[7].rm_eo - matches[7].rm_so);
            temp[matches[7].rm_eo - matches[7].rm_so] = '\0';
            int endSeconds = atoi(temp);

            // Si quisieras el valor de milisegundos, lo puedes incluir también
            /*
            strncpy(temp, buffer + matches[8].rm_so, matches[8].rm_eo - matches[8].rm_so);
            temp[matches[8].rm_eo - matches[8].rm_so] = '\0';
            int endMilliseconds = atoi(temp);
            */

            int totalSeconds = endHours * 3600 + endMinutes * 60 + endSeconds;
            progress_ = totalSeconds;
        }
    }

    regfree(&regex);
    pclose(pipe);
}


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
    char fileSelectionMode[255] = { 0 };

    char modelPath[512] = "~/whisper.cpp/models/ggml-base.bin";
    char inputPath[512] = "~/whisper.cpp/samples/jfk.wav";
    char outputPath[512] = { 0 };
    
    while(!WindowShouldClose())
    {

        if (fileDialogState.SelectFilePressed)
        {
            strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));

            if(!strcmp(fileSelectionMode,"MODEL"))
            {
                fprintf(stdout, "Modelo seleccionado: ");
                fprintf(stdout, fileNameToLoad);
                fprintf(stdout, "\n");

                strcpy(modelPath, fileNameToLoad);
            }else if(!strcmp(fileSelectionMode,"INPUT"))
            {   
                strcpy(inputPath, fileNameToLoad);
            }else if(!strcmp(fileSelectionMode,"OUTPUT"))
            {   
                strcpy(outputPath, fileNameToLoad);
            }
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

            if (GuiButton((Rectangle){ 20, 20, 200, 30 }, GuiIconText(ICON_GEAR, "Elegir modelo")))
            {
                strcpy(fileSelectionMode, "MODEL");
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ 20, 20*2+30, 200, 30 }, GuiIconText(ICON_AUDIO, "Elegir archivo de audio")))
            {
                strcpy(fileSelectionMode, "INPUT");
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ 20, 20*3+30*2, 200, 30 }, GuiIconText(ICON_FILE_SAVE, "Elegir archivo de salida")))
            {
                strcpy(fileSelectionMode, "OUTPUT");
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ 20, 20*4+30*3, 200, 30 }, GuiIconText(ICON_PLAYER_PLAY, "Convertir")))
            {
                runCommand(modelPath,inputPath);
            }

            GuiTextBoxMulti((Rectangle){ 20, 20*5+30*4, 200, 200 }, "Lorem ipsum dolor sit amet consectetur adipiscing elit massa blandit leo tempor porttitor consequat magna phasellus, vel eget dictum non malesuada nunc netus platea mattis mauris curabitur erat per convallis. Ad nullam eros semper nunc libero vestibulum pharetra accumsan, venenatis gravida a vehicula leo conubia etiam, sem eget diam odio lacus vel rhoncus. Malesuada aenean primis auctor quisque netus nulla hendrerit blandit tortor praesent, sed potenti eu dictumst cum placerat litora vivamus risus ad, imperdiet magnis mollis felis a bibendum suscipit venenatis interdum.\n Massa eros netus volutpat taciti et, nibh eu ultrices velit purus, senectus lobortis inceptos parturient. Eget facilisi dapibus montes commodo placerat purus integer ridiculus nullam, malesuada scelerisque venenatis consequat primis viverra quam lacus cum conubia, nisi orci morbi natoque laoreet id elementum est. Per blandit phasellus habitasse morbi litora rutrum velit, convallis lacinia molestie montes vestibulum mattis, turpis cubilia natoque gravida hac auctor.", 10, 0);

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
