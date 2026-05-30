#include <stddef.h>
#include <stdio.h>
#include "map.h"

//전체 스테이지 목록
static const MapFloor fixed_map[MAX_FLOOR] = {
    {{STAGE_ENEMY}, 1},
    {{STAGE_EVENT, STAGE_ENEMY}, 2},
    {{STAGE_ENEMY}, 1},
    {{STAGE_SHOP, STAGE_ENEMY}, 2},
    {{STAGE_ENEMY, STAGE_EVENT}, 2},
    {{STAGE_ENEMY, STAGE_REST}, 2},
    {{STAGE_ELITE, STAGE_EVENT}, 2},
    {{STAGE_ENEMY}, 1},
    {{STAGE_CHEST}, 1},
    {{STAGE_SHOP, STAGE_ENEMY}, 2},
    {{STAGE_ENEMY, STAGE_REST}, 2},
    {{STAGE_ELITE, STAGE_ENEMY}, 2},
    {{STAGE_ENEMY}, 1},
    {{STAGE_REST}, 1},
    {{STAGE_BOSS}, 1}
};

//유효한 층인지 판정하는 함수
int is_valid_floor(int floor) {
    return floor >= 1 && floor <= MAX_FLOOR;
}

//최종층인지 판정하는 함수
int is_final_floor(int floor) {
    return floor == MAX_FLOOR;
}

//맵층을 가져오는 함수
const MapFloor *get_map_floor(int floor) {
    if (!is_valid_floor(floor)) {
        return NULL;
    }

    return &fixed_map[floor - 1];
}

//해당층의 스테이지 타입을 가져오는 함수
StageType get_default_stage_type(int floor) {
    const MapFloor *map_floor;

    map_floor = get_map_floor(floor);

    if (map_floor == NULL || map_floor->choice_count <= 0) {
        return STAGE_ENEMY;
    }

    return map_floor->choices[0];
}

//스테이지 타입별 이름 설정
const char *get_stage_type_name(StageType stage) {
    switch (stage) {
    case STAGE_ENEMY:
        return "일반 전투";
    case STAGE_ELITE:
        return "정예 전투";
    case STAGE_REST:
        return "휴식";
    case STAGE_SHOP:
        return "상점";
    case STAGE_CHEST:
        return "보물";
    case STAGE_EVENT:
        return "이벤트";
    case STAGE_BOSS:
        return "보스";
    default:
        return "알 수 없음";
    }
}