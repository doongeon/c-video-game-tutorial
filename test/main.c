
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "circular_queue.h"
#include "slime.h"
#include "weapon.h"
#include "player.h"
#include "damage.h"
#include "linked_damage_list.h"
#include "env_item.h"
#include "texture.h"

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
    DamageNode *damageList = initializeList();
    Damage damage = (Damage){{0, 0}, 0, 0};
    Damage damages[255];

    EnvItem envItems[] = {
        createEnvItem(
            "background",
            (Vector2){0, 0},
            screenWidth / ENVITEM_WIDTH_UNIT + 1,
            screenHeight / ENVITEM_HEIGHT_UNIT + 1,
            0, 0
        ),
        createEnvItem(
            "ground",
            (Vector2){0, GROUND_Y_POSITION},
            screenWidth / ENVITEM_WIDTH_UNIT + 1,
            (screenHeight - GROUND_Y_POSITION) / ENVITEM_HEIGHT_UNIT + 1,
            0, 1
        ),
        createEnvItem(
            "hill",
            (Vector2){500, GROUND_Y_POSITION - ENVITEM_HEIGHT_UNIT},
            3,
            1,
            0, 1
        ),
        createEnvItem(
            "fence",
            (Vector2){300, GROUND_Y_POSITION - ENVITEM_HEIGHT_UNIT},
            1,
            2,
            1, 1
        ),
    };
    int envItemsLength = sizeof(envItems) / sizeof(EnvItem);

    Texture2D grassTexture = getGrassTexture();
    Texture2D dirtTexture = getDirtTexture();

    SetTargetFPS(60);
    //------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //--------------------------------------------------------------------------
        bool hitObstacleY = false;

        for (int i = 0; i < envItemsLength; i++)
        {
            EnvItem *ei = envItems + i;
            Player *p = &player;
            if (
                ei->blockY &&
                ei->rect.x <= p->position.x + abs((int)(p->frameRec.width)) - 30 &&
                ei->rect.x + ei->rect.width >= p->position.x + 30 &&
                ei->rect.y >= p->position.y + p->frameRec.height - 3 &&
                ei->rect.y <= p->position.y + p->frameRec.height - 3 + player.vMoveVector)
            {
                hitObstacleY = true;
                if (hitObstacleY)
                    break;
            }
        }
        if (hitObstacleY)
        {
            player.vMoveVector = 0;
            player.jumpState = false;
        }
        else
        {
            player.vMoveVector += 1; // 중력처럼 작용
            player.jumpState = true;
        }

        updatePlayerPosition(&player);

        removeExpiredNode(&damageList);

        if (IsKeyDown(KEY_A))
        {
            setAttackState(&player);
            attack(&player, &slime, &damageList);
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
        updatePlayerFrame(scarfy, &player); // Player
        //--------------------------------------------------------------------------

        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();
        {
            ClearBackground(SKYBLUE);

            // map
            //
            // for (int i = 0; i < envItemsLength; i++) // 블럭
            //     DrawRectangleRec(envItems[i].rect, envItems[i].color);

            drawGrassFieldTexture(envItems[1], grassTexture, dirtTexture);
            drawGrassFieldTexture(envItems[2], grassTexture, dirtTexture);
            drawGrassFieldTexture(envItems[3], grassTexture, dirtTexture);

            drawSlime(&slime); // 슬라임

            if (!isDamageListEmpty(&damageList)) // 데미지
            {
                drawDamages(&damageList);
            }

            drawPlayer(scarfy, player); // 플레이어
            // drawPlayerRec(player);
            // drawPlayerWeaponRange(player);
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