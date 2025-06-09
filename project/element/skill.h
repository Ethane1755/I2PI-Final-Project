#ifndef SKILL_H
#define SKILL_H

#include "character.h"

typedef enum {
    SKILL_HEAL_10P,
    SKILL_SHIELD_10P,
    SKILL_SPEED_10P,
    SKILL_MULTI_SHOT,
    SKILL_COUNT
} SkillType;

typedef struct {
    SkillType selected_skill;
    SkillType options[3]; // Three random skills
    double last_select_time;
    bool selecting;
} SkillSystem;

void SkillSystem_init(SkillSystem* sys);
void SkillSystem_update(SkillSystem* sys, Character* player);
void SkillSystem_draw_menu(SkillSystem* sys);
void SkillSystem_apply(SkillSystem* sys, Character* player);

#endif