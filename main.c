#include <stdio.h>
#include "middle.h"
#include "HAL.h"
/*******************************************************************************
 * Include
 ******************************************************************************/

static void ConvertFileExtension(entryList_t *Entry);
static void ConvertShortName(entryList_t *Entry);
static void ConvertTime(entryList_t *Entry);
static void ConvertTimeModified(entryList_t *Entry);
static void ConvertDate(entryList_t *Entry);
static void ConvertDateModified(entryList_t *Entry);
static void PrintEntryList(entryList_t **HeadEntry);
static void Check_Select(uint8_t userSelect,volatile uint8_t *updateOption, entryList_t **HeadEntry);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(){
	uint8_t status; 
	uint8_t userOption = 0;
	volatile uint8_t Check_option = 0;
	status = middle_Init();
	entryList_t *currentDirectory = NULL;
	
	
	if(!status){
		printf("ERROR!");
	}
	printf("NO\tNAME\t\tType\tTime Create\t\tTime Modified\t\tSize\n");
	Check_option = middle_ReadRoot(&currentDirectory);
	PrintEntryList(&currentDirectory);
	while(1){
		
		do{
			printf(">>Enter your select:");
			scanf("%d",&userOption);
			fflush(stdin);
			if((0 >= userOption) || (userOption > Check_option)){
				printf("ERROR: incorrect option!\n");
			}
		}while((0 >= userOption) || (userOption > Check_option));
			Check_Select(userOption, &Check_option, &currentDirectory);
	}
	middle_Deinit();
	return 0;
}
/*------------------------Function to Check userSelect------------------------*/
static void Check_Select(uint8_t userSelect,volatile uint8_t *updateOption, entryList_t **HeadEntry){
	
	volatile uint32_t cluster = 0;
	
	entryList_t* Entry = middle_Find_Entry(userSelect, HeadEntry);
	cluster = Entry->entryData.firstCluster;
	
	if((Entry->entryData.attribute == ATTRIBUTE_FILE) | (Entry->entryData.attribute == ATTRIBUTE_FILE_FAT32)){                                  /*Check attribute in entry to readfile*/
		middle_ReadFile(cluster);
		printf("\nMADE by DATNT212\n\n\n");
		printf("NO\tNAME\t\tType\tTime Create\t\tTime Modified\t\tSize\n");
		PrintEntryList(HeadEntry);
	}else {
//	if(Entry->entryData.attribute == ATTRIBUTE_FOLDER){                                /*Check attribute in entry to readfolder*/
		middle_FreeEntryList(HeadEntry);
		printf("\nMADE by DATNT212\n\n\n");
		if(Entry->entryData.firstCluster == START_CLUSTER_ROOT){
			printf("NO\tNAME\t\tType\tTime Create\t\tTime Modified\t\tSize\n");
			*updateOption = middle_ReadRoot(HeadEntry);
			PrintEntryList(HeadEntry);
		}else{
			printf("NO\tNAME\t\tType\tTime Create\t\tTime Modified\t\tSize\n");
			*updateOption = middle_ReadSubDirectory(cluster, HeadEntry);
			PrintEntryList(HeadEntry);
		}
	}
}
/*---------------------------Function to print linked list to screen------------*/
static void PrintEntryList(entryList_t **HeadEntry){
	entryList_t *Entry = *HeadEntry;
	while(Entry != NULL){
		printf("%d.\t", Entry->entryData.NumberOrder);
			ConvertShortName(Entry);
			ConvertFileExtension(Entry);
			
			if(Entry->entryData.attribute == ATTRIBUTE_FILE | Entry->entryData.attribute == ATTRIBUTE_FILE_FAT32){
				printf("\tFile");
			} else {
				printf("\tFolder");
			}
		ConvertTime(Entry);
		ConvertDate(Entry);
		ConvertTimeModified(Entry);
		ConvertDateModified(Entry);
		printf("\t%d", Entry->entryData.size);
		printf("\n");
		Entry = Entry->next;
	}
}
/*----------------------Function to convert FileExtension-------------------*/
static void ConvertFileExtension(entryList_t *Entry){
	uint8_t countbyte = 0;
	char hex_str[LENGTH_FILEEXTENSION] ;
    
    for(countbyte=0; countbyte<LENGTH_FILEEXTENSION; countbyte++){
    	hex_str[countbyte] = Entry->entryData.shortFileExtension[countbyte];
	}
	for(countbyte=0; countbyte<LENGTH_FILEEXTENSION; countbyte++){
			printf("%c", hex_str[countbyte]);
	}
}
/*---------------------Function to convert ShortName-------------*/
static void ConvertShortName(entryList_t *Entry){
	
	uint8_t countbyte = 0;
	char hex_str[LENGTH_SHORTNAME];
	
	for(countbyte=0; countbyte<LENGTH_SHORTNAME;countbyte++){
    hex_str[countbyte] = Entry->entryData.shortName[countbyte];
	}
	
    if(Entry->entryData.attribute == ATTRIBUTE_FILE){

		for(countbyte = 0; countbyte<LENGTH_SHORTNAME; countbyte++){
			if(hex_str[countbyte] == SPACE_OFFSET){
				printf(".");
				countbyte = LENGTH_SHORTNAME;
			}else {
				printf("%c", hex_str[countbyte]);
			}
		}
	} else {
		for(countbyte=0; countbyte<LENGTH_SHORTNAME; countbyte++){
				printf("%c", hex_str[countbyte]);
		}
	}
}
/*-----------------------Function to convert Time Created--------------*/
static void ConvertTime(entryList_t *Entry){
	
	uint8_t seconds = 0;
	uint8_t minutes = 0;
	uint8_t hours = 0;
	
	seconds = ((Entry->entryData.timeCreated>>SECONDS_SHIFTS)&SECONDS)*REVERVED_TIME;
	minutes = (Entry->entryData.timeCreated>>MINUTES_SHIFTS)&MINUTES;
	hours = (Entry->entryData.timeCreated>>HOUR_SHIFTS)&HOUR;
	printf("\t%d:%d:%d", hours, minutes, seconds);
}
/*---------------------Finction to convert Time modified-----------------*/
static void ConvertTimeModified(entryList_t *Entry){
	
	uint8_t seconds_modified = 0;
	uint8_t minutes_modified = 0;
	uint8_t hours_modified = 0;
	
	seconds_modified = ((Entry->entryData.timeModified>>SECONDS_SHIFTS)&SECONDS)*2;
	minutes_modified = (Entry->entryData.timeModified>>MINUTES_SHIFTS)&MINUTES;
	hours_modified = (Entry->entryData.timeModified>>HOUR_SHIFTS)&HOUR;
	
	if(Entry->entryData.attribute == ATTRIBUTE_FILE){
		printf("\t%d:%d:%d", hours_modified, minutes_modified, seconds_modified);
	}else{
		printf("\t\t%d:%d:%d", hours_modified, minutes_modified, seconds_modified);
	}
}
/*-----------------------Function to convert Date created-------------------*/
static void ConvertDate(entryList_t *Entry){
	
	uint8_t day = 0;
	uint8_t month = 0;
	uint8_t year = 0;

	day = (Entry->entryData.date>>DAY_SHIFTS)&DAY;
	month = (Entry->entryData.date>>MONTH_SHIFTS)&MONTH;
	year = (Entry->entryData.date>>YEAR_SHIFTS)&YEAR;
	printf(" %d/%d/%d", day, month, year+REAL_YEAR);
}
/*-----------------------Function to convert Date modified--------------------*/
static void ConvertDateModified(entryList_t *Entry){
	
	uint8_t day_modified = 0;
	uint8_t month_modified = 0;
	uint8_t year_modified = 0;
	
	day_modified = (Entry->entryData.dateModified>>DAY_SHIFTS)&DAY;
	month_modified = (Entry->entryData.dateModified>>MONTH_SHIFTS)&MONTH;
	year_modified = (Entry->entryData.dateModified>>YEAR_SHIFTS)&YEAR;
	printf(" %d/%d/%d", day_modified, month_modified, year_modified+REAL_YEAR);
}


