#include <stdio.h>

function void AddDoor(Tile *tiles, u8 exit = 1) {
    u32 exit_x = WORLD_X_SIZE - 8;
    u32 exit_y = 8;
    u32 exit_height = 3;
    u32 exit_width = 2;
    Tile_Type type = Tile_Exit;
    if(!exit) {
        exit_x = 4;
        exit_y = WORLD_Y_SIZE - 5;
        type = Tile_Entrance;
    }
    for(u32 x = 0; x < exit_width; x++) {
        for(u32 y = 0; y < exit_height; y++) {
            Tile *tile = &tiles[((exit_y+y) * WORLD_Y_SIZE) + exit_x+x];
            tile->type = type;
        }
    }
    if(exit) {
        for(u32 i = 0; i < 4; i++) {
            Tile *tile = &tiles[((exit_y+exit_height) * WORLD_Y_SIZE) + exit_x+exit_width-3+i];
            tile->type = Tile_Ground;
        }
    }
}

function u8 CountNeighbours(Tile *tiles, u8 x, u8 y) {
    u8 count = 0;
    for(u8 i = 0; i < 3; i++) {
        for(u8 j = 0; j < 3; j++) {
            if(j==1 && i==1)
                continue;
            u8 x_safe = x+i-1 >= 0 && x+i-1 < WORLD_X_SIZE;
            u8 y_safe = y+j-1 >= 0 && y+j-1 < WORLD_Y_SIZE;
            if(x_safe && y_safe) {
                if(tiles[((y+j-1) * WORLD_Y_SIZE) + x+i-1].type != Tile_Air)
                    count++; 
            }
        }
    }
    return count;
}

function void SimGeneration(Tile *tiles, u8 generations, u8 add_enterance = 1) {
    for(int i = 0; i < generations; i++) {
        for(int x = 2; x < WORLD_X_SIZE-3; x++) {
            for(int y = 2; y < WORLD_Y_SIZE-3; y++) {
                u8 count = CountNeighbours(tiles, x, y);
                Tile *tile = &tiles[(y * WORLD_Y_SIZE) + x];
                if(tile->type != Tile_Air) {
                    if(count > 2) {
                        tile->type = Tile_Ground;
                    } else {
                        tile->type = Tile_Air;
                    }
                } else if(count > 5){
                    tile->type = Tile_Ground;
                } else {
                    tile->type = Tile_Air;
                }
            }
        }
    }
    AddDoor(tiles);
    if(add_enterance)
        AddDoor(tiles, 0);
}

function void SpawnPopulation(Tile *tiles, Random *random) {
    for(int i = 0; i < WORLD_X_SIZE; i++) {
        for(int j = 0; j < WORLD_Y_SIZE; j++) {
            Tile *tile = &tiles[(j * WORLD_Y_SIZE) + i];
            tile->grid_p = V2U(i, j);
            tile->type = Tile_Air;
            if(RandomU32(random, 0, 9) > 4) {
                tile->type = Tile_Ground;
            }
            if((i < 14 && j > WORLD_Y_SIZE - 14) || (j < 14 && i > WORLD_X_SIZE - 14)) {
                tile->type = Tile_Air;
            }
            if(i < 3 || i > WORLD_X_SIZE-3 || j<3 || j>WORLD_Y_SIZE-3) {
                tile->type = Tile_Ground;    
            }
        }
    }
}
