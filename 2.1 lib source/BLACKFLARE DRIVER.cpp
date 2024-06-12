#include <interception.h>
#include <cstring>
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>

extern "C" __declspec(dllexport) void move_mouse(float x, float y);
extern "C" __declspec(dllexport) void mouse_click();
extern "C" __declspec(dllexport) void get_mouse_pos(int* x, int* y);
extern "C" __declspec(dllexport) void slow_down_mouse(bool slow_down, double timeout_seconds);

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

void slow_down_mouse(bool slow_down, double timeout_seconds) {
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    // Interception context initialization
    context = interception_create_context();

    // Filter for mouse move events
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

    // Set the scaling factor (0.5 will reduce speed by half)
    double scalingFactor = 0.5;

    // Get the current time in seconds
    time_t start_time = time(NULL);

    // Main loop to intercept and modify mouse events
    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        if (interception_is_mouse(device)) {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

            // Modify the mouse move deltas by the scaling factor if slow_down is true
            if (slow_down) {
                mstroke.x *= scalingFactor;
                mstroke.y *= scalingFactor;
            }

            // Send the modified mouse event back to the system
            interception_send(context, device, &stroke, 1);
        }

        // Check if the timeout has been reached
        if (time(NULL) - start_time >= timeout_seconds) {
            break; // Quit the loop if the timeout has been reached
        }
    }

    // Clean up the interception context
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
