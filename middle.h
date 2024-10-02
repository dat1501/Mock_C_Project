#ifndef _MIDDLE_H_
#define _MIDDLE_H

/*******************************************************************************
 * Include
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
 /*! @name IBIC - I2C Bus Interrupt Config Register*/
#define LENGTH_SHORTNAME        8U                        /**< Length of Short Name 8byte*/
#define LENGTH_FILEEXTENSION    3U                        /**< Length of File Name Extension 3byte*/

 /*! @name TIME - Bit Time to Convert Date and Time*/
#define	SECONDS                 0x001F                    /**< Bit MASK of Seconds */
#define SECONDS_SHIFTS          0U                        /**< Bit Shifts of Seconds*/
#define	MINUTES                 0x003F                    /**< Bit MASK of Minutes*/
#define MINUTES_SHIFTS          5U                        /**< Bit Shifts of Minutes*/
#define	HOUR                    0x001F                    /**< Bit MASK of Hour*/
#define HOUR_SHIFTS             11U                       /**< Bit Shifts of Hour*/
#define	DAY                     0x001F                    /**< Bit MASK of Day*/
#define DAY_SHIFTS              0U                        /**< Bit Shifts of Day*/
#define	MONTH                   0x000F                    /**< Bit MASK of Months*/
#define MONTH_SHIFTS            5U                        /**< Bit Shifts of Months*/
#define	YEAR                    0x007F                    /**< Bit MASK of Years*/
#define YEAR_SHIFTS             9U                        /**< Bit Shifts of Years*/
#define REAL_YEAR               1980                      /**< Plus to Year */
#define SPACE_OFFSET            32U                       /**< Bit Space offset*/
#define REVERVED_TIME           2U                        /**< Bit to Shifts of Time*/

 /*! @name ATTRIBUTE - Bit to Check File or Folder*/
#define ATTRIBUTE_FILE          0x00                      /**< Bit check Attribute by Entry is File*/
#define ATTRIBUTE_FOLDER        0x10                      /**< Bit check Attribute by Entry is Folder*/
#define ATTRIBUTE_LONGFILE      0x0F                      /**< Bit check Attribute by Entry is Long file name*/
#define START_CLUSTER_ROOT      0x00                      /**< Number of Start Cluster for Root Directory Region*/
#define ATTRIBUTE_FILE_FAT32    0x20                      /**< Bit check Attribute by Entry is File Fat32*/
#define ATTRIBUTE_VOLUME_LABLE  0x08                      /**< Bit check Attribute Volume Label*/

/*!
* @addtogroup EntryInfor_Access_Layer EntryInfor access layer
*{
*/

typedef struct _entry_infor_{
	uint8_t shortName[LENGTH_SHORTNAME];                  /**< EntryInfor Short Name */
	uint8_t shortFileExtension[LENGTH_FILEEXTENSION];     /**< EntryInfor Shor File Extension */
	uint8_t attribute;                                    /**< EntryInfor Attribute of Entry */
	uint16_t timeCreated;                                 /**< EntryInfor Time Created File */
	uint16_t timeModified;                                /**< EntryInfor Time Modified File */
	uint16_t date;                                        /**< EntryInfor Date Created File */
	uint16_t dateModified;                                /**< EntryInfor Date Modified File */
	uint32_t size;                                        /**< EntryInfor Size of File */
	uint16_t firstCluster;                                /**<EntryInfor First Cluster Start File */
	uint8_t NumberOrder;                                  /**< EntryInfor Number Order sequence select form user*/
}entryInfor_t;
/*!
 * @}
 */ /* End of EntryInfor access layer */

/* ----------------------------------------------------------------------------
   -- Structure for the Entry_Infor_Offset
   ---------------------------------------------------------------------------- */

/*!
* @brief Structure for the Entry_Infor_Offset
*{
* Defines the structure for Entry request collections 
* to handle data in Entry.
*/
typedef enum {
	Name_offset                   = 0x00,                  /**< Byte Name offset 0x00 */
	fileExtension_offset          = 0x08,                  /**< Byte File Name Extension offset 0x08 */
	attri_offset                  = 0x0B,                  /**< Byte Attribute offset 0x0B */
	time_offset                   = 0x0E,                  /**< Byte Time offset 0x0E */
	date_offset                   = 0x10,                  /**< Byte Date offset 0x10 */
	time_modified_offset          = 0x16,                  /**< Byte Time Modified offset 0x16 */
	date_modified_offset          = 0x18,                  /**< Byte Date Modified offset 0x18 */
	start_cluster_offset          = 0x1A,                  /**< Byte start Cluster offset 0x1A */
	size_offset                   = 0x1C                   /**< Byte size offset 0x1C */
}entry_Infor_Offset;
/*!
 * @}
 */ /* End of EntryOffset access layer */


/* ----------------------------------------------------------------------------
   -- Structure for the _Entry_linked_list
   ---------------------------------------------------------------------------- */
/*!
* @brief Structure for the EntryList_t
*
* Make linked list to managerment entry_Infor  
* handle entry and put data to variable in structure 
*/
typedef struct _entry_linked_list{
	entryInfor_t entryData;                               /**< Entry have Data of EntryInfor */
	struct _entry_linked_list *next;                      /**< Pointer next, pointer to next EntryInfor */
}entryList_t;


/*******************************************************************************
 * API
 ******************************************************************************/
 
/**
* @brief Function to init for FAT file system.
* 
* @return True if init succes and False if unsucces.
*/
bool middle_Init(void);

/**
* @brief Read SubDirectory 
* 
* @param [in] startCluster is the first cluster acces to SubDirectory and HeadEntry pointer to Head linked list
* @return counter num Entry in SubDirectory
*/
uint8_t middle_ReadSubDirectory(volatile uint32_t startCluster, entryList_t **HeadEntry);

/**
* @brief Read RootDirectoryRegion 
* 
* @param [in] HeadEntry pointer to Head linked list
* @return counter num Entry in SubDirectory
*/
uint8_t middle_ReadRoot(entryList_t **HeadEntry);

/**
* @brief Read File 
* 
* @param [in] startCluster is the first cluster acces file.
* @return counter num Entry in SubDirectory
*/
void middle_ReadFile(uint32_t startCluster);

/**
* @brief Find Entry if match 
* 
* @param [in] UserSelect to find Entry match with UserSect and pointer HeadEntry to acces Linked list 
* @return Entry have UserSelect
*/
entryList_t *middle_Find_Entry(uint8_t UserSelect, entryList_t **HeadEntry);

/**
* @brief Function to deinit for FAT file system 
* 
*/
void middle_Deinit(void);

#endif /*_MIDDLE_H_*/

