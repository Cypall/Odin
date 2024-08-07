#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmo.h"
#include "core.h"
#include "grfio.h"
#include "itemdb.h"

#define ITEMDB_HASH_SIZE 64

static int itemdb_hash[ITEMDB_HASH_SIZE];
static int itemdb_size,itemdb_num;

int search_itemdb_nameid(char name[24]){
	
	int begin = 501; //first item
	int last = 10020; //last
	int j=0,i;
	
	for(j=begin;j<last;j++){ //item range , not the best way.. ideas?
		i = search_itemdb_index(j);
		if(strcmp(itemdb[i].name,name)==0) 
			return j;
	}
	return -1;
}


int search_itemdb_index(int nameid)
{
  int i,j;
  for(i=itemdb_hash[nameid%ITEMDB_HASH_SIZE];i>=0;i=itemdb[i].next)
    if(itemdb[i].nameid==nameid)
      return i;
  if(itemdb_size<=itemdb_num){
    itemdb_size+=ITEMDB_HASH_SIZE;
    itemdb=realloc(itemdb,sizeof(itemdb[0])*itemdb_size);
  }

  i=itemdb_num;
  itemdb_num++;
  itemdb[i].nameid=nameid;
  itemdb[i].next=itemdb_hash[nameid%ITEMDB_HASH_SIZE];
  itemdb_hash[nameid%ITEMDB_HASH_SIZE]=i;

// init on first search
  itemdb[i].type=9;
  itemdb[i].job=0;
  itemdb[i].equip=0;
  itemdb[i].weight=10;
  itemdb[i].atk=0;
  itemdb[i].def=0;
  itemdb[i].slot=0;
  itemdb[i].view=0;

          // init value fix
            itemdb[i].price    = 10;
            itemdb[i].sell     = 1;
            itemdb[i].matk     = 0;
            itemdb[i].mdef     = 0;
            itemdb[i].range    = 0;
            itemdb[i].str      = 0;
            itemdb[i].agi      = 0;
            itemdb[i].vit      = 0;
            itemdb[i].int_     = 0;
            itemdb[i].dex      = 0;
            itemdb[i].luk      = 0;
            itemdb[i].gender   = 0;
            itemdb[i].loc      = 0;
            itemdb[i].wlv      = 0;
            itemdb[i].elv      = 0;
            itemdb[i].ele      = 0;
            itemdb[i].eff      = 0;
            itemdb[i].hp1      = 0;
            itemdb[i].hp2      = 0;
            itemdb[i].sp1      = 0;
			itemdb[i].sp2      = 0;
            itemdb[i].rare     = 0;
            itemdb[i].box      = 0;
            itemdb[i].hit      = 0,
            itemdb[i].critical = 0;
            itemdb[i].flee     = 0;
            itemdb[i].skill_id = 0;



  for(j=0;j<MAX_JOBS+1;j++) itemdb[i].job_equipt[j] = 0;
  // noone can equipt by default -> change above 0 to 1
  // and unknown items can be put on by all jobclasses

  return i;
}
//finds item by id and returns his index
int search_item2(struct map_session_data *sd, int nameid)
{
	int i, index = 0;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid == nameid) {
			index = i + 2;
			break;
		}
	}
	return index;
}

int forge_db_index(int nameid){
	
	int i;
	for(i=0;i<MAX_SKILL_REFINE;i++){
		if(forge_db[i].nameid == nameid )
			break;
	}
	return i;
}

