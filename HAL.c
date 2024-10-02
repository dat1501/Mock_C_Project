#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "HAL.h"
/*******************************************************************************
* Include
******************************************************************************/
 
FILE *fp = NULL;
uint32_t sizeSector = SECTOR_SIZE;
/*******************************************************************************
* Variables
******************************************************************************/

/*-----------------Function to init file----------------*/
bool HAL_Init(){
	
	fp = fopen("floppy.img", "rb");

	bool current = true;
	
	if(!fp){
		current = false;
	}
	return current;
}
/*----------------------Function to Read Sector------------------*/
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff){
	
	uint32_t byte_Read = 0;
	uint32_t byte_offset = index*sizeSector;
	
	if(0 == fseek(fp, byte_offset, SEEK_SET)){
		byte_Read = fread(buff, ELEMENT_SIZE, sizeSector, fp);
	}
	return byte_Read;
}
/*----------------Function to Read MulipleSector--------------*/
int32_t HAL_ReadMultiSector(uint32_t index, uint32_t num, uint8_t *buff){
	
	uint32_t byte_Read = 0;
	uint32_t byte_offset = num*sizeSector;
	
	if(0 == fseek(fp, index*sizeSector, SEEK_SET)){
		byte_Read = fread(buff, ELEMENT_SIZE, byte_offset, fp);
	}
	return byte_Read;
}
/*------------------Function to update SectorSize-------------*/
bool HAL_UpdateSectorSize(uint16_t newSectorSize){
	if(newSectorSize % SECTOR_SIZE == 0){
		sizeSector = newSectorSize;
	} else {
		return false;
	}
}
/*-----------------Function to deinit file----------------*/
void HAL_Deinit(){
	fclose(fp);
}

