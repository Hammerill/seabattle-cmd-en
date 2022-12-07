#include <iostream>
#include <time.h>
#include <windows.h>
#include <conio.h>
using namespace std;
enum Color {Black = 0, Blue, Green, Cyan, Red, Magenta, Brown, LightGray, DarkGray, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow, White};

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
void setc(Color text = White, Color background = Black)
{
	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

void setp(int x = 0, int y = 0)
{
	COORD myCoords = {x, y};
	SetConsoleCursorPosition(hStdOut, myCoords);
}

// Shows menu, proposes player to choose something and returns its index
int menu(char* choices[], int length, int x = 5, int y = 5)
{
    int choice = 0, oldchoice, button;
    bool key;

    for (size_t i = 0; i < length; i++)
    {
        setp(x, y + i);
        if (i == 0)
        {
            setc(White, Red);
        }
        else
        {
            setc();
        }
        cout << choices[i] << '\n';
    }
    setc();
    setp(x, y + length + 3);
    cout << "Use W and S to navigate through the menu.\n";
    setp(x, y + length + 4);
    cout << "Use Enter to confirm choice.";
    do
    {
        oldchoice = choice;
        button = _getch();
        key = true;

        switch (button)
        {
        case 119:                       //W
            if (choice <= 0)
            {
                choice = length - 1;
            }
            else
            {
                choice--;
            }
            break;

        case 115:                       //S
            if (choice >= length - 1)
            {
                choice = 0;
            }
            else
            {
                choice++;
            }
            break;

        default:
            key = false;
            break;
        }

        if (key)
        {
            setc();
            setp(x, y + oldchoice);
            cout << choices[oldchoice];

            setc(White, Red);
            setp(x, y + choice);
            cout << choices[choice];
        }

    } while (button != 13);             //Enter

    return choice;
}

// To declare a Boat we need to know its...
struct Boat
{
    int x;              // X...
    int y;              // ... and Y position;
    int len;            // its length;
    bool isHorizontal;  // and is it placed horizontally.
};

// Bot intelligence is consists of:
struct Bot
{
    bool remember;  // does they remember where is the Boat (from the start No; when they found a Boat Yes; then after destroying it again No and cycle); 
    int x;          // X...
    int y;          // ... and Y position of the tile where Boat was found;
    int dir;        // which is direction of the Boat according to them (0 - don't know; 1 - horizontal; 2 - vertical)
};

// Clear Bot intelligence struct
void clearBot(Bot& bot)
{
    bot.remember = false;
    bot.x = 0;
    bot.y = 0;
    bot.dir = 0;
}

// Draw one field tile to the screen
void drawTile(int tilenum, int x, int y, bool debug)
{
    setp(x, y);
    if (tilenum == 1 && debug)   // With boat (debug)
    {
        setc(Red, Red);
    }
    else if (tilenum == 2)       // Miss
    {
        setc(DarkGray, Black);
    }
    else if (tilenum == 3)       // Injured Boat
    {
        setc(Yellow, Red);
    }
    else if (tilenum == 4)       // Destroyed Boat
    {
        setc(DarkGray, DarkGray);
    }
    else                         // Empty or with Boat (no debug)
    {
        setc(Black, Black);
    }
    cout << '*';
}

// Draw an empty tile
void drawTile(Color color, int x, int y)
{
    setc(color, color);
    setp(x, y);
    cout << '*';
}

// Draw a field
// If debug enabled, will show all the boats
void drawField(int ground[10][10], int x = 1, int y = 1, bool debug = true)
{
    for (size_t yi = 0; yi < 10; yi++)
    {
        for (size_t xi = 0; xi < 10; xi++)
        {
            drawTile(ground[xi][yi], x + xi, y + yi, debug);
        }
    }
}

// Draw a border of the field
void drawBorder(int x = 0, int y = 0)
{
    setc(White, White);
    setp(x, y);
    cout << '*';

    setc(Black, White);
    for (size_t i = 1; i < 11; i++)
    {
        setp(x, y + i);
        cout << i - 1;
    }

    setc(White, White);
    setp(x, y + 11);
    cout << '*';

    setc(White, White);
    for (size_t i = 0; i < 12; i++)
    {
        setp(x + 11, y + i);
        cout << '*';
    }

    setc(Black, White);
    setp(x + 1, y);
    cout << "ABCDEFGHIJ";

    setc(White, White);
    setp(x + 1, y + 11);
    cout << "**********";
}

// Clear field array
void clearField(int ground[10][10])
{
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            ground[i][j] = 0;
        }
    }
}

