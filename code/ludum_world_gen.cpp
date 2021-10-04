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
        Scratch_Memory scratch = GetScratch();
        Tile *new_tiles = AllocArray(scratch.arena, Tile, WORLD_X_SIZE*WORLD_Y_SIZE);
        CopySize(new_tiles, tiles, WORLD_X_SIZE*WORLD_Y_SIZE*sizeof(Tile));
        for(int x = 2; x < WORLD_X_SIZE-3; x++) {
            for(int y = 2; y < WORLD_Y_SIZE-3; y++) {
                u8 count = CountNeighbours(tiles, x, y);
                Tile *tile = &tiles[(y * WORLD_Y_SIZE) + x];
                Tile *new_tile = &new_tiles[(y * WORLD_Y_SIZE) + x];
                if(tile->type != Tile_Air) {
                    if(count > 2) {
                        new_tile->type = Tile_Ground;
                    } else {
                        new_tile->type = Tile_Air;
                    }
                } else if(count > 5){
                    new_tile->type = Tile_Ground;
                } else {
                    new_tile->type = Tile_Air;
                }
            }
        }
        CopySize(tiles, new_tiles, WORLD_X_SIZE*WORLD_Y_SIZE*sizeof(Tile));
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

function void GenerateWorld(Tile *tiles, Random *random, Asset_Manager *assets) {
    SpawnPopulation(tiles, random);
    SimGeneration(tiles, 15);

    str8 ground_tile_names[] = {
        WrapConst("ground_01"),
    };

    for (u32 y = 0; y < WORLD_Y_SIZE; ++y) {
        for (u32 x = 0; x < WORLD_X_SIZE; ++x) {
            Tile *tile  = &tiles[(y * WORLD_Y_SIZE) + x];
            if (tile->type == Tile_Air) { continue; }

            v2s tile_p  = V2S(x, y);
            v2s above_p = V2S(x, y - 1);

            u32 index = NextRandom(random) % ArraySize(ground_tile_names);
            str8 name = ground_tile_names[index];

            if (IsValidTile(above_p)) {
                Tile *above = &tiles[(above_p.y * WORLD_Y_SIZE) + above_p.x];

                if (above->type != Tile_Air) {
                    name = WrapConst("ground_under");
                }
            }

            tile->image = GetImageByName(assets, name);
        }
    }
}
