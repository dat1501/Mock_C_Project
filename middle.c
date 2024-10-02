#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "HAL.h"
#include "middle.h"
#include <string.h>
#include <stdlib.h>

/*******************************************************************************
 * Include
 ******************************************************************************/

 /*! @name FAT - Check FAT File systen is FAT12, FAT16 or FAT32*/
#define FAT12                   0xFFF                          /**<FAT12 offset*/
#define FAT16                   0xFFFF                         /**<FAT16 offset*/
#define FAT32                   0x0FFFFFFF                     /**<FAT32 offset*/

 /*! @name BOOTSECTOR - Byte offset and byte convert */
#define MAX_CLUSTER_FAT12       4087                           /**<MAX cluster in FAT12*/
#define MAX_CLUSTER_FAT16       65526                          /**<MAX clister in FAT16*/
#define BIT_SHIFT_8BIT          8U                             /**<Bit Shifts convert 8bit*/
#define BIT_SHIFT_16BIT         16U                            /**<Bit Shifts convert 16bit*/
#define BIT_SHIFT_24BIT         24U                            /**<Bit Shifts convert 24bit*/
#define BOOT_SECTOR_OFFSET      0U                             /**<Boot sector offset */
#define SIZE_ENTRY 				32U                            /**<Size of Entry */
#define CHECK_ENTRY 			0x0B                           /**<Byte offset of Entry*/
#define ENTRY_RESERVED          2U                             /**<Sector Reserved of Cluster*/
#define EVEN_SHIFTS             8U                             /**<Bit Shifts 8bit*/
#define ODD_SHIFTS              4U                             /**<Bit Shifts 4bit*/
#define CHECK_EVEN              2U                             /**<Bit Check Even 2bit*/
#define NONE_DATA               0x00                           /**<Check have Data or NULL*/
#define TOTAL_SECTOR            0x20                           /**<Byte offset total sector*/

 /*! @name CONVERT - Convert 16Bit and 32Bit for littel edian*/
#define CONVERT_16BIT(buff, index) (buff[index])|(buff[index+1]<<8)
#define CONVERT_32BIT(buff, index) (buff[index]|(buff[index+1]<<8)|(buff[index+2]<<16)|(buff[index+3]<<24))

typedef struct _BOOT_SECTOR{
	uint16_t length_Byte_Sector;                               /**<BootSector length of sector*/
	uint16_t Reserved_Sector;                                  /**<BootSector num Sector reserved*/
	uint8_t length_Cluster;                                    /**<BootSector Length of Cluster*/
	uint8_t FAT_Table;                                         /**<BootSector num of Fat table*/
	uint32_t Num_Root_Directory_Entries;                       /**<BootSector num Entry in Root Directory entries*/
	uint32_t Total_Sector;                                     /**<BootSector total Sector have in FAT file*/
	uint32_t Num_Sector_FAT_Table;                             /**<BootSector num sector have in Fat table*/
	uint32_t start_RootRegion;                                 /**<BootSector sector offset Root Directory Region*/
	uint32_t start_DataRegion;                                 /**<BootSector sector offset Data Region*/
	uint32_t end_FAT;                                          /**<BootSector End Fat check FAT12, FAT16 or FAT32*/
} boot_Sector_t;

typedef enum Boot_offset{
	leng_sector_offset         =       0x00B,                  /**<Byte offset leng sector*/
	leng_cluster_offset        =       0x00D,                  /**<Byte offset leng Cluster*/
	reserved_sector_offset     =       0x00E,                  /**<Byte offset reserved sector*/
	fat_table_offset           =       0x010,                  /**<Byte offset Fat table*/
	num_root_entries_offset    =       0x011,                  /**<Byte offset num entries in Root*/
	total_sector_offset        =       0x013,                  /**<Byte offset total sector in FAT file*/
	num_table_fatTable_offset  =       0x016,                  /**<Byte offset num sector of Fat table*/
	num_table_fat32Table_offset=       0x024,                  /**<Byte offset num sector of Fat32 table*/
	num_start_Root_fat32_offset=       0x02C                   /**<Byte offset num start Root in Fat32*/
}boot_Sector_offset_t;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
* @brief Handle Boot Sector 
* 
* @param [in] Buffer to read data
*/
static void middle_HandleBootSector(uint8_t *buff);