// Clear an array of the Boats
void clearBoats(Boat boats[10])
{
    for (size_t i = 0; i < 10; i++)
    {
        boats[i].x = 0;
        boats[i].y = 0;
        boats[i].len = 0;
        boats[i].isHorizontal = 0;
    }
}

// Copies parameters of the one boat to another
void copyBoat(Boat source, Boat& destination)
{
    destination.x = source.x;
    destination.y = source.y;
    destination.len = source.len;
    destination.isHorizontal = source.isHorizontal;
}

// Add one Boat to the field
void addBoat(int ground[10][10], Boat boat, Boat boats_massive[10], int cur_id)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal && ground[boat.x + i][boat.y] == 0)
        {
            ground[boat.x + i][boat.y] = 1;
        }
        else if (ground[boat.x][boat.y + i] == 0)
        {
            ground[boat.x][boat.y + i] = 1;
        }
    }

    copyBoat(boat, boats_massive[cur_id]);
}

// Show one specified Boat (used in the field constructor)
void showBoat(Boat boat, Color color = Blue, int x = 1, int y = 1)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal)
        {
            drawTile(color, x + boat.x + i, y + boat.y);
        }
        else
        {
            drawTile(color, x + boat.x, y + boat.y + i);
        }
    }
}

// Show Boat with BG color (to clear it)
void showBoat(Boat boat, int ground[10][10], int x = 1, int y = 1)
{
    for (size_t i = 0; i < boat.len; i++)
    {
        if (boat.isHorizontal)
        {
            drawTile(ground[boat.x + i][boat.y], x + boat.x + i, y + boat.y, true);
        }
        else
        {
            drawTile(ground[boat.x][boat.y + i], x + boat.x, y + boat.y + i, true);
        }
    }
}

// Check if Boat can fit in the field
bool inBorders(Boat boat)
{
    if (boat.x < 0 || boat.y < 0)   // If its base is outside left or top border
    {
        return false;
    }

    if (boat.isHorizontal)  // If it's horizontal
    {
        if ((boat.x + boat.len > 10) || (boat.y > 9))   // If it's outside right or bottom border
        {
            return false;
        }
    }
    else    // If it's vertical
    {
        if ((boat.y + boat.len > 10) || (boat.x > 9))   // If it's outside right or bottom border
        {
            return false;
        }
    }

    return true;
}

