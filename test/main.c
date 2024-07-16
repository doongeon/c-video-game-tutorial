
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "circular_queue.h"
#include "slime.h"
#include "weapon.h"
#include "player.h"

Image GenerateGrassTexture(int width, int height)
{
    Image image = GenImageColor(width, height, DARKGREEN);

    for (int y = 0; y < height; y += 2)
    {
        for (int x = 0; x < width; x += 2)
        {
            Color color = (GetRandomValue(0, 1) == 0) ? GREEN : DARKGREEN;
            ImageDrawPixel(&image, x, y, color);
        }
    }

    return image;
}

Image GenerateUnderGroundTexture(int width, int height)
{
    Image image = GenImageColor(width, height, DARKBROWN);

    for (int y = 0; y < height; y += 2)
    {
        for (int x = 0; x < width; x += 2)
        {
            Color color = (GetRandomValue(0, 1) == 0) ? DARKBROWN : BROWN;
            ImageDrawPixel(&image, x, y, color);
        }
    }

    return image;
}

// Game assets
// ----------------------------------------------------------------------------------
typedef struct DamageGUI
{
    Vector2 position;
    int frameCounter;
    int hitFrameCounter;
} DamageGUI;

DamageGUI createDamageGUI(Slime *slime)
{
    DamageGUI damageGUI = {slime->position, 0, 0};
    return damageGUI;
}

