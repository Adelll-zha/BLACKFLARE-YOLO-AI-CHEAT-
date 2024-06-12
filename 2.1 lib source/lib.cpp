#include <interception.h>
#include <cstring>
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>

extern "C" __declspec(dllexport) void move_mouse(float x, float y);
extern "C" __declspec(dllexport) void mouse_click();
extern "C" __declspec(dllexport) void get_mouse_pos(int* x, int* y);

int main() {

}

void move_mouse(float x, float y) {
    InterceptionContext context = interception_create_context();

    // Creates an InterceptionMouseStroke object to move the cursor
    InterceptionMouseStroke stroke;
    stroke.state = INTERCEPTION_FILTER_MOUSE_MOVE;
    stroke.flags = 0;
    stroke.rolling = 0;
    stroke.x = x;
    stroke.y = y;
    stroke.information = 0;

    // Copies stroke data to first strokes element
    InterceptionStroke strokes[1];
    memcpy(strokes, &stroke, sizeof(strokes));

    // Sends cursor movement
    interception_send(context, INTERCEPTION_MOUSE(0), strokes, 1);

    // Free up resources
    interception_destroy_context(context);
}

void mouse_click() {
    InterceptionContext context = interception_create_context();

    // Creates an InterceptionMouseStroke object for the click
    InterceptionMouseStroke stroke;
    stroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
    stroke.flags = 0;
    stroke.rolling = 0;
    stroke.x = 0;
    stroke.y = 0;
    stroke.information = 0;

    // Copies stroke data to first strokes element
    InterceptionStroke strokes[1];
    memcpy(strokes, &stroke, sizeof(strokes));

    // Send click
    interception_send(context, INTERCEPTION_MOUSE(0), strokes, 1);

    // Free up resources
    interception_destroy_context(context);
}

void __stdcall get_mouse_pos(int* x, int* y) {
    InterceptionContext context = interception_create_context();
    InterceptionDevice device = INTERCEPTION_MOUSE(0); // Device 0 is mouse

    InterceptionMouseStroke stroke;
    interception_receive(context, device, (InterceptionStroke*)&stroke, 1);

    *x = stroke.x;
    *y = stroke.y;

    interception_destroy_context(context);
}