// Check if we can place the Boat in this field
bool canPlaceHere(int ground[10][10], Boat boat, int scanFor = 1)
{
    if (!inBorders(boat))   // If it doesn't fit in the borders
    {
        return false;
    }

    if (boat.isHorizontal)  // If it's horizontal
    {
        for (int i = -1; i < boat.len + 1; i++) // Check line on the top of the Boat
        {
            if ((boat.x + i >= 0) && (boat.y - 1 >= 0) && (boat.x + i <= 9))    // If it fits
            {
                if (ground[boat.x + i][boat.y - 1] == scanFor)    // If it's occupied
                {
                    return false;
                }
            }
        }
        for (int i = -1; i < boat.len + 1; i++) // Check line on the bottom of the Boat
        {
            if ((boat.x + i >= 0) && (boat.y + 1 <= 9) && (boat.x + i <= 9))    // If it fits
            {
                if (ground[boat.x + i][boat.y + 1] == scanFor)    // If it's occupied
                {
                    return false;
                }
            }
        }

        for (size_t i = 0; i < boat.len; i++)   // Check line of the Boat itself
        {
            if (ground[boat.x + i][boat.y] == scanFor)    // If it's occupied
            {
                return false;
            }
        }

        if (boat.x >= 1) // If there's no field border on the left
        {
            if (ground[boat.x - 1][boat.y] == scanFor)    // If it's occupied
            {
                return false;
            }
        }
        if (boat.x + boat.len <= 10) // If there's no field border on the right
        {
            if (ground[boat.x + boat.len][boat.y] == scanFor) // If it's occupied
            {
                return false;
            }
        }
    }
    else    // If it's vertical
    {
        for (int i = -1; i < boat.len + 1; i++) // Check line on the left of the Boat
        {
            if ((boat.y + i >= 0) && (boat.x - 1 >= 0) && (boat.y + i <= 9))    // If it fits
            {
                if (ground[boat.x - 1][boat.y + i] == scanFor)    // If it's occupied
                {
                    return false;
                }
            }
        }
        for (int i = -1; i < boat.len + 1; i++) // Check line on the right of the Boat
        {
            if ((boat.y + i >= 0) && (boat.x + 1 <= 9) && (boat.y + i <= 9))    // If it fits
            {
                if (ground[boat.x + 1][boat.y + i] == scanFor)    // If it's occupied
                {
                    return false;
                }
            }
        }

        for (size_t i = 0; i < boat.len; i++)   // Check line of the Boat itself
        {
            if (ground[boat.x][boat.y + i] == scanFor)    // If it's occupied
            {
                return false;
            }
        }

        if (boat.y >= 1) // If there's no field border on the top
        {
            if (ground[boat.x][boat.y - 1] == scanFor)    // If it's occupied
            {
                return false;
            }
        }
        if (boat.y + boat.len <= 10) // If there's no field border on the bottom
        {
            if (ground[boat.x][boat.y + boat.len] == scanFor) // If it's occupied
            {
                return false;
            }
        }
    }

    return true;
}

// Detect where's the free place for the Boat (not random)
int* detectFreeSpace(int ground[10][10], Boat boat)
{
    int* ret = new int[3];

    boat.isHorizontal = true;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            boat.x = j;
            boat.y = i;
            if (canPlaceHere(ground, boat))
            {
                ret[0] = boat.x;
                ret[1] = boat.y;
                ret[2] = boat.isHorizontal;
                return ret;
            }
        }
    }

    boat.isHorizontal = false;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            boat.x = j;
            boat.y = i;
            if (canPlaceHere(ground, boat))
            {
                ret[0] = boat.x;
                ret[1] = boat.y;
                ret[2] = boat.isHorizontal;
                return ret;
            }
        }
    }

    return 0;
}

void fieldConstructor(int ground[10][10], Boat boats[10])
{
    Boat cur_boat, old_boat;
    bool key;
    int cur_id = 0, button;
    int* params = new int[3];

    clearField(ground);
    clearBoats(boats);

    setc();
    system("cls");
    drawBorder();

    setc();
    setp(0, 13);
    cout << "Use WASD to move the boat.\nUse Q to rotate it.\nUse Enter to place it.";

    do
    {
        if (cur_id == 0)
        {
            cur_boat.len = 4;
        }
        else if ((cur_id == 1) || (cur_id == 2))
        {
            cur_boat.len = 3;
        }
        else if ((cur_id >= 3) && (cur_id <= 5))
        {
            cur_boat.len = 2;
        }
        else
        {
            cur_boat.len = 1;
        }

        params = detectFreeSpace(ground, cur_boat);
        cur_boat.x = params[0];
        cur_boat.y = params[1];
        cur_boat.isHorizontal = params[2];

        copyBoat(cur_boat, old_boat);

        drawField(ground);

        do
        {
            key = true;

            showBoat(old_boat, ground);
            showBoat(cur_boat);
            copyBoat(cur_boat, old_boat);

            button = _getch();

            setc(Black);
            setp(0, 17);
            cout << "You can't place a boat here!";

            switch (button)
            {
            case 119:   //W
                cur_boat.y--;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 97:    //A
                cur_boat.x--;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 115:   //S
                cur_boat.y++;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 100:   //D
                cur_boat.x++;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 113:   //Q
                cur_boat.isHorizontal = !cur_boat.isHorizontal;
                if (!inBorders(cur_boat))
                {
                    copyBoat(old_boat, cur_boat);
                }
                break;
            case 13:    //Enter
                if (canPlaceHere(ground, cur_boat))
                {
                    key = false;
                }
                else
                {
                    setc(White);
                    setp(0, 17);
                    cout << "You can't place a boat here!";
                }
            }
        } while (key);

        addBoat(ground, cur_boat, boats, cur_id);
        cur_id++;

    } while (cur_id < 10);

    delete[] params;
}

