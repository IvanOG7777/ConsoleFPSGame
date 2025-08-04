//
// Created by elder on 7/31/2025.
//

#include <cmath>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <algorithm>
#undef max
#undef min

// scrren height and width
int nScreenWidth = 120;
int nScreenHeight = 40;

//initial position of the player
//placing him in the center of the 16/16 map
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;

//initial angle of the player
float fPlayerAngle = 0.0f;
float fPlayerPitch = 0.0f;

// height and width of the map
int nMapHeight = 50;
int nMapWidth = 50;

float fFOV = 3.14159 / 2.0; //fov of the player
float fDepth = 16.0f; // maximum ray must travel to compute something
float mouseSensitivity = 0.0015f;

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

std::vector<std::vector<wchar_t>> generateMap(int height, int width) {
    std::vector<std::vector<wchar_t>> map(height, std::vector<wchar_t>(width));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                map[y][x] = '#';
            }
            else {
                map[y][x] = rand() % 100 < 75 ? L'.' : L'#';
            }
        }
    }
    return map;
}

std::pair<float, float> getStartingPlaceXY(float& playerX, float& playerY, std::vector<std::vector<wchar_t>>& passedMap) {
    std::vector<std::vector<wchar_t>> map = passedMap;
    float middleHeight = map.size() / 2;
    float middleWidth = map[0].size() / 2;

    bool hasFoundSpot = false;
    int height = passedMap.size();
    int width = map[0].size();

    while (hasFoundSpot == false) {
        int randX = rand() % map.size();
        int randY = rand() % map[0].size();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (map[randY][randX] == '.') {
                    hasFoundSpot = true;
                    return { static_cast<float>(randX), static_cast<float>(randY) };
                }
            }
        }
    }
    return { middleHeight, middleWidth };
}



 