// new version  wisc
int itemdb_type(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].type;
}
int itemdb_loc(int nameid) {
  return itemdb[search_itemdb_index(nameid)].loc;
}
/*position of equipment*/
//Return values
//2 = weapon
//3 = head B
//4 = head T
//5 = head m
int itemdb_stype(int nameid) {
 if (itemdb_type(nameid)==4) {
  switch (itemdb_loc(nameid)) {
   case 2: //One Handed
    return 2; //weapon
    break;
   case 34: //Two Handed
    return 2; //weapon
    break;
   default:
    printf("%d item: Unknown item loc: %d\n",nameid,itemdb_loc(nameid));
    return 6;
    break;
  }
 }else if (itemdb_type(nameid)==5){
  switch (itemdb_loc(nameid)) {
   case 0: //Head Bottom
    return 3;
    break;
   case 1: //Head Bottom
    return 3;
    break;
   case 4: //Garment
    return 6;
    break;
   case 16: //Armor
    return 6;
    break;
   case 32: //Shield
    return 8;
    break;
   case 64: //Shoes
    return 6;
    break;
   case 136: //Accessorys
    return 6;
    break;
   case 256: //Head Top
    return 4;
    break;
   case 512: //Head Middle
    return 5;
    break;
   case 513: //Head Top + Middle
    return 4;
    break;
   case 769: //Head Top + Middle + Bottom
    return 3;
    break;
   default:
    printf("%d item: Unknown item loc: %d\n",nameid,itemdb_loc(nameid));
    return 6;
    break;
  }
 }else if (itemdb_type(nameid)==8) { //pet stuff
  return 6;
 }else {
  printf("%d item: Unknown item Type: %d\n",nameid,itemdb_type(nameid));
  return 6;
 }

	//‚Q‚P‚O‚O~‚Q‚P‚O‚W shield
	//‚Q‚R‚O‚P~‚Q‚U‚Q‚O armor, shoes, and accessory
	//‚T‚O‚O‚P~‚T‚O‚P‚X all equipment
	//2224,2225,2264 head gear
	//2201~229 head gear(including mid, and lowerj
/*
	if(nameid>2100 && nameid<2109)
		return 8;//LOOK_shield;
	//upper tier equipment
	if((nameid>2205 && nameid<2212) || (nameid>2212 && nameid<2218)
		|| (nameid>2219 && nameid<2237) || (nameid>2243 && nameid<2262)
			|| (nameid>2272 && nameid<2276) || (nameid == 2277)
			|| (nameid>2278 && nameid<2281) ||(nameid>2281 && nameid<2286) 
			||(nameid>5000  && nameid < 5006) || (nameid > 5006 && nameid<5040) 
			||(nameid == 2264) ||(nameid>2286 && nameid<2288) 
			||(nameid>2288 && nameid<2291)
			|| (nameid>5041 && nameid<5043) || (nameid>5043 && nameid<5054)
			|| (nameid >2291 && nameid<2295) || (nameid >2295 && nameid <2300))
				return LOOK_HEAD_TOP;
	if((nameid >2200 && nameid<2206) || (nameid == 2212) 
		||( nameid == 2239) || (nameid == 2242) ||
		(nameid == 2243) || (nameid == 2262) ||
		(nameid == 2276) || (nameid == 2278) ||
		(nameid == 2263) || //In. also lower equipment makes middle(?) (’†A‰º‘•”õ‚à^‚ñ’†‚É‚µ‚Ä‚¨‚­)
		(nameid == 2281) ||(nameid == 2288) || (nameid == 2291) || (nameid == 5006)
		|| (nameid == 2295) || (nameid == 5040) || (nameid == 5041) || (nameid == 5043) || (nameid == 2297))
				return LOOK_HEAD_MID;
	if((nameid == 2218) || (nameid ==  2219) ||( nameid == 2237 )||(nameid ==  2238) 
			|| (nameid == 2240) || (nameid == 2241)||( nameid>2264 && nameid<2271))
			return LOOK_HEAD_BOTTOM;
	if(nameid>1100 && nameid<2000)
   		 return 2;//LOOK_WEAPON;	//weapon
   		
   	return LOOK_06;
	*/
	return LOOK_SHOES;
}