/**
* @brief Convert FAT table Raw to standard 
* 
*/
static void middle_Convert_FAT_Table();

/**
* @brief Create Node 
* 
* @param [in] Byte Entry, Entry , buffer, CountEntry, put data in buffer to Entry
* @return Entry
*/
static entryList_t *middle_CreateNode(uint32_t byteEntry, entryList_t* Entry, uint8_t *buff, uint8_t CountEntry);

/**
* @brief Add New Entry to Linked List 
* 
* @param [in] Entry and pointer HeadEntry
*/
static void middle_Add_EntryList(entryList_t *Entry, entryList_t **HeadEntry);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 
static boot_Sector_t g_Boot_Sector;
static uint32_t *FATTable = NULL;

 /*******************************************************************************
 * Variables
 ******************************************************************************/

/*-----------------Function to init file-------------*/
bool middle_Init(){
	
	uint8_t status;
	uint8_t *buff = NULL;
	
	status = HAL_Init();
	
	if(status) {
		buff = (uint8_t*)malloc(SECTOR_SIZE*sizeof(uint8_t));
		HAL_ReadSector(BOOT_SECTOR_OFFSET, buff);                              /*Read Boot Sector*/
		middle_HandleBootSector(buff);                                         /*Handle Boot Sector*/
		free(buff);
		HAL_UpdateSectorSize(g_Boot_Sector.length_Byte_Sector);                /*Update Size Sector */
	}
	middle_Convert_FAT_Table();                                                /*Convert FAT table RAW to Standard*/
	return status;
}
/*-----------------------------Function to Read Root Directory Region--------------------*/
uint8_t middle_ReadRoot(entryList_t **HeadEntry) {
	
	uint32_t Root_Size = g_Boot_Sector.Num_Root_Directory_Entries*SIZE_ENTRY;
	uint8_t *RootData;
	uint32_t countSector = 0;
	uint32_t numSector = 0;
	uint32_t byteOffset = 0;
	uint32_t countEntry = 0;
	uint8_t entry = 0;
	uint32_t SectorOffset = 0;
	uint32_t sta_Cluster = 0;
	uint32_t startCluster = 0;
	uint32_t countByteLongFileName = 0;
	uint8_t countLongFileName = 0;
	
	
	if(g_Boot_Sector.end_FAT == FAT32){                                        /*Handle if FAT file system is FAT32*/
		Root_Size = g_Boot_Sector.length_Byte_Sector*g_Boot_Sector.length_Cluster;
		SectorOffset = g_Boot_Sector.start_RootRegion + g_Boot_Sector.Reserved_Sector + g_Boot_Sector.FAT_Table*g_Boot_Sector.Num_Sector_FAT_Table - ENTRY_RESERVED;
		RootData = (uint8_t*)malloc(Root_Size*sizeof(uint8_t));
		startCluster = g_Boot_Sector.start_RootRegion;
		sta_Cluster = SectorOffset;
		
		while(sta_Cluster != FAT32){
			SectorOffset = startCluster + g_Boot_Sector.Reserved_Sector + g_Boot_Sector.FAT_Table*g_Boot_Sector.Num_Sector_FAT_Table - ENTRY_RESERVED;
			byteOffset = HAL_ReadMultiSector(SectorOffset, g_Boot_Sector.length_Cluster, RootData);
			
			for(countEntry = 0; countEntry<Root_Size; countEntry+=SIZE_ENTRY){
				if((RootData[CHECK_ENTRY + countEntry] != ATTRIBUTE_LONGFILE) && (RootData[Name_offset+countEntry] != NONE_DATA)){
					if(RootData[CHECK_ENTRY + countEntry] != ATTRIBUTE_VOLUME_LABLE){
						entryList_t* newEntry = (entryList_t*)malloc(sizeof(entryList_t));
						entry++;
						middle_CreateNode(countEntry, newEntry, RootData, entry);
						middle_Add_EntryList(newEntry, HeadEntry);	               /*Add newEntry to linked list*/	
					}
				}
			}
			startCluster = FATTable[startCluster];
			sta_Cluster = startCluster;
		}
	} else {                                                                   /*Handle if FAT file system is FAT12 or FAT16*/
		
		RootData = (uint8_t*)malloc(Root_Size*sizeof(uint8_t));
		byteOffset = HAL_ReadMultiSector(g_Boot_Sector.start_RootRegion, (Root_Size/g_Boot_Sector.length_Byte_Sector), RootData);
		
		for(countSector = 0; countSector<(Root_Size/g_Boot_Sector.length_Byte_Sector); countSector++){
			if(RootData[countSector*g_Boot_Sector.length_Byte_Sector] == 0){
				numSector = countSector;
				countSector = Root_Size/g_Boot_Sector.length_Byte_Sector;
			}
		}
		
		for(countEntry = 0; countEntry<numSector*g_Boot_Sector.length_Byte_Sector; countEntry+=SIZE_ENTRY){
			if(RootData[CHECK_ENTRY + countEntry] != ATTRIBUTE_LONGFILE && RootData[Name_offset+countEntry] != NONE_DATA){
				
				entryList_t* newEntry = (entryList_t*)malloc(sizeof(entryList_t));
				entry++;

				middle_CreateNode(countEntry, newEntry, RootData, entry);
				middle_Add_EntryList(newEntry, HeadEntry);	                  /*Add newEntry to linked list*/
			}
		}
	}
	
	free(RootData);
	return entry;                                                             /*Return number Entry have in Root*/
}
/*--------------------------Function to Read SubDirectory-------------------------*/
uint8_t middle_ReadSubDirectory(volatile uint32_t startCluster, entryList_t **HeadEntry) {
	
	uint32_t SubDirectory_Size = 0;;
	uint8_t *SubDirectoryData;
	uint32_t countCluster = 0;
	uint32_t countSector = 0;
	uint32_t byte_Offset_SubDirectory = 0;
	uint32_t byte_Offset = 0;
	uint32_t countEntry = 0;
	uint8_t entry = 0;
	uint32_t sta_Cluster = 0;
	uint32_t sta_Fat = 0;
	
	sta_Cluster = startCluster;
	sta_Fat = g_Boot_Sector.end_FAT;                                       /*Status FAT File is FAT12, FAT16 or FAT32*/
	
	while(startCluster != sta_Fat){	                                       /*Check End_cluster */
		if(sta_Fat == FAT32){
			byte_Offset_SubDirectory = startCluster + g_Boot_Sector.Reserved_Sector + g_Boot_Sector.FAT_Table*g_Boot_Sector.Num_Sector_FAT_Table - ENTRY_RESERVED*g_Boot_Sector.length_Cluster;
		}else{
			byte_Offset_SubDirectory = startCluster*g_Boot_Sector.length_Cluster + g_Boot_Sector.start_DataRegion - ENTRY_RESERVED*g_Boot_Sector.length_Cluster;
		}
		
		
		SubDirectory_Size = g_Boot_Sector.length_Byte_Sector*g_Boot_Sector.length_Cluster;
		SubDirectoryData = (uint8_t*)malloc(SubDirectory_Size*sizeof(uint8_t));
		byte_Offset = HAL_ReadMultiSector(byte_Offset_SubDirectory,g_Boot_Sector.length_Cluster, SubDirectoryData);
								
		for(countEntry = 0; countEntry<SubDirectory_Size; countEntry+=SIZE_ENTRY){
			if(SubDirectoryData[CHECK_ENTRY + countEntry] != ATTRIBUTE_LONGFILE && SubDirectoryData[Name_offset+countEntry] != NONE_DATA){
				if(SubDirectoryData[CHECK_ENTRY + countEntry] != ATTRIBUTE_VOLUME_LABLE){
					entryList_t* newEntry = (entryList_t*)malloc(sizeof(entryList_t));
					entry++;
					middle_CreateNode(countEntry, newEntry, SubDirectoryData, entry);
					middle_Add_EntryList(newEntry, HeadEntry);	
				}			
			}
			if(SubDirectoryData[Name_offset+countEntry] == NONE_DATA){
				countEntry = SubDirectory_Size;
			}
		}
		startCluster = FATTable[startCluster];                          /*Take next Cluster form FATtable*/
	}			
	
	free(SubDirectoryData);

	return entry;                                                       /*Return number Entry have in SubDirectory*/
}
/*------------------------------Function to Read file ----------------------*/
void middle_ReadFile(uint32_t startCluster) {
	
	uint32_t byteOffset = 0;
	uint8_t Read_Data[g_Boot_Sector.length_Byte_Sector*g_Boot_Sector.length_Cluster];
	uint32_t countByte = 0;
	uint32_t sta_Fat = 0;
	
	sta_Fat = g_Boot_Sector.end_FAT;
	
	if(startCluster == START_CLUSTER_ROOT){
		printf("\nERROR: Emty.");
		return;
	}
	
	while(startCluster != sta_Fat){                                    /*Check end of Cluster*/
		if(sta_Fat == FAT32){
			byteOffset = startCluster + g_Boot_Sector.Reserved_Sector + g_Boot_Sector.FAT_Table*g_Boot_Sector.Num_Sector_FAT_Table - ENTRY_RESERVED*g_Boot_Sector.length_Cluster;
		}else{
			byteOffset = startCluster*g_Boot_Sector.length_Cluster + g_Boot_Sector.start_DataRegion - ENTRY_RESERVED*g_Boot_Sector.length_Cluster;
		}
		
		HAL_ReadMultiSector(byteOffset,g_Boot_Sector.length_Cluster, Read_Data);
		printf("%s", Read_Data);
		printf("\n");
		startCluster = FATTable[startCluster];
	}
}
/*--------------------------Function to add EntryNode to linked list---------------------*/
static void middle_Add_EntryList(entryList_t *Entry, entryList_t **HeadEntry){
	
	entryList_t* current_Entry = *HeadEntry;
	
	if(*HeadEntry == NULL){                                               /*Add entry to Head linked list*/
		*HeadEntry = Entry;
		Entry->next = NULL;
		return;
	}
	while(current_Entry->next!=NULL){

		current_Entry = current_Entry->next;
	}
	current_Entry->next = Entry;                                         /*Add entry to Tail linked list*/
	Entry->next = NULL;
}
/*--------------------Function to find EntryNode in linked list and return Entry if match------------*/
entryList_t *middle_Find_Entry(uint8_t UserSelect, entryList_t **HeadEntry){
	
	entryList_t* current_Entry = *HeadEntry;
	
	while(current_Entry != NULL){
		if(current_Entry->entryData.NumberOrder == UserSelect){
			return current_Entry;                                         /*Return Entry if Match*/
		}
		current_Entry = current_Entry->next;
	}
	return NULL;                                                          /*Return Null if don't Match*/
}
/*------------------------Function to Handle BootSetor------------------------------*/
static void middle_HandleBootSector(uint8_t *buff){
	
	HAL_ReadSector(BOOT_SECTOR_OFFSET, buff);
	
	g_Boot_Sector.length_Byte_Sector = CONVERT_16BIT(buff, leng_sector_offset);                         /*Convert 2byte leng Sector put to BootSector*/
	g_Boot_Sector.length_Cluster = buff[leng_cluster_offset];                                           /*Put lengCluster to BootSector*/
	g_Boot_Sector.Reserved_Sector = CONVERT_16BIT(buff, reserved_sector_offset);                        /*Convert 2byte Reserved put to BootSector*/
	g_Boot_Sector.FAT_Table = buff[fat_table_offset];                                                   /*Put num Sector FAT table to BootSector*/
	g_Boot_Sector.Num_Root_Directory_Entries = CONVERT_16BIT(buff, num_root_entries_offset);            /*Convert 2byte num entries put to BootSector*/
	g_Boot_Sector.Total_Sector = CONVERT_16BIT(buff, total_sector_offset);                              /*Convert 2byte total sector put to BootSector*/
	if(g_Boot_Sector.Total_Sector == NONE_DATA){ 
		g_Boot_Sector.Total_Sector = CONVERT_32BIT(buff, TOTAL_SECTOR);                                 /*Convert 4byte total sector put to BootSector*/
	}
	g_Boot_Sector.Num_Sector_FAT_Table = CONVERT_16BIT(buff, num_table_fatTable_offset);                /*Convert 2byte num Sector Fat table put to BootSector*/
	g_Boot_Sector.start_RootRegion = g_Boot_Sector.FAT_Table*g_Boot_Sector.Num_Sector_FAT_Table + g_Boot_Sector.Reserved_Sector; /*Num sector to Start RootRegion*/
	g_Boot_Sector.start_DataRegion = g_Boot_Sector.start_RootRegion + (g_Boot_Sector.Num_Root_Directory_Entries*SIZE_ENTRY/g_Boot_Sector.length_Byte_Sector); /*Num Sector to Start DataRegion*/
	if(((g_Boot_Sector.Total_Sector - g_Boot_Sector.start_RootRegion)/g_Boot_Sector.length_Cluster) <= MAX_CLUSTER_FAT12){
		g_Boot_Sector.end_FAT = FAT12; 
	} else if((((g_Boot_Sector.Total_Sector - g_Boot_Sector.start_RootRegion)/g_Boot_Sector.length_Cluster) > MAX_CLUSTER_FAT12) && (((g_Boot_Sector.Total_Sector - g_Boot_Sector.start_RootRegion)/g_Boot_Sector.length_Cluster) < MAX_CLUSTER_FAT16)){
		g_Boot_Sector.end_FAT = FAT16; 
	} else{
		g_Boot_Sector.end_FAT = FAT32;
	}
	
	if(g_Boot_Sector.end_FAT == FAT32){
		g_Boot_Sector.start_RootRegion = CONVERT_32BIT(buff, num_start_Root_fat32_offset);
		g_Boot_Sector.Num_Sector_FAT_Table = CONVERT_32BIT(buff, num_table_fat32Table_offset);
	}
}
/*-------------------------Fucntion to create new EntryNode-------------------------*/
static entryList_t *middle_CreateNode(uint32_t byteEntry, entryList_t* Entry, uint8_t *buff, uint8_t CountEntry){
	
	uint8_t countbyte = 0;
	
	for(countbyte = 0; countbyte<LENGTH_SHORTNAME; countbyte++){
		Entry->entryData.shortName[countbyte] = buff[countbyte+Name_offset+byteEntry];                      /*Put data shortName to struct Entry*/
	}
	for(countbyte = 0; countbyte<LENGTH_FILEEXTENSION; countbyte++){
		Entry->entryData.shortFileExtension[countbyte] = buff[countbyte+fileExtension_offset+byteEntry];    /*Put data FileExtension to struct Entry*/
	}
	
	Entry->entryData.attribute = buff[attri_offset+byteEntry];                                              /*Put data attribute to struct Entry*/
	Entry->entryData.timeCreated = CONVERT_16BIT(buff, (time_offset + byteEntry));                          /*Put data Time created to struct Entry*/
	Entry->entryData.date = CONVERT_16BIT(buff, (date_offset+byteEntry));                                   /*Put data Date creared to struct Entry*/
	Entry->entryData.timeModified = CONVERT_16BIT(buff, (time_modified_offset+byteEntry));                  /*Put data Time modified to struct Entry*/
	Entry->entryData.dateModified = CONVERT_16BIT(buff, (date_modified_offset+byteEntry));                  /*Put data Date modified to struct Entry*/	
	Entry->entryData.size = CONVERT_32BIT(buff, (size_offset+byteEntry));                                   /*Put data Size to struct Entry*/
	Entry->entryData.firstCluster = CONVERT_16BIT(buff, (start_cluster_offset+byteEntry));                  /*Put data First Cluster to struct Entry*/
	Entry->entryData.NumberOrder = CountEntry;                                                              /*Put data NumberOder to struct Entry*/
}

