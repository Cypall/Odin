#ifndef ITEMDB_H
#define ITEMDB_H

#define MAX_JOBS 18

int itemdb_type(int nameid);
int itemdb_sellvalue(int nameid);
int itemdb_weight(int nameid);
int itemdb_isequip(int named);
int itemdb_stype(int nameid);
int itemdb_equip_point(int named,struct map_session_data *sd);
int itemdb_init(void);
int itemdb_view_point(int nameid);
struct item_db2 item_database(int item_id);
//int itemdb_hands(int nameid);
int itemdb_can_equipt(int nameid,short class);
int itemdb_elv(int nameid);
int search_itemdb_index(int nameid);
int itemdb_recv_data (int fd);   //mysql
int itemdb_recv_hdata (int fd);   //mysql

int skill_forge_db_init(void);//forge skills db
int search_item2(struct map_session_data *sd,int nameid);
int forge_db_index(int nameid);//returns forge db item number
	
int search_itemdb_nameid(char name[24]);

struct item_database{
  // for hash
  int nameid;
  int next;
    // data
    char name[24];
    int type;
    int price;
    int sell;
    int weight;
    int atk;
    int matk;
    int def;
    int mdef;
    int range;
    int slot;
    int str;
    int agi;
    int vit;
    int int_;
    int dex;
    int luk;
    int job;
    int gender;
    int loc;
    int wlv;
    int elv;
    int view;
    int ele;
    int eff;
    int hp1;
    int hp2;
    int sp1;
    int sp2;
    int rare;
    int box;
    int hit,critical,flee,skill_id;
    //itemslottable
    int equip;    
    //class_equip.txt
    short job_equipt[MAX_JOBS+1];


}*itemdb; 

#endif /* ITEMDB_H */