int skill_forge_db_init(){
	
	FILE *fp;
	int nameid,req_skill,itemlv,mat_id[5],mat_amount[5];
	int i=0;
	char line[2040];
	fp= fopen("data/databases/forge_db.txt","r");
	if(fp)
	{
		while(fgets(line,2040,fp)){
			sscanf(line,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&nameid,&itemlv,&req_skill,&mat_id[0],&mat_amount[0],
			&mat_id[1],&mat_amount[1],&mat_id[2],&mat_amount[2],
			&mat_id[3],&mat_amount[3],&mat_id[4],&mat_amount[4]);
				
			forge_db[i].nameid= nameid;
			forge_db[i].itemlv= itemlv;
			forge_db[i].req_skill= req_skill;
			forge_db[i].mat_id[0]= mat_id[0];
			forge_db[i].mat_amount[0]= mat_amount[0];
			forge_db[i].mat_id[1]= mat_id[1];
			forge_db[i].mat_amount[1]= mat_amount[1];
			forge_db[i].mat_id[2]= mat_id[2];
			forge_db[i].mat_amount[2]= mat_amount[2];
			forge_db[i].mat_id[3]= mat_id[3];
			forge_db[i].mat_amount[3]= mat_amount[3];
			forge_db[i].mat_id[4]= mat_id[4];
			forge_db[i].mat_amount[4]= mat_amount[4];
			
			i++;
		}
		fclose(fp);	
	} else printf ("\nCouldn't Find data/databasesforge_db.txt ...\n");
	return 0;

}
int itemdb_db2_init()
{
	FILE *fp;
	int item_id,Type,Price,Sell,Weight,ATK,MATK,DEF,MDEF,Range,Slot;
    int STR,AGI,VIT,INT,DEX,LUK,Job,Gender,Loc,wLV,eLV,View,Ele,Eff;
    int HP1,HP2,SP1,SP2,Rare,Box,HIT,CRITICAL,FLEE,SKILL_ID,weap_typ;
	char name[128],JName[256];
	char line[2040];

    int item_index;

	fp = fopen("data/databases/item_db.txt","r");
	if(fp)
	{
		while(fgets(line,2040,fp)){
//all fixed by  wisc
            item_id=Type=Price=Sell=Weight=ATK=MATK=DEF=MDEF=Range=Slot=
            STR=AGI=VIT=INT=DEX=LUK=Job=Gender=Loc=wLV=eLV=View=Ele=Eff=
            HP1=HP2=SP1=SP2=Rare=Box=HIT=CRITICAL=FLEE=SKILL_ID= -1;

		sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		&item_id,name,JName,&Type,&Price,&Sell,&Weight,&ATK,&MATK,&DEF,
                &MDEF,&Range,&Slot,&STR,&AGI,&VIT,&INT,&DEX,&LUK,&Job,
                &Gender,&Loc,&wLV,&eLV,&View,&Ele,&Eff,&HP1,&HP2,&SP1,
                &SP2,&Rare,&Box,&HIT,&CRITICAL,&FLEE,&SKILL_ID,&weap_typ);

            if (item_id < 0) continue;
            item_index = search_itemdb_index(item_id);
	    
	    strcpy(itemdb[item_index].name,name);
            itemdb[item_index].type     = Type;
            itemdb[item_index].price    = Price;
            itemdb[item_index].sell     = Sell;
            itemdb[item_index].weight   = Weight;
            itemdb[item_index].atk      = ATK;
            itemdb[item_index].matk     = MATK;
            itemdb[item_index].def      = DEF;
            itemdb[item_index].mdef     = MDEF;
            itemdb[item_index].range    = Range;
            itemdb[item_index].slot     = Slot;
            itemdb[item_index].str      = STR;
            itemdb[item_index].agi      = AGI;
            itemdb[item_index].vit      = VIT;
            itemdb[item_index].int_     = INT;
            itemdb[item_index].dex      = DEX;
            itemdb[item_index].luk      = LUK;
            itemdb[item_index].job      = Job;
            itemdb[item_index].gender   = Gender;
            itemdb[item_index].loc      = Loc;
            itemdb[item_index].wlv      = wLV;
            itemdb[item_index].elv      = eLV;
            itemdb[item_index].view     = View;
            itemdb[item_index].ele      = Ele;
            itemdb[item_index].eff      = Eff;
            itemdb[item_index].hp1      = HP1;
            itemdb[item_index].hp2      = HP2;
            itemdb[item_index].sp1      = SP1;
			itemdb[item_index].sp2      = SP2;
            itemdb[item_index].rare     = Rare;
            itemdb[item_index].box      = Box;
            itemdb[item_index].hit      = HIT,
            itemdb[item_index].critical = CRITICAL;
            itemdb[item_index].flee     = FLEE;
            itemdb[item_index].skill_id = SKILL_ID;

//printf("%d sell:%d\n",item_id,Sell);

			//Fred fix1 (Item Selling Price)
//			itemdb[search_itemdb_index(item_id)].sell = Price/2+1;


		}  // fixed by MagicalTux (2003/07/12)
	  fclose(fp);
  } else {
    printf("Warning : couldn't open cfg/item_db2.txt !\n");
  }
	return 0;
}

int itemdb_sellvalue(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].sell;
}

int itemdb_weight(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].weight;
}

int itemdb_isequip(int nameid)
{
  return itemdb[search_itemdb_index(nameid)].equip!=0;
}

int itemdb_elv(int nameid)  
{
  return itemdb[search_itemdb_index(nameid)].elv; //lvl need to equip
}