// Automatic field construction by the bot
void autoFieldConstructor(int ground[10][10], Boat boats[10])
{
    Boat cur_boat;

    clearField(ground);
    clearBoats(boats);

    for (size_t i = 0; i < 10; i++)
    {
        if (i == 0)
        {
            cur_boat.len = 4;
        }
        else if ((i == 1) || (i == 2))
        {
            cur_boat.len = 3;
        }
        else if ((i >= 3) && (i <= 5))
        {
            cur_boat.len = 2;
        }
        else
        {
            cur_boat.len = 1;
        }

        do
        {
            cur_boat.x = rand() % 10;
            cur_boat.y = rand() % 10;
            cur_boat.isHorizontal = rand() % 2;
            Sleep(200);
        } while (!canPlaceHere(ground, cur_boat));

        addBoat(ground, cur_boat, boats, i);

        Sleep(200);
        cout << '.';
    }
}

// Give user a control and return coordinates of the tile where he pointed
int* aim(int ground[10][10], int x, int y, int defx = 0, int defy = 0)
{
    int button, xtable, ytable, oldx = 0, oldy = 0;
    bool key;
    int* coords = new int[2];
    coords[0] = defx;
    coords[1] = defy;

    if (y >= 2) { ytable = y - 2; }
    else { ytable = y + 11; }

    if (x >= 1) { xtable = x - 1; }
    else { xtable = x; }

    setc();
    setp(xtable, ytable + 16);
    cout << "Use WASD to move the cursor.";
    setp(xtable, ytable + 17);
    cout << "Use Enter to shoot.";

    do
    {
        key = true;

        drawTile(ground[oldx][oldy], x + oldx, y + oldy, false);
        drawTile(Blue, x + coords[0], y + coords[1]);

        oldx = coords[0];
        oldy = coords[1];

        button = _getch();

        setc(Black);
        setp(xtable, ytable);
        cout << "You already tried to shoot here!";

        switch (button)
        {
        case 119:   //W
            if (coords[1] > 0)
            {
                coords[1]--;
            }
            else
            {
                coords[1] = 0;
            }
            break;

        case 97:    //A
            if (coords[0] > 0)
            {
                coords[0]--;
            }
            else
            {
                coords[0] = 0;
            }
            break;

        case 115:   //S
            if (coords[1] < 9)
            {
                coords[1]++;
            }
            else
            {
                coords[1] = 9;
            }
            break;

        case 100:   //D
            if (coords[0] < 9)
            {
                coords[0]++;
            }
            else
            {
                coords[0] = 9;
            }
            break;

        case 13:    //Enter
            if (ground[coords[0]][coords[1]] >= 2)
            {
                setc();
                setp(xtable, ytable);
                cout << "You already tried to shoot here!";
            }
            else
            {
                key = false;
            }
            break;
        }

    } while(key);

    setc(Black);
    setp(xtable, ytable + 16);
    cout << "Use WASD to move the cursor.";
    setp(xtable, ytable + 17);
    cout << "Use Enter to shoot.";

    return coords;
}

