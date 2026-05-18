#ifndef MAP_H
#define MAP_H

#include "type.h"

int is_valid_floor(int floor);
int is_final_floor(int floor);

const MapFloor *get_map_floor(int floor);
StageType get_default_stage_type(int floor);
const char *get_stage_type_name(StageType stage);

#endif