typedef struct EnvItem
{
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;
// ----------------------------------------------------------------------------------

int main(void)
{
    // Initialization
    //------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "animation tutorial");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D scarfy = LoadTexture(TEXTURE_FILE_PATH); // Texture loading

    if (scarfy.id == 0)
    {
        printf("Failed to load texture\n");
        return -1; // Or handle the error as appropriate
    }

    Player player = createPlayer();

    Slime slime = createSlime();

    EnvItem envItems[] = {
        {{0, 0, screenWidth, screenHeight}, 0, SKYBLUE},
        {{0, GROUND_Y_POSITION, 1000, 200}, 1, GRAY},
        {{screenWidth / 2, GROUND_Y_POSITION - 30, 100, 30}, 1, GRAY}};
    int envItemsLength = sizeof(envItems) / sizeof(EnvItem);

    CircularQueue damageQue;
    initQueue(&damageQue);

    Image grassImage = GenerateGrassTexture(100, 10);
    Texture2D grassTexture = LoadTextureFromImage(grassImage);
    UnloadImage(grassImage);

    Image underGroundImage = GenerateUnderGroundTexture(10, 10);
    Texture2D underGroundTexture = LoadTextureFromImage(underGroundImage);
    UnloadImage(underGroundImage);

    SetTargetFPS(60);
    //------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //--------------------------------------------------------------------------
        bool hitObstacle = false;

        for (int i = 0; i < envItemsLength; i++)
        {
            EnvItem *ei = envItems + i;
            Player *p = &player;
            if (
                ei->blocking &&
                ei->rect.x <= p->position.x + abs((int)(p->frameRec.width)) - 30 &&
                ei->rect.x + ei->rect.width >= p->position.x + 30 &&
                ei->rect.y >= p->position.y + p->frameRec.height - 3 &&
                ei->rect.y <= p->position.y + p->frameRec.height - 3 + player.vMoveVector)
            {
                hitObstacle = true;
                if (hitObstacle)
                    break;
            }
        }
        if (hitObstacle)
        {
            player.vMoveVector = 0;
            player.jumpState = false;
        }
        else
        {
            player.vMoveVector += 1;
            player.jumpState = true;
        }

        updatePlayerPosition(&player);

        if (IsKeyDown(KEY_A))
        {
            setAttackState(&player);
            attack(&player, &slime);
        }
        if (IsKeyDown(KEY_D))
        {
            if (!player.jumpState && !player.attackState)
                player.vMoveVector = -10;
            setJumpState(&player);
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            if (!player.attackState && !player.jumpState && !player.moveLeftState)
            {
                setMoveRightState(&player);
                player.hMoveVector += 0.5;
            }
            if (!player.attackState && player.jumpState && !player.moveLeftState)
            {
                setMoveRightState(&player);
                player.hMoveVector += 0.2;
            }
            if (player.hMoveVector > 2)
            {
                player.hMoveVector = 2;
            }
        }
        if (IsKeyUp(KEY_RIGHT))
        {
            player.moveRightState = false;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            if (!player.attackState && !player.jumpState && !player.moveRightState)
            {
                setMoveLeftState(&player);
                player.hMoveVector -= 0.5;
            }
            if (!player.attackState && player.jumpState && !player.moveRightState)
            {
                setMoveLeftState(&player);
                player.hMoveVector -= 0.2;
            }
            if (player.hMoveVector < -2)
            {
                player.hMoveVector = -2;
            }
        }
        if (IsKeyUp(KEY_LEFT))
        {
            player.moveLeftState = false;
        }

        if (!player.moveLeftState && !player.moveRightState && !player.jumpState)
        {
            if (player.hMoveVector > 0)
            {
                player.hMoveVector -= 0.5;
                if (player.hMoveVector < 0)
                {
                    player.hMoveVector = 0;
                }
            }

            if (player.hMoveVector < 0)
            {
                player.hMoveVector += 0.5;
                if (player.hMoveVector > 0)
                {
                    player.hMoveVector = 0;
                }
            }
        }

        // Set animating frame
        //

        // Player
        //
        player.frameCounter++;
        if (player.attackState && player.frameCounter >= (60 / 6))
        {
            // 공격하는 프레임 설정 ( 6 FPS )
            //
            player.frameCounter = 0;

            player.frameRec.x = (float)player.attackFrameCounter * (float)scarfy.width / 6;
            player.frameRec.y = scarfy.height / 8 * 3;

            player.attackFrameCounter++;

            if (!player.jumpState)
                player.hMoveVector = 0;

            if (player.attackFrameCounter >= NUM_ATTACK_FRAME)
            {
                player.attackFrameCounter = 0;
                player.frameCounter = 30;
                player.attackState = false;
            }
        }
        else if (player.moveRightState && player.frameCounter >= (60 / 8))
        {
            // 오른쪽으로 이동하는 프레임 설정 ( 8 FPS )
            //
            player.frameCounter = 0;

            player.frameRec.x = (float)player.moveRightFrameCounter * (float)scarfy.width / 6;
            player.frameRec.y = scarfy.height / 8 * 0;

            if (player.frameRec.width < 0)
            {
                // 캐릭터가 오른쪽을 보도록 지정
                //
                player.frameRec.width *= -1;
            }

            player.moveRightFrameCounter++;
            if (player.moveRightFrameCounter > 5)
            {
                player.moveRightFrameCounter = 0;
                player.frameCounter = 30;
            }
        }
        else if (player.moveLeftState && player.frameCounter >= (60 / 8))
        {
            // 왼쪽으로 이동하는 프레임임 설정 ( 8 FPS )
            //
            player.frameCounter = 0;
            player.frameRec.x = (float)player.moveLeftFrameCounter * (float)scarfy.width / 6;
            player.frameRec.y = scarfy.height / 8 * 0;
            if (player.frameRec.width > 0)
            {
                // 캐릭터가 왼쪽을 보도록 지정
                //
                player.frameRec.width *= -1;
            }
            player.moveLeftFrameCounter++;
            if (player.moveLeftFrameCounter > 5)
            {
                player.moveLeftFrameCounter = 0;
                player.frameCounter = 30;
            }
        }
        else if (player.frameCounter >= (60 / 2))
        {
            // 서있는 프레임 설정 ( 2FPS )
            //
            player.frameCounter = 0;

            player.frameRec.x = (float)player.standingFrameCounter * (float)scarfy.width / 6;
            player.frameRec.y = scarfy.height / 8 * 1;

            player.standingFrameCounter++;
            if (player.standingFrameCounter >= NUM_STAND_FRAME)
            {
                player.standingFrameCounter = 0;
            }
        }
        //

        // slime
        //
        slime.frameCounter++;
        if (slime.frameCounter > 8)
        {
            slime.frameCounter = 0;

            if (slime.hittedState)
            {
                slime.hitFrameCounter++;
                if (slime.hitFrameCounter > 5)
                {
                    slime.hittedState = false;
                    slime.hitFrameCounter = 0;
                }
            }
        }
        //

        //--------------------------------------------------------------------------

        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();
        {
            ClearBackground(WHITE);

            // map
            //
            for (int i = 0; i < envItemsLength; i++) // 블럭
                DrawRectangleRec(envItems[i].rect, envItems[i].color);

            for ( // 땅 흙 텍스쳐
                int x = 0;
                x < screenWidth;
                x += underGroundTexture.width)
            {
                for (
                    int y = GROUND_Y_POSITION + grassTexture.height;
                    y < screenHeight;
                    y += underGroundTexture.height)
                {
                    DrawTexture(underGroundTexture, x, y, WHITE);
                }
            }
            for ( // 땅 잔디 텍스쳐
                int x = 0;
                x < screenWidth;
                x += grassTexture.width)
            {
                for (int y = GROUND_Y_POSITION; y < GROUND_Y_POSITION + 10; y += grassTexture.height)
                {
                    DrawTexture(grassTexture, x, y, WHITE);
                }
            }

            for ( // 언덕 흙 텍스쳐
                int x = envItems[2].rect.x;
                x < envItems[2].rect.x + envItems[2].rect.width;
                x += underGroundTexture.width)
            {
                for (
                    int y = envItems[2].rect.y + grassTexture.height;
                    y < envItems[2].rect.y + envItems[2].rect.height;
                    y += underGroundTexture.height)
                {
                    DrawTexture(underGroundTexture, x, y, WHITE);
                }
            }
            for ( // 언덕 잔지 텍스쳐
                int x = envItems[2].rect.x;
                x < envItems[2].rect.x + envItems[2].rect.width;
                x += grassTexture.width)
            {
                for (
                    int y = envItems[2].rect.y;
                    y < envItems[2].rect.y + 10;
                    y += grassTexture.height)
                {
                    DrawTexture(grassTexture, x, y, WHITE);
                }
            }
            //

            // 슬라임
            //
            drawSlime(slime);
            if (slime.hittedState)
            {
                DrawText("hit", slime.position.x, slime.position.y - 20 - slime.hitFrameCounter * 3, 20, RED);
            }

            // 플레이어
            //
            drawPlayer(scarfy, player);
            drawPlayerRec(player);
            drawPlayerWeaponRange(player);
            //
        }
        EndDrawing();
        //--------------------------------------------------------------------------
    }

    // De-Initialization
    //------------------------------------------------------------------------------
    UnloadTexture(scarfy);

    CloseWindow();
    //------------------------------------------------------------------------------

    return 0;
}