//function returns the number from equipment to item
int itemdb_equip_point(int nameid,struct map_session_data *sd)
{
  return itemdb[search_itemdb_index(nameid)].equip;
}

int itemdb_view_point(int nameid)
{
 return itemdb[search_itemdb_index(nameid)].view;
}


int itemdb_weapon_init()
{
    // designed to check for weappon equipting
    // now only for converting txt files ..
    // this function is not called by anymore

   FILE *fp;
   FILE *fw;
   int item_id,hands;
   int job[20];
   char name[128];
   int n;
   int i,j;
   char line[1024];

   fp = fopen("cfg/weapon_info.txt","r");
   fw = fopen("data/tables/class_equip.txt","w");

   if(fp)
   {

       j=0;
       while(fgets(line,1024,fp)){

           for(i=0;i<20;i++) job[i] = -1;

           if((n = sscanf(line,"%d,%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&item_id,name,&hands,&job[0],&job[1],&job[2],&job[3],&job[4],&job[5],&job[6],&job[7],&job[8],&job[9],&job[10],&job[11],&job[12],&job[13],&job[14],&job[15],&job[16],&job[17],&job[18],&job[19])) < 3)
               continue;

           j=search_itemdb_index(item_id);
//           itemdb[j].hands = hands;
           if(fw) fprintf(fw,"%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",item_id,name,job[0],job[1],job[2],job[3],job[4],job[5],job[6],job[7],job[8],job[9],job[10],job[11],job[12],job[13],job[14],job[15],job[16],job[17],job[18],job[19]);
           for(i=0;i<MAX_JOBS+1;i++) itemdb[j].job_equipt[i] = 0;

           if (job[0] == 99) {
               for(i=0;i<MAX_JOBS+1;i++) itemdb[j].job_equipt[i] = 1;
           } else {
               for(i=0;i<n-3;i++) itemdb[j].job_equipt[job[i]] = 1; 
           }
       }
       fclose(fp); // Fixed by MagicalTux (2003/07/12)
   } else {
       printf("Warning : couldn't open cfg/weapon_info.txt !\n");
   }
   if (fw) {
       fclose(fw);
       printf("class_equip file created\n");
   } else {
       printf("Warning : couldn't open data/tables/class_equip.txt for writing !\n");
   }
   return 0;
}

int itemdb_equipment_init()
{
   //set all the possible equip classes

   FILE *fp;

   int item_id;
   int job[20];
   char name[128];
   int n;
   int i,j;
   char line[1024];

   fp = fopen("data/tables/class_equip.txt","r");

   if(fp)
   {

       j=0;
       while(fgets(line,1024,fp)){
		
	   //init
           for(i=0;i<20;i++)  { job[i] = 0; }

	   // read from file
           if((n = sscanf(line,"%d,%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&item_id,name,&job[0],&job[1],&job[2],&job[3],&job[4],&job[5],&job[6],&job[7],&job[8],&job[9],&job[10],&job[11],&job[12],&job[13],&job[14],&job[15],&job[16],&job[17],&job[18],&job[19])) < 3) {
               continue;
	   }
	   
	   // get index 
           j=search_itemdb_index(item_id);
	   
	   //init
           for(i=0;i<20;i++)  { itemdb[j].job_equipt[i] = 0; }

	   // set values
           if (job[0] == 99) {
               for(i=0;i<20;i++)  { itemdb[j].job_equipt[i] = 1; }
           }
           
           else {
               for(i=0;i<n-3;i++) { 
               		if(job[i]!= 111) {itemdb[j].job_equipt[job[i]] = 1; }	
               }
           }
       }
       fclose(fp);
   } else { // Fixed by MagicalTux (2003/07/12)
       printf("Warning : couldn't open data/tables/class_equip.txt !\n");
   }
   return 0;
}


int itemdb_can_equipt(int nameid,short class)
{
   int i=search_itemdb_index(nameid);
   int class1=itemdb[i].job_equipt[class];
   int class2=0;
   int ret;
   if(!class1) {
    if(class==7) class2=1;
	if(class==9) class2=2;
	if(class==11) class2=3;
	if(class==8) class2=4;
	if(class==10) class2=5;
	if(class==12) class2=6;
	ret=itemdb[i].job_equipt[class2];
   } else { ret=itemdb[i].job_equipt[class]; }
   return ret;          //return 1 can equipt
}


static int itemdb_read_itemslottable(void)
{
  char *buf,*p;
  int s;
  buf=grfio_read("data\\itemslottable.txt");
  if(buf==NULL)
    return -1;
  s=grfio_size("data\\itemslottable.txt");
  buf[s]=0;
  for(p=buf;p-buf<s;){
    int nameid,equip;
    /* item name and number for equipping*/
    sscanf(p,"%d#%d#",&nameid,&equip);
    itemdb[search_itemdb_index(nameid)].equip=equip;
    p=strchr(p,10);
    if(!p) break;
    p++;
    p=strchr(p,10);
    if(!p) break;
    p++;
  }
  free(buf);
  return 0;
}

int itemdb_init(void)
{
  int i;
  for(i=0;i<ITEMDB_HASH_SIZE;i++)
    itemdb_hash[i]=-1;
  itemdb_size=ITEMDB_HASH_SIZE;
  itemdb_num=0;
  itemdb=malloc(sizeof(itemdb[0])*itemdb_size);
  
  itemdb_read_itemslottable();
  itemdb_db2_init();
  itemdb_equipment_init();
  skill_forge_db_init();
//  itemdb_weapon_init();     //txt files converting function now ;)
#if 0
  for(i=0;i<ITEMDB_HASH_SIZE;i++){
    int j,c;
    for(j=itemdb_hash[i],c=0;j>=0;j=itemdb[j].next,c++);
    printf("%4d",c);
    if(i%16==15) printf("\n");
  }
  exit(1);
#endif
  return 0;
}


/*
int itemdb_view_point(int nameid)
{
//printf("item_viewpoint called ->file accessed<- this function summons evil\n");

	char strgat[1024 * 80],s_nameid[8],s_view[4];
	FILE *fp;
	int t_nameid,view,fcount,scount,i,tcount,j;
	memset(strgat,'\0',sizeof(strgat));
	memset(s_nameid,'\0',sizeof(s_nameid));
	memset(s_view,'\0',sizeof(s_view));
	scount = 0;
	fcount = 0;
	tcount = 0;
	//the number of the ID from item_db.txt is removed (25th)
	fp = fopen("cfg/id_view.txt","r");
	while( (strgat[fcount++] =	getc(fp)) != EOF )
		;
	fclose(fp);
	while(strgat[scount] != '*')
	{
		i = 0;
		j = 0;
		while(strgat[scount] != ':' && i <=3){
			s_nameid[i] = strgat[scount];
			scount++;
			i++;
		}
		scount++;
		t_nameid = atoi(s_nameid);
		memset(s_view,'\0',sizeof(s_view));
		while(strgat[scount] != '\n'){
				s_view[j] = strgat[scount];
				scount++;
				j++;
			}

		view = atoi(s_view);
		scount++;
		if(t_nameid == nameid)
		{
//			printf("ID:VI %d:%d\n",t_nameid,view);
			return view;
		}
	}
//	printf("Miss Search Item ViewNumber\n");
	return 0;
}
*/


// item_db2 speed up  wisc
struct item_db2 ret_item;
// do not use this if possible 
// create returning functions using the old (now working) db
struct item_db2 item_database(int item_id)
{
    int i;

/*	FILE *fp;
	//ID,Name,Jname,Type,Price,Sell,Weight,ATK,MATK,DEF,MDEF,Range,Slot,STR,AGI,VIT,INT,DEX
	//,LUK,Job,Gender,Loc,wLV,eLV,View,Ele,Eff,HP1,HP2,SP1,SP2,Rare,Box
//	int name_id,type,price,sell,weight,atk,matk,def,mdef,range,slot,str,agi,vit,int_,dex;
//	int luk,job,gender,loc,wlv,elv,view,ele,eff,hp1,hp2,sp1,sp2,rare,box;
//	int hit,critical,flee,skill_id;
//	char name[128],JName[256];
//	char line[2040];

//printf("item_database called ->file accessed:(\n");

//	fcount = 0;
//	i = 0;
	//the number of the ID from item_db.txt is removed (25th)

	fp = fopen("cfg/item_db2.txt","r");
	if(fp)
	{
		//	printf("item_id %d\n",item_id);
		while(fgets(line,2040,fp)){
			sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				&name_id,name,JName,
				&type,&price,&sell,&weight,&atk,&matk,&def,&mdef,&range,&slot,
				&str,&agi,&vit,&int_,&dex,&luk,&job,&gender,&loc,&wlv,
				&elv,&view,&ele,&eff,&hp1,&hp2,&sp1,&sp2,&rare,&box,&hit,&critical,&flee,&skill_id);

			if(name_id == item_id)
			{
*/

            i = search_itemdb_index(item_id);

			ret_item.nameid = itemdb[i].nameid;
			ret_item.type = itemdb[i].type;
			ret_item.sell = itemdb[i].price;
			ret_item.weight = itemdb[i].weight;
			ret_item.atk = itemdb[i].atk;
			ret_item.matk = itemdb[i].matk;
			ret_item.def = itemdb[i].def;
			ret_item.mdef = itemdb[i].mdef;
			ret_item.range = itemdb[i].range;
			ret_item.slot = itemdb[i].slot;
			ret_item.str = itemdb[i].str;
			ret_item.agi = itemdb[i].agi;
			ret_item.vit = itemdb[i].vit;
			ret_item.int_ = itemdb[i].int_;
			ret_item.dex = itemdb[i].dex;
			ret_item.luk = itemdb[i].luk;
			ret_item.job = itemdb[i].job;
			ret_item.gender = itemdb[i].gender;
			ret_item.loc = itemdb[i].loc;
			ret_item.wlv = itemdb[i].wlv;
			ret_item.elv = itemdb[i].elv;
			ret_item.view = itemdb[i].view;
			ret_item.ele = itemdb[i].ele;
			ret_item.eff = itemdb[i].eff;
			ret_item.hp1 = itemdb[i].hp1;
			ret_item.hp2 = itemdb[i].hp2;
			ret_item.sp1 = itemdb[i].sp1;
			ret_item.sp2 = itemdb[i].sp2;
			ret_item.hit = itemdb[i].hit;
			ret_item.critical = itemdb[i].critical;
			ret_item.flee = itemdb[i].flee;
			ret_item.skill_id = itemdb[i].skill_id;

/*
			fclose(fp);
			return ret_item;

			}
		}
	}

	printf("That item does not exist!\n");
	fclose(fp);
*/
	return ret_item;
}

int itemdb_recv_data (fd) {
// char sends item config to map
    int item_index;
      item_index = search_itemdb_index(RFIFOL(fd,2));

            itemdb[item_index].price    = RFIFOL(fd,10);
            itemdb[item_index].sell     = (int) ((RFIFOL(fd,10)+1)/2)-1;
            itemdb[item_index].weight   = RFIFOL(fd,14);
            itemdb[item_index].atk      = RFIFOL(fd,18);
            itemdb[item_index].matk     = RFIFOL(fd,22);
            itemdb[item_index].def      = RFIFOL(fd,26);
            itemdb[item_index].mdef     = RFIFOL(fd,30);
            itemdb[item_index].range    = RFIFOL(fd,34);
            itemdb[item_index].slot     = RFIFOL(fd,38);
            itemdb[item_index].str      = RFIFOL(fd,42);
            itemdb[item_index].agi      = RFIFOL(fd,46);
            itemdb[item_index].vit      = RFIFOL(fd,50);
            itemdb[item_index].int_     = RFIFOL(fd,54);
            itemdb[item_index].dex      = RFIFOL(fd,58);
            itemdb[item_index].luk      = RFIFOL(fd,62);
            itemdb[item_index].job      = RFIFOL(fd,66);
            itemdb[item_index].gender   = RFIFOL(fd,70);
            itemdb[item_index].loc      = RFIFOL(fd,74);
            itemdb[item_index].hp1      = RFIFOL(fd,82);
            itemdb[item_index].sp1      = RFIFOL(fd,86);
    

return 0 ;
}

int itemdb_recv_hdata (fd) {
// char sends heal item config to map
    int item_index;
          item_index = search_itemdb_index(RFIFOL(fd,2));
      
            itemdb[item_index].price    = RFIFOL(fd,6);
            itemdb[item_index].sell     = (int) ((RFIFOL(fd,6)+1)/2)-1;
            itemdb[item_index].weight   = RFIFOL(fd,10);
            itemdb[item_index].hp1      = RFIFOL(fd,14);
            itemdb[item_index].hp2      = RFIFOL(fd,18);
            itemdb[item_index].sp1      = RFIFOL(fd,22);
            itemdb[item_index].sp2      = RFIFOL(fd,24);
    
    
return 0;
}