// Delete one Boat from the Boats array and mark it as fragged on the field
void fragged(int ground[10][10], Boat boats[10], int element, int& boat_count)
{
    for (size_t i = 0; i < boats[element].len; i++)
    {
        if (boats[element].isHorizontal)
        {
            ground[boats[element].x + i][boats[element].y] = 4;
        }
        else
        {
            ground[boats[element].x][boats[element].y + i] = 4;
        }
    }
    boat_count--;
}

// Check all the boats on the field and call fragged() if it's destroyed
void checkFrags(int ground[10][10], Boat boats[10], int& boat_count)
{
    bool key;

    for (size_t i = 0; i < 10; i++)
    {
        key = true;

        for (size_t j = 0; j < boats[i].len; j++)
        {
            if (boats[i].isHorizontal)
            {
                if (ground[boats[i].x + j][boats[i].y] != 3)
                {
                    key = false;
                }
            }
            else
            {
                if (ground[boats[i].x][boats[i].y + j] != 3)
                {
                    key = false;
                }
            }
        }

        if (key)
        {
            fragged(ground, boats, i, boat_count);
        }
    }
}

// If there's a Bot intelligence struct in the params, clear it when Boat is found
void checkFrags(int ground[10][10], Boat boats[10], int& boat_count, Bot& bot)
{
    bool key;

    for (size_t i = 0; i < 10; i++)
    {
        key = true;

        for (size_t j = 0; j < boats[i].len; j++)
        {
            if (boats[i].isHorizontal)
            {
                if (ground[boats[i].x + j][boats[i].y] != 3)
                {
                    key = false;
                }
            }
            else
            {
                if (ground[boats[i].x][boats[i].y + j] != 3)
                {
                    key = false;
                }
            }
        }

        if (key)
        {
            fragged(ground, boats, i, boat_count);
            clearBot(bot);  // Clear Bot intelligence struct
        }
    }
}

// Used for decreasing code (used in botAim())
void forwardingx(int ground[10][10], int& x, int y, int updx, int& stats)
{
    Boat check;
    check.isHorizontal = true;
    check.len = 1;
    do
    {
        x += updx;
        if ((x >= 0) && (x <= 9))
        {
            check.x = x;
            check.y = y;
            if ((ground[x][y] < 2) && (canPlaceHere(ground, check, 4)))
            {
                stats = 2;
            }
            else if ((ground[x][y] != 3) || (!canPlaceHere(ground, check, 4)))
            {
                stats = 1;
            }
        }
        else
        {
            stats = 1;
        }
    } while (stats == 0);
}

// Used for decreasing code (used in botAim())
void forwardingy(int ground[10][10], int x, int& y, int updy, int& stats)
{
    Boat check;
    check.isHorizontal = true;
    check.len = 1;
    do
    {
        y += updy;
        if ((y >= 0) && (y <= 9))
        {
            check.x = x;
            check.y = y;
            if ((ground[x][y] < 2) && (canPlaceHere(ground, check, 4)))
            {
                stats = 2;
            }
            else if ((ground[x][y] != 3) || (!canPlaceHere(ground, check, 4)))
            {
                stats = 1;
            }
        }
        else
        {
            stats = 1;
        }
    } while (stats == 0);
}

