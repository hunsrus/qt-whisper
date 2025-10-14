#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

typedef struct{
    char modelPath[512];
    char inputPath[512];
    char outputPath[512];
}args;

// #include <regex.h>
// #include <stdbool.h>

#define BUFFER_SIZE 512

// static char whisperPath[512] = "~/whisper.cpp/build/bin/whisper-cli";
static char whisperPath[512] = "..\\src\\whisper\\whisper-cli.exe";

// Variables globales o parte de alguna estructura
bool PROC_SHOULD_RUN = true;
static float progress_ = 0.0f;
static float audioLength = 0.1f;

static char TEXT_VIEW[BUFFER_SIZE];

// void runCommand(char modelPath[512], char inputPath[512]) {
//     char fullCommand[512];
//     snprintf(fullCommand, sizeof(fullCommand),
//              "%s -l es -m %s -f %s",
//              whisperPath, modelPath, inputPath);

//     FILE* pipe = popen(fullCommand, "r");
//     if (!pipe) {
//         fprintf(stderr, "Failed to run command.\n");
//         return;
//     }

//     char buffer[BUFFER_SIZE];

//     // Expresión regular POSIX
//     regex_t regex;
//     const char* pattern = "\\[([0-9]+):([0-9]+):([0-9]+)\\.([0-9]+) --> ([0-9]+):([0-9]+):([0-9]+)\\.([0-9]+)\\]";
//     if (regcomp(&regex, pattern, REG_EXTENDED)) {
//         fprintf(stderr, "Failed to compile regex\n");
//         pclose(pipe);
//         return;
//     }

//     regmatch_t matches[9];

//     while (fgets(buffer, sizeof(buffer), pipe) != NULL && PROC_SHOULD_RUN) {
//         if (regexec(&regex, buffer, 9, matches, 0) == 0) {
//             // Extraer los tiempos del grupo 5 al 8 (hora, minuto, segundo, milisegundo)
//             char temp[8];

//             strncpy(temp, buffer + matches[5].rm_so, matches[5].rm_eo - matches[5].rm_so);
//             temp[matches[5].rm_eo - matches[5].rm_so] = '\0';
//             int endHours = atoi(temp);

//             strncpy(temp, buffer + matches[6].rm_so, matches[6].rm_eo - matches[6].rm_so);
//             temp[matches[6].rm_eo - matches[6].rm_so] = '\0';
//             int endMinutes = atoi(temp);

//             strncpy(temp, buffer + matches[7].rm_so, matches[7].rm_eo - matches[7].rm_so);
//             temp[matches[7].rm_eo - matches[7].rm_so] = '\0';
//             int endSeconds = atoi(temp);

//             // Si quisieras el valor de milisegundos, lo puedes incluir también
//             /*
//             strncpy(temp, buffer + matches[8].rm_so, matches[8].rm_eo - matches[8].rm_so);
//             temp[matches[8].rm_eo - matches[8].rm_so] = '\0';
//             int endMilliseconds = atoi(temp);
//             */

//             int totalSeconds = endHours * 3600 + endMinutes * 60 + endSeconds;
//             progress_ = totalSeconds;
//         }
//     }

//     regfree(&regex);
//     pclose(pipe);
// }

void killWhisper()
{
    char cmd[128];
    sprintf(cmd, "taskkill /F /IM whisper-cli.exe");
    
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to run command.\n");
    }

    pclose(pipe);
}

