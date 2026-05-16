#ifndef ENEMY_H
#define ENEMY_H

#include "type.h"

void init_slime(Enemy *enemy);

int is_enemy_alive(const Enemy *enemy);

int calculate_enemy_attack_damage(const Enemy *enemy, const Player *player);
void enemy_attack_player(Enemy *enemy, Player *player);

void enemies_take_turn(Enemy enemies[], int enemy_count, Player *player);

#endif