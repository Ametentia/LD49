#if !defined(LUDUM_WORLD_GEN_H_)
#define LUDUM_WORLD_GEN_H_

function void SimGeneration(Tile (*tiles)[WORLD_Y_SIZE], u8 generations);
function void SpawnPopulation(Tile (*tiles)[WORLD_Y_SIZE], Random random);

#endif  // LUDUM_WORLD_PLAY_H_