/*----------------Function to convert Raw FatTable to Real FatTable---------- */
static void middle_Convert_FAT_Table(){

	uint32_t countByte = 0;
	uint16_t value = 0;
	uint16_t value1 = 0;
	uint16_t valueFatTable = 0;
	uint8_t num = 0;
	uint32_t sizeof_FAT_Table = 0;
	uint32_t sizeofFAT = g_Boot_Sector.Num_Sector_FAT_Table*g_Boot_Sector.length_Byte_Sector;
	uint8_t *FAT_Table_Raw_Data;
	
	FAT_Table_Raw_Data = (uint8_t*)malloc(sizeofFAT*sizeof(uint8_t));
	HAL_ReadMultiSector(g_Boot_Sector.Reserved_Sector, g_Boot_Sector.Num_Sector_FAT_Table, FAT_Table_Raw_Data);

	for(countByte = 0; countByte<g_Boot_Sector.Num_Sector_FAT_Table; countByte++){           /*Check how many sector have Data*/
		if(FAT_Table_Raw_Data[countByte*g_Boot_Sector.length_Byte_Sector] == NONE_DATA){
			num = countByte;
			countByte = g_Boot_Sector.Num_Sector_FAT_Table;
		}
	}
	sizeofFAT = num*g_Boot_Sector.length_Byte_Sector;

	
	FATTable = (uint32_t*)malloc(sizeofFAT*sizeof(uint32_t));
	
	if(g_Boot_Sector.end_FAT == FAT12){                                                       /*Convert FAT table Raw to standard 12Bit*/
		for(countByte = 0; countByte<sizeofFAT; countByte++){
			if(countByte % CHECK_EVEN == 0){
				value = FAT_Table_Raw_Data[(int)((3*countByte)/2)];
				value1 = FAT_Table_Raw_Data[1+(int)((3*countByte)/2)];
				valueFatTable = ((value1<<EVEN_SHIFTS)&0x0F00)|value;
			}else{
				value = FAT_Table_Raw_Data[(int)((3*countByte)/2)];
				value1 = FAT_Table_Raw_Data[1+(int)((3*countByte)/2)];
				valueFatTable = (value>>ODD_SHIFTS&0x0F)|((value1<<ODD_SHIFTS)&0x0FF0);
			}
			FATTable[countByte] = valueFatTable;
		}
	} else if (g_Boot_Sector.end_FAT == FAT16){                                                /*Convert FAT table Raw to standard 16Bit*/
		for(countByte = 0; countByte<sizeofFAT; countByte+=2){
			FATTable[countByte/2] = CONVERT_16BIT(FAT_Table_Raw_Data,countByte);
		}
	} else {                                                                                   /*Convert FAT table Raw to standard 32Bit*/
		for(countByte = 0; countByte<sizeofFAT; countByte+=4){
			FATTable[countByte/4] = CONVERT_32BIT(FAT_Table_Raw_Data, countByte);
		}
	}
	free(FAT_Table_Raw_Data);
}

/*----------------Function to free linked list--------------*/
void middle_FreeEntryList(entryList_t **HeadEntry){
	
	entryList_t *Entry = *HeadEntry;
	entryList_t *NextEntry = Entry->next;
	
	while(NextEntry != NULL){
		*HeadEntry = NextEntry;
		NextEntry = NextEntry->next;
		free(Entry);
		Entry = *HeadEntry;
	}
	*HeadEntry = NULL;
	free(NextEntry);
}
/*------------Function to Deinit FAT File system------------*/
void middle_Deinit(void){
	
	HAL_Deinit();
}