// Return a point where Bot wants to shoot
int* botAim(int ground[10][10], Bot& bot)
{
    int* coords = new int[2];
    int temp_dir, stats;    // for stats: 0 - need to continue, 1 - no sense to continue, to the other side, 2 - need to shoot
    bool key;
    Boat check; // for borders check (function canPlaceHere())

    Sleep(500);

    if (bot.remember)
    {
        coords[0] = bot.x;
        coords[1] = bot.y;

        if (bot.dir == 0)
        {
            key = true;
            do
            {
                temp_dir = rand() % 4;
                switch (temp_dir)
                {
                case 0:     // UP = Y-
                    if (bot.y > 0)  // If there's no field border on the top
                    {
                        if (ground[bot.x][bot.y - 1] < 2)   // If we don't know what's here
                        {
                            key = false;
                        }
                    }
                    break;

                case 1:     // RIGHT = X+
                    if (bot.x < 9)  // If there's no field border on the right
                    {
                        if (ground[bot.x + 1][bot.y] < 2)   // If we don't know what's here
                        {
                            key = false;
                        }
                    }
                    break;

                case 2:     // DOWN = Y+
                    if (bot.y < 9)  // If there's no field border on the bottom
                    {
                        if (ground[bot.x][bot.y + 1] < 2)   // If we don't know what's here
                        {
                            key = false;
                        }
                    }
                    break;

                default:    // LEFT = X-
                    if (bot.x > 0)  // If there's no field border on the left
                    {
                        if (ground[bot.x - 1][bot.y] < 2)   // If we don't know what's here
                        {
                            key = false;
                        }
                    }
                    break;
                }
            } while (key);

            switch (temp_dir)
            {
            case 0:
                if (ground[coords[0]][--coords[1]] == 1)
                {
                    bot.dir = 2;
                }
                break;

            case 1:
                if (ground[++coords[0]][coords[1]] == 1)
                {
                    bot.dir = 1;
                }
                break;

            case 2:
                if (ground[coords[0]][++coords[1]] == 1)
                {
                    bot.dir = 2;
                }
                break;

            default:
                if (ground[--coords[0]][coords[1]] == 1)
                {
                    bot.dir = 1;
                }
                break;
            }
            return coords;
        }

        else if (bot.dir == 1)
        {
            temp_dir = rand() % 2; // Bot will randomly choose the direction of the killing (just4fun)
            stats = 0;
            if (temp_dir) // Will start to the left
            {
                forwardingx(ground, coords[0], coords[1], -1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingx(ground, coords[0], coords[1], 1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
            else    // To the right
            {
                forwardingx(ground, coords[0], coords[1], 1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingx(ground, coords[0], coords[1], -1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
        }

        else
        {
            temp_dir = rand() % 2; // Bot will randomly choose the direction of the killing (just4fun)
            stats = 0;
            if (temp_dir) // Will start to the top
            {
                forwardingy(ground, coords[0], coords[1], -1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingy(ground, coords[0], coords[1], 1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
            else    // To the bottom
            {
                forwardingy(ground, coords[0], coords[1], 1, stats);

                if (stats == 2)
                {
                    return coords;
                }
                else
                {
                    stats = 0;
                    forwardingy(ground, coords[0], coords[1], -1, stats);

                    if (stats == 2)
                    {
                        return coords;
                    }
                }
            }
        }
    }
    else
    {
        check.isHorizontal = true;
        check.len = 1;
        do
        {
            coords[0] = rand() % 10;
            coords[1] = rand() % 10;
            check.x = coords[0];
            check.y = coords[1];
        } while ((ground[coords[0]][coords[1]] >= 2) || (!canPlaceHere(ground, check, 4)));
    }

    if (ground[coords[0]][coords[1]] == 1)
    {
        bot.remember = true;
        bot.x = coords[0];
        bot.y = coords[1];
    }

    return coords;
}

// Main function to play with Bot
// Don't mispell INJURED and DESTROYED
void gameBot()
{
    int ground_p1[10][10], ground_p2[10][10];   //0 - untouched empty tile, 1 - untouched tile with Boat, 2 - injured empty tile, 3 - tile with injured Boat, 4 - tile with destroyed Boat (doesn't affect program logic - only visual)
    int boats_count_p1 = 10;
    int boats_count_p2 = 10;
    Boat boats_p1[10];
    Boat boats_p2[10];

    bool cancel;
    char* yes_no[] = {"Yes", "No"};

    int* aim_coords_p1 = new int[2];
    aim_coords_p1[0] = 0;
    aim_coords_p1[1] = 0;
    int* aim_coords_p2 = new int[2];
    aim_coords_p2[0] = 0;
    aim_coords_p2[1] = 0;

    Bot bot;
    clearBot(bot);

    do
    {
        fieldConstructor(ground_p1, boats_p1);

        setc();
        system("cls");

        drawBorder();
        drawField(ground_p1);

        setc();
        setp(15);
        cout << "Do you want to start the game with this field?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    setc();
    system("cls");

    setp(3, 3);
    cout << "Ok, wait until bot will build their field";

    autoFieldConstructor(ground_p2, boats_p2);

    system("cls");

    drawBorder(3, 3);
    drawField(ground_p1, 4, 4, false);
    setc();
    setp(3, 15);
    cout << "FIELD OF PLAYER #1";
    setp(3, 16);
    cout << "(You)";

    drawBorder(25, 3);
    drawField(ground_p2, 26, 4, false);
    setc();
    setp(25, 15);
    cout << "FIELD OF PLAYER #2";
    setp(25, 16);
    cout << "(Bot)";

    do  // Game will go until all the Boats of some player are destroyed
    {
        do  // Player will play until he miss
        {
            aim_coords_p1 = aim(ground_p2, 26, 4, aim_coords_p1[0], aim_coords_p1[1]);  // Player aims
            if (ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] == 0) // If there's nothing
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 2;      // Show miss
            }
            else                                                    // Else
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 3;      // Show Boat injury
            }

            checkFrags(ground_p2, boats_p2, boats_count_p2);

            drawField(ground_p2, 26, 4, false);   // Update field

        } while ((ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] != 2) && (boats_count_p2 > 0));

        if (boats_count_p2 > 0)
        {
            do  // Bot will play until they miss
            {
                aim_coords_p2 = botAim(ground_p1, bot);   // Bot aims
                if (ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] == 0) // If there's nothing
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 2;      // Show miss
                }
                else                                                    // Else
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 3;      // Show Boat injury
                }

                checkFrags(ground_p1, boats_p1, boats_count_p1, bot);

                drawField(ground_p1, 4, 4, false);    // Update field

            } while ((ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] != 2) && (boats_count_p1 > 0));
        }
    } while ((boats_count_p1 > 0) && (boats_count_p2 > 0));

    setc();
    system("cls");

    drawBorder(3, 3);
    drawField(ground_p1, 4, 4);
    setc();
    setp(3, 15);
    cout << "FIELD OF PLAYER #1";

    drawBorder(25, 3);
    drawField(ground_p2, 26, 4);
    setc();
    setp(25, 15);
    cout << "FIELD OF PLAYER #2";

    if (boats_count_p1 < 1)
    {
        setp(3, 16);
        cout << "Lose...";
        setp(25, 16);
        cout << "Win!";
    }
    else
    {
        setp(3, 16);
        cout << "Win!";
        setp(25, 16);
        cout << "Lose...";
    }

    delete[] yes_no;
    delete[] aim_coords_p1;
    delete[] aim_coords_p2;
}

// Main cycle of the PvP game
void gameLocal()
{
    int ground_p1[10][10], ground_p2[10][10];
    int boats_count_p1 = 10;
    int boats_count_p2 = 10;
    Boat boats_p1[10];
    Boat boats_p2[10];

    bool cancel;
    char* yes_no[] = {"Yes", "No"};

    int* aim_coords_p1 = new int[2];
    aim_coords_p1[0] = 0;
    aim_coords_p1[1] = 0;
    int* aim_coords_p2 = new int[2];
    aim_coords_p2[0] = 0;
    aim_coords_p2[1] = 0;

    do
    {
        setc();
        system("cls");
        setp();
        cout << "Player #1 please come to the computer, \nPlayer #2 please don't spy on Player #1.";
        system("pause>>NULL");

        fieldConstructor(ground_p1, boats_p1);

        setc();
        system("cls");

        drawBorder();
        drawField(ground_p1);

        setc();
        setp(15);
        cout << "Do you want to start the game with this field?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    do
    {
        setc();
        system("cls");
        setp();
        cout << "Player #2 please come to the computer, \nPlayer #1 please don't spy on Player #2.";
        system("pause>>NULL");

        fieldConstructor(ground_p2, boats_p2);

        setc();
        system("cls");

        drawBorder();
        drawField(ground_p2);

        setc();
        setp(15);
        cout << "Do you want to start the game with this field?";
        cancel = (bool)menu(yes_no, 2, 15, 2);
    } while (cancel);

    setc();
    system("cls");

    drawBorder(3, 3);
    drawField(ground_p1, 4, 4, false);
    setc();
    setp(3, 15);
    cout << "FIELD OF PLAYER #1";

    drawBorder(25, 3);
    drawField(ground_p2, 26, 4, false);
    setc();
    setp(25, 15);
    cout << "FIELD OF PLAYER #2";

    // Ah, old me, what you're doing?! Look, you've duplicated this huge code and I need to translate it AGAIN!!! (ctrl+c,v are gonna break)

    do  // Game will go until all the Boats of some player are destroyed
    {
        do  // Player #1 will play until they miss
        {
            aim_coords_p1 = aim(ground_p2, 26, 4, aim_coords_p1[0], aim_coords_p1[1]);  // Player #1 aims
            if (ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] == 0) // If there's nothing
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 2;      // Show miss
            }
            else                                                    // Else
            {
                ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] = 3;      // Show Boat injury
            }

            checkFrags(ground_p2, boats_p2, boats_count_p2);    

            drawField(ground_p2, 26, 4, false);     // Update field

        } while ((ground_p2[aim_coords_p1[0]][aim_coords_p1[1]] != 2) && (boats_count_p2 > 0));

        if (boats_count_p2 > 0)
        {
            do  // Player #2 will play until they miss
            {
                aim_coords_p2 = aim(ground_p1, 4, 4, aim_coords_p2[0], aim_coords_p2[1]);   // Player #2 aims
                if (ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] == 0) // If there's nothing
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 2;      // Show miss
                }
                else                                                    // Else
                {
                    ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] = 3;      // Show Boat injury
                }

                checkFrags(ground_p1, boats_p1, boats_count_p1);

                drawField(ground_p1, 4, 4, false);    // Update field

            } while ((ground_p1[aim_coords_p2[0]][aim_coords_p2[1]] != 2) && (boats_count_p1 > 0));
        }
    } while ((boats_count_p1 > 0) && (boats_count_p2 > 0));

    setc();
    system("cls");

    drawBorder(3, 3);
    drawField(ground_p1, 4, 4);
    setc();
    setp(3, 15);
    cout << "FIELD OF PLAYER #1";

    drawBorder(25, 3);
    drawField(ground_p2, 26, 4);
    setc();
    setp(25, 15);
    cout << "FIELD OF PLAYER #2";

    if (boats_count_p1 < 1)
    {
        setp(3, 16);
        cout << "Lose...";
        setp(25, 16);
        cout << "Win!";
    }
    else
    {
        setp(3, 16);
        cout << "Win!";
        setp(25, 16);
        cout << "Lose...";
    }

    delete[] yes_no;
    delete[] aim_coords_p1;
    delete[] aim_coords_p2;
}

int main()
{
    srand(time(NULL));

    char* menu_choises[] = {"Singleplayer", "Local multiplayer", "Exit"};
    const unsigned int menu_len = 3;
    int choised;

    setc();
    system("cls");
    choised = menu(menu_choises, menu_len);
    setc();
    system("cls");

    switch (choised)
    {
    case 0:
        gameBot();
        break;

    case 1:
        gameLocal();
        break;

    default:
        setp();
        cout << "Bye.\n";
        break;
    }

    delete[] menu_choises;

    system("pause>>NULL");
}
