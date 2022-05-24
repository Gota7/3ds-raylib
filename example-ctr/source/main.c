#include "raylib.h"

#include <3ds.h>
#include <stdio.h>

int main()
{
	InitWindow(800, 240, "Test");
	return 0;
	printf("Hi!\n");
	while (!WindowShouldClose())
	{
		gspWaitForVBlank();
		return 0;
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown &KEY_START)
			break; // break in order to return to hbmenu
	}
	printf("Bye!\n");
	return 0;
	CloseWindow(); // TODO: rlgl can't close because it is never opened!
	return 0;
}