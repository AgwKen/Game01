#ifndef RESULT_H
#define RESULT_H

// Initialize the result screen
void Result_Initialize();

// Cleanup when leaving result screen
void Result_Finalize();

// Update function called every frame
void Result_Update(double elapsed_time);

// Draw function called every frame
void Result_Draw();

#endif // RESULT_H

