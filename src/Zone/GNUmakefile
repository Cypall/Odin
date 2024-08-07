all z: Odin-Zone 

COMMON_OBJ = ../Core/core.o ../Core/socket.o ../Core/timer.o
COMMON_H = ../Core/core.h ../Core/socket.h ../Core/timer.h ../Core/common.h

Odin-Zone: map_core.o save.o grfio.o npc.o item.o itemdb.o skill_db.o script.o party.o guild.o pet.o skill.o element_damage.o chat.o card.o equip.o trade.o storage.o monster_skills.o $(COMMON_OBJ)
	$(CC) -o ../$@ $^ -lz -ggdb

map_core.o: map_core.c
npc.o: npc.c
itemdb.o: itemdb.c
skill_db.o: skill_db.c
skill.o: skill.c
grfio.o: grfio.c
save.o: save.c
party.o: party.c
guild.o: guild.c
script.o: script.c
pet.o: pet.c
element_damage.o: element_damage.c
chat.o: chat.c
item.o: item.c
card.o: card.c
equip.o: equip.c
trade.o: trade.c
storage.o: storage.c
monster_skills.o: monster_skills.c

clean:
	rm -f *.o
