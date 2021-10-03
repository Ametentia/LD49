#include <stdio.h>

function u8 CountNeighbours(Tile (*tiles)[WORLD_Y_SIZE], u8 x, u8 y) {
    u8 count = 0;
    for(u8 i = 0; i < 3; i++) {
        for(u8 j = 0; j < 3; j++) {
            if(j==1 && i==1)
                continue;
            u8 x_safe = x+(i-1) >= 0 && x+(i-1) < WORLD_X_SIZE;
            u8 y_safe = y+(j-1) >= 0 && y+(j-1) < WORLD_Y_SIZE;
            if(x_safe && y_safe) {
                if(tiles[x+(i-1)][y+(j-1)].alive)
                    count++; 
            }
        }
    }
    return count;
}

function void SimGeneration(Tile (*tiles)[WORLD_Y_SIZE], u8 generations) {
    for(int i = 0; i < generations; i++) {
        for(int x = 2; x < WORLD_X_SIZE-3; x++) {
            for(int y = 2; y < WORLD_Y_SIZE-3; y++) {
                u8 count = CountNeighbours(tiles, x, y);
                if(tiles[x][y].alive) {
                    tiles[x][y].alive = count > 3;
                } else {
                    tiles[x][y].alive = count > 5;
                }
            }
        }
    }
}

function void SpawnPopulation(Tile (*tiles)[WORLD_Y_SIZE], Random *random) {
    for(int i = 0; i < WORLD_X_SIZE; i++) {
        for(int j = 0; j < WORLD_Y_SIZE; j++) {
            tiles[i][j].alive = (u8)RandomU32(random, 0, 9) > 3;
            if(i < 2 || i > WORLD_X_SIZE-2 || j<2 || j>WORLD_Y_SIZE-2) {
                tiles[i][j].alive = 1;    
            }
        }
    }
}