// void* runCommand(char modelPath[512], char inputPath[512]) {
void* runCommand(void* paths)
{
    char fullCommand[512];
    snprintf(fullCommand, sizeof(fullCommand),
             "%s -l es -m %s -f %s --output-txt --output-file %s 2>&1",
             whisperPath, ((args*)paths)->modelPath, ((args*)paths)->inputPath, ((args*)paths)->outputPath);

    FILE* pipe = popen(fullCommand, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to run command.\n");
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    int samples;

    char format[512];
    sprintf(format, "main: processing '%s' (%%d samples, %%f sec)", ((args*)paths)->inputPath);

    while (fgets(buffer, sizeof(buffer), pipe) != NULL && PROC_SHOULD_RUN) {

        int sh, sm, ss, sms;
        int eh, em, es, ems;
        fprintf(stdout, buffer);
        if(sscanf(buffer, format, &samples ,&audioLength) == 2)
        {
            fprintf(stdout, "audio length: %f\n", audioLength);
        }

        if (sscanf(buffer, " [%d:%d:%d.%d --> %d:%d:%d.%d]",
                   &sh, &sm, &ss, &sms,
                   &eh, &em, &es, &ems) == 8) {

            int totalSeconds = eh * 3600.0f + em * 60.0f + es;
            progress_ = totalSeconds;

            strcpy(TEXT_VIEW,buffer);
            // fprintf(stdout, "progress: %d\n", progress_);
        }
    }

    pclose(pipe);
}

int main(int argc, char *argv[])
{
    int windowWidth = 460;
    int windowHeight = 240;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth,windowHeight,"ray-whisper");

    // Custom file dialog
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    float margin = windowHeight/20.0f;
    Vector2 buttonSize = {(windowWidth-margin*3.0f)/2.0f, (windowHeight-margin*6.0f)/5.0f};
    Vector2 textBoxSize = {buttonSize.x, windowHeight-margin*2.0f};
    
    char fileNameToLoad[512] = { 0 };
    char fileSelectionMode[255] = { 0 };

    args *paths = (args *)malloc(sizeof(args));
    strcpy(paths->modelPath, "..\\src\\models\\ggml-base.bin");
    strcpy(paths->inputPath, "..\\src\\samples\\000981_jfk-space-race-speech-59951.mp3");
    strcpy(paths->outputPath, "..\\src\\samples\\000981_jfk-space-race-speech-59951.txt");

    pthread_t whisperThread;
    
    while(!WindowShouldClose())
    {
        if(IsWindowResized())
        {
            windowWidth = GetScreenWidth();
            windowHeight = GetScreenHeight();

            margin = windowHeight/20.0f;
            buttonSize.x = (windowWidth-margin*3.0f)/2.0f;
            buttonSize.y = (windowHeight-margin*6.0f)/5.0f;
            textBoxSize.x = buttonSize.x;
            textBoxSize.y = windowHeight-margin*2.0f;
        }

        if (fileDialogState.SelectFilePressed)
        {
            strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));

            if(!strcmp(fileSelectionMode,"MODEL"))
            {
                fprintf(stdout, "Modelo seleccionado: ");
                fprintf(stdout, fileNameToLoad);
                fprintf(stdout, "\n");

                strcpy(paths->modelPath, fileNameToLoad);
            }else if(!strcmp(fileSelectionMode,"INPUT"))
            {   
                strcpy(paths->inputPath, fileNameToLoad);
            }else if(!strcmp(fileSelectionMode,"OUTPUT"))
            {   
                strcpy(paths->outputPath, fileNameToLoad);
            }

            fileDialogState.SelectFilePressed = false;
            fileDialogState.saveFileMode = false;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            if (fileDialogState.windowActive) GuiLock();

            if (GuiButton((Rectangle){ margin, margin, buttonSize.x, buttonSize.y }, GuiIconText(ICON_GEAR, "Elegir modelo")))
            {
                strcpy(fileSelectionMode, "MODEL");
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ margin, margin*2+buttonSize.y, buttonSize.x, buttonSize.y }, GuiIconText(ICON_AUDIO, "Elegir archivo de audio")))
            {
                strcpy(fileSelectionMode, "INPUT");
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ margin, margin*3+buttonSize.y*2, buttonSize.x, buttonSize.y }, GuiIconText(ICON_FILE_SAVE, "Elegir archivo de salida")))
            {
                strcpy(fileSelectionMode, "OUTPUT");
                fileDialogState.saveFileMode = true;
                fileDialogState.windowActive = true;
            }

            if (GuiButton((Rectangle){ margin, margin*4+buttonSize.y*3, buttonSize.x, buttonSize.y }, GuiIconText(ICON_PLAYER_PLAY, "Convertir")))
            {
                pthread_create(&whisperThread, NULL, runCommand, (void *)paths);
            }

            GuiProgressBar((Rectangle){ margin, margin*5+buttonSize.y*4, buttonSize.x, buttonSize.y },"", "", &progress_, 0, audioLength);

            GuiTextBoxMulti((Rectangle){ margin*2+buttonSize.x, margin, textBoxSize.x, textBoxSize.y }, TEXT_VIEW, 10, 0);


            GuiUnlock();

            // GUI: Dialog Window
            //--------------------------------------------------------------------------------
            GuiWindowFileDialog(&fileDialogState);
            //--------------------------------------------------------------------------------

        EndDrawing();
    }

    PROC_SHOULD_RUN = false;

    CloseWindow();

    killWhisper();
    pthread_join(whisperThread, NULL);
    // pthread_kill(whisperThread, SIGTERM);
    
    return EXIT_SUCCESS;
}