int main() {
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwByteWritten = 0;

    //uses a wide string that hold w_Char(wide char of 2 bytes) instead of a regular string of 1 byte
    //allows to use unicode and wide charaters instead of just ascii
    srand((unsigned)time(NULL));
    std::vector<std::vector<wchar_t>> newMap = generateMap(nMapWidth, nMapHeight);
    auto spawn = getStartingPlaceXY(fPlayerX, fPlayerY, newMap);

    fPlayerX = spawn.first;
    fPlayerY = spawn.second;

    //These two values grap the time of the computer are runtime
    //They will have diffrent time but the difference is very small.
    //Used to calculated how much time has passed
    // tp1 = 12:00:00:00001
    // tp2 = 12:00:00:00003
    // elapsedTime = tp2 - tp1
    // elapsedTime = 00002
    auto timePoint1 = std::chrono::system_clock::now();
    auto timePoint2 = std::chrono::system_clock::now();

    // bool to keep game runnig
    bool gameRun = true;

    POINT mousePosition; // POINT struct that hold an X and Y value of the mouse postion

    int screenX = GetSystemMetrics(SM_CXSCREEN) / 2; // GetSystemMetrics gets the screen size, SM_CXSCREEN os the width(X) in pixels, divide by 2 to get middle of screen
    int screenY = GetSystemMetrics(SM_CYSCREEN) / 2; // GetSystemMetrics gets the screen size, SM_CXYCREEN os the height(Y) in pixels, divide by 2 to get middle of screen

    ShowCursor(FALSE); // hides the mouse from view
    SetCursorPos(screenX, screenY); //sets the cursor to be in the middle of the screen using screen X and Y

    while (gameRun) { //while gameRun is true keep looping


		timePoint2 = std::chrono::system_clock::now(); // continuously update timePoint2 once per frame with the systems current clock
		std::chrono::duration<double> elapsedTime = timePoint2 - timePoint1; //calculates the diffrence in time of tp2-tp1 stores in elapsed time in seconds as a double
        timePoint1 = timePoint2; // sets the later time from tp2 to tp1
        float fElapsedTime = elapsedTime.count(); //gets the current elapsed time (in seconds) casts it to float since it was stored as a double
        
        fPlayerPitch = clamp(fPlayerPitch, -1.0f, 1.0f);

        GetCursorPos(&mousePosition); // refercenes the position of the mouse and passes it to GetCursorPos
       
        int deltaX = mousePosition.x - screenX; // deltaX used for change in x axis, calculates how far mouse has moved from center of screen on x axis
        int deltaY = mousePosition.y - screenY; // // deltaY used for change in y axis, calculates how far mouse has moved from center of screen on y axis

        fPlayerAngle += deltaX * mouseSensitivity; // takes current deltaX and multilpies it by the sensitivity Ex: 20 * 0.0015f = 0.03
        fPlayerPitch -= deltaY * mouseSensitivity;

        if (fPlayerAngle < 0) fPlayerAngle += 2 * 3.14159f; // if angle is less than 0 (too far left) we add 2pi radias to the angle to wrap forward
        if (fPlayerAngle >= 2 * 3.14159f) fPlayerAngle -= 2 * 3.14159f; // if angle is more than 2pi (too far right) we subtract 2pi radians to wrap backward

        SetCursorPos(screenX, screenY); // move the mouse back to the center of the screen after every frame

        if (GetAsyncKeyState((unsigned short)'Q') & 0x8000) { // if key is bieng held down uses ascii code and checks key state with GeetAsynKeyState
            gameRun = false;
            break;
        }

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) { // if key is bieng held down uses ascii code and checks key state with GeetAsynKeyState
            float strafeAngle = fPlayerAngle + (3.14159f / 2.0f); // adds 90 degrees (1.570795 Radians) to the current angle placing that number to strafe angle
            // gets current player x coordinate adds and sums the sin of strafe angle * 5 * elapsed time 
            // if strafe angle is aprox 1.600 (91 degrees)
            //sin(1.600) * 5 * .016(60fps) = 0.0799
            //adds 0.0799 to players current x
            //adds -.00234 to players current y
            fPlayerX += sinf(strafeAngle) * 5.0f * fElapsedTime;
            fPlayerY += cosf(strafeAngle) * 5.0f * fElapsedTime;

            // casts current players x and y floats to their leading integer
            // if map at index [y * width + x] == '#' since map is 1d
            if (newMap[(int)fPlayerY][(int)fPlayerX] == '#') {
                fPlayerX -= sinf(strafeAngle) * 5.0f * fElapsedTime; // move back by the same distance we moved up 
                fPlayerY -= cosf(strafeAngle) * 5.0f * fElapsedTime; // move back by the same distance we moved up 
            }
            // above happens per frame, we arent actually 'hitting' anything but imediatly moveing back the same distance we moved forward by
        }

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            float strafeAngle = fPlayerAngle - (3.14159f / 2.0f);
            fPlayerX += sinf(strafeAngle) * 5.0f * fElapsedTime;
            fPlayerY += cosf(strafeAngle) * 5.0f * fElapsedTime;

            if (newMap[(int)fPlayerY][(int)fPlayerX] == '#') {
                fPlayerX -= sinf(strafeAngle) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(strafeAngle) * 5.0f * fElapsedTime;
            }
        }


        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerAngle) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerAngle) * 5.0f * fElapsedTime;

            if (newMap[(int)fPlayerY][(int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerAngle) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerAngle) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerAngle) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerAngle) * 5.0f * fElapsedTime;

            if (newMap[(int)fPlayerY][(int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerAngle) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerAngle) * 5.0f * fElapsedTime;
            }
        }

        for (int x = 0; x < nScreenWidth; x++) { //loop through each x column of the screens width and use that current value of calculations
            float fRayAngle = (fPlayerAngle - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV; // calculates a ray angle from 0 to n-1 depending on fov and player angle and screen width

            float fDistanceToWall = 0; // calculates how far the ray at current angle must travel before "hitting" something
            bool bHitWall = false; // bool to check if we have hit said wall set to false as we havnet hit anything yet
            bool bBoundry = false; // bool to check if we have hit a boundry set to false as we havnet hit anything yet

            float fEyeX = sinf(fRayAngle);// gets current rayAngle and takes the sine, determins how far in the x direction ray must travel
            float fEyeY = cosf(fRayAngle);// gets current rayAngle and takes the cose, determins how far in the y direction ray must travel

            while (!bHitWall && fDistanceToWall < fDepth) { // keep casting ray fowrard until hitWall is true and while ray travels farther than depth
                fDistanceToWall += 0.1f; // move ray foward 0.1 units per frame

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); // Calculates the rays current X position on the map grid 
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall); // Calculates the rays current Y position on the map grid 

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) { // if the x and y positions are not within the map, essentialy meaning we hit a wall
                    bHitWall = true; // set hitWall to true
                    fDistanceToWall = fDepth; // cap the distance to depth (16.0f)
                }
                else { // if we havent hit a wall
                    if (newMap[nTestY][nTestX] == '#') {// if map at current index is a wall(#)
                        bHitWall = true; // we have hit a wall set hitWall to true

                        std::vector < std::pair<float, float >> p; // create a vector of pairs of type floats

                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dotProduct = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(std::make_pair(d, dotProduct));
                            }
                        }
                        std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first;});

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) bBoundry = true;
                        if (acos(p.at(1).second) < fBound) bBoundry = true;
                        if (acos(p.at(2).second) < fBound) bBoundry = true;
                        if (acos(p.at(3).second) < fBound) bBoundry = true;
                    }
                }
            }

            int nWallHeight = (int)(nScreenHeight / fDistanceToWall);
            int nWallTop = (nScreenHeight / 2) - (nWallHeight / 2) + (fPlayerPitch * nScreenHeight);
            int nWallBottom = nWallTop + nWallHeight;

            short nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f) {
                nShade = 0x2588;
            } else if (fDistanceToWall < fDepth / 3.0f) {
                nShade = 0x2593;
            } else if (fDistanceToWall < fDepth / 2.0f) {
                nShade = 0x2592;
            } else if (fDistanceToWall < fDepth) {
                nShade = 0x2591;
            } else {
                nShade = ' ';
            }

            if (bBoundry) nShade = ' ';

            for (int y = 0; y < nScreenHeight; y++) {

                float fMiddleOfScreen = nScreenHeight / 2.0f;
                float fDifferenceFromCenter = y - fMiddleOfScreen;
                float fVerticalAngle = atanf(fDifferenceFromCenter / fMiddleOfScreen);
                float fTotalPitch = fVerticalAngle + fPlayerPitch;


                if (y < nWallTop) {


                    float ceiling = ((float)y - 0.0f) / ((float)nWallTop);
                    if (ceiling < 0.5f) screen[y * nScreenWidth + x] = '~';
                    else if (ceiling < 0.65f) screen[y * nScreenWidth + x] = '-';
                    else if (ceiling < 0.8f) screen[y * nScreenWidth + x] = '.';
                    else screen[y * nScreenWidth + x] = ' ';
                }
                else if (y >= nWallTop && y <= nWallBottom ) {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else {
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25) nShade = '#';
                    else if (b < 0.5) nShade = 'x';
                    else if (b < 0.75) nShade = '-';
                    else if (b < 0.8) nShade = '.';
                    else nShade = '.';
                    screen[y * nScreenWidth + x] = nShade;

                }
            }
        }

        swprintf_s(screen, nScreenWidth, L"X=%3.2f, Y=%3.2f, A=%3.2f, P=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerAngle, fPlayerPitch ,1.0f / fElapsedTime);


        for (int ny = 0; ny < std::min(nMapHeight, nScreenHeight - 1); ny++) {
            for (int nx = 0; nx < std::min(nMapWidth, nScreenWidth); nx++) {
                screen[(ny + 1) * nScreenWidth + nx] = newMap[ny][nx];
            }
        }


        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwByteWritten);
    }

    delete[] screen;
    CloseHandle(hConsole);
    return 0;
}