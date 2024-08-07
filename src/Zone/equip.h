#ifndef EQUIP_H
#define EQUIP_H

int mmo_map_all_nonequip(int fd, unsigned char *buf);
int mmo_map_all_equip(int fd, unsigned char *buf);
void mmo_cart_send_items(int fd);
int mmo_cart_item_remove(int fd, unsigned char* buf, int index, int amount);
void remove_item(int fd, int id);

#endif
