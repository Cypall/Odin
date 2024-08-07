#ifndef MONSTER_SKILLS_H
#define MONSTER_SKILLS_H

void init_monster_skills();
void monster_heal(int fd, int m, int n);
void monster_use_effects(struct map_session_data *sd, int fd, int myoption, int m, int n);
void monster_use_skill1(struct map_session_data *sd, int fd, int skill_num, int m, int n);
void monster_use_skill2(struct map_session_data *sd, int fd, int skill_num, int m, int n);
void monster_skills(struct map_session_data *sd, int fd, int damage, int m, int n);
void check_monster_skills(struct map_session_data *sd, unsigned int fd, short m, int n);

#endif
