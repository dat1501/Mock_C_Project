#ifndef _HAL_H_
#define _HAL_H
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SECTOR_SIZE 512U
#define ELEMENT_SIZE 1U

/*******************************************************************************
 * API
 ******************************************************************************/
 
/* @brief Init FAT file system
* 
*/
bool HAL_Init();   

/* @brief Read Sector 
* 
* @param [in] Buffer to read data and index to begin 
* @return Byte_Sector Read
*/
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff);

/* @brief Read Multiple Sector 
* 
* @param [in] Buffer to read data, index to begin read and num sequence sector
* @return Byte_Sector Read
*/
int32_t HAL_ReadMultiSector(uint32_t index, uint32_t num, uint8_t *buff);

/* @brief Update Size Sector of FAT file 
* 
* @param [in] New Sector size
*/
void Hal_UpdateSectorSize(uint32_t NewSectorSize);

/* @brief Deinit FAT file system 
* 
*/
void HAL_Deinit();

#endif /*_HAL_H_*/

