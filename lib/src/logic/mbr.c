/*
* (C) Copyright 2007-2012
* Allwinner Technology Co., Ltd. <www.allwinnertech.com>
* Neil Peng<penggang@allwinnertech.com>
*
* See file CREDITS for list of people who contributed to this
* project.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/

#include "../include/mbr.h"
#include "../include/nand_type.h"
#include "../include/nand_drv_cfg.h"
#include "../include/nand_logic.h"
#include "../include/nand_format.h"
#include "../include/nand_scan.h"
#include "../include/nand_physic.h"
#include "../include/nfc.h"


ND_MBR *mbr=NULL;

int part_secur[NAND_MAX_PART_CNT]={0};
extern struct nand_disk nand_disk_array[NAND_MAX_PART_CNT];
extern __u32 nand_part_cnt;


typedef struct tag_CRC32_DATA
{
	__u32 CRC;				//int�Ĵ�С��32λ
	__u32 CRC_32_Tbl[256];	//�����������
}CRC32_DATA_t;

__u32 nand_calc_crc32(void * buffer, __u32 length)
{
	__u32 i, j;
	CRC32_DATA_t crc32;		//
	__u32 CRC32 = 0xffffffff; //���ó�ʼֵ
	crc32.CRC = 0;

	for( i = 0; i < 256; ++i)//��++i�����Ч��
	{
		crc32.CRC = i;
		for( j = 0; j < 8 ; ++j)
		{
			//���ѭ��ʵ���Ͼ�����"���㷨"����ȡCRC��У����
			if(crc32.CRC & 1)
				crc32.CRC = (crc32.CRC >> 1) ^ 0xEDB88320;
			else //0xEDB88320����CRC-32������ʽ��ֵ
				crc32.CRC >>= 1;
		}
		crc32.CRC_32_Tbl[i] = crc32.CRC;
	}

	CRC32 = 0xffffffff; //���ó�ʼֵ
    for( i = 0; i < length; ++i)
    {
        CRC32 = crc32.CRC_32_Tbl[(CRC32^((unsigned char*)buffer)[i]) & 0xff] ^ (CRC32>>8);
    }
    //return CRC32;
	return CRC32^0xffffffff;
}

__s32 _get_mbr(void)
{
	__u32 	i;
	__s32  mbr_get_sucess = 0;

	/*request mbr space*/
	mbr = MALLOC(sizeof(ND_MBR));
	if(mbr == NULL)
	{
		PRINT("%s : request memory fail\n",__FUNCTION__);
		return -1;
	}

	/*get mbr from nand device*/
	for(i = 0; i < ND_MBR_COPY_NUM; i++)
	{
		if(LML_Read((ND_MBR_START_ADDRESS + ND_MBR_SIZE*i)/512,ND_MBR_SIZE/512,mbr) == 0)
		{
			/*checksum*/
			if(*(__u32 *)mbr == nand_calc_crc32((__u32 *)mbr + 1,ND_MBR_SIZE - 4))
			{
				mbr_get_sucess = 1;
				break;
			}
		}
	}

	if(mbr_get_sucess)
		return 0;
	else
		return -1;

}

__s32 _free_mbr(void)
{
	if(mbr)
	{
		FREE(mbr,sizeof(ND_MBR));
		mbr = 0;
	}

	return 0;
}

int mbr2disks(struct nand_disk* disk_array)
{
	int part_cnt = 0;

#if 0
 	//���ҳ����е�LINUX�̷�
    for(part_cnt = 0; part_cnt < nand_part_cnt && part_cnt < NAND_MAX_PART_CNT; part_cnt++)
    {
            disk_array[part_cnt].offset =nand_disk_array[part_cnt].offset;
    		disk_array[part_cnt].size = nand_disk_array[part_cnt].size;
    		DBUG_MSG("part %d: offset = %x, size = %x\n", part_cnt, disk_array[part_cnt].offset, disk_array[part_cnt].size);
    }	
    return nand_part_cnt;

#else
    int storage_type =0;
    storage_type = NAND_get_storagetype();
    PRINT("storage_type=%d\n",storage_type);
    if((1 != storage_type)&&(2 != storage_type))
    {
    	//���ҳ����е�LINUX�̷�
    	PRINT("boot from nand\n");
    	for(part_cnt = 0; part_cnt < nand_part_cnt && part_cnt < NAND_MAX_PART_CNT; part_cnt++)
    	{
            disk_array[part_cnt].offset =nand_disk_array[part_cnt].offset;
    		disk_array[part_cnt].size = nand_disk_array[part_cnt].size;
    		DBUG_MSG("part %d: offset = %x, size = %x\n", part_cnt, disk_array[part_cnt].offset, disk_array[part_cnt].size);
    	}
		return nand_part_cnt;
    }   
     else
    {   
          PRINT("boot from sd/mmc\n");
          nand_part_cnt = 1;
          disk_array[0].offset = 0;
          disk_array[0].size = DiskSize;
		  return nand_part_cnt;
     }
#endif
}

int NAND_PartInit(void)
{
	int part_cnt = 0;
	int part_index;
	struct __NandPartTable_t NandPartTable;

	if(_get_mbr()){
		PRINT("get mbr error\n" );
		return part_cnt;
	}
	part_index = 0;

	for(part_cnt = 0; part_cnt<NAND_MAX_PART_CNT; part_cnt++)
		part_secur[part_index] = 0;


	//���ҳ����е�LINUX�̷�
	for(part_cnt = 0; part_cnt < mbr->PartCount && part_cnt < NAND_MAX_PART_CNT; part_cnt++)
	{
	    //if((mbr->array[part_cnt].user_type == 2) || (mbr->array[part_cnt].user_type == 0))
	    {
			MBR_DBG("The %d disk name = %s, class name = %s, disk size = %x\n", part_index, mbr->array[part_cnt].name,
						mbr->array[part_cnt].classname, mbr->array[part_cnt].lenlo);

	        nand_disk_array[part_index].offset = mbr->array[part_cnt].addrlo;
			nand_disk_array[part_index].size = mbr->array[part_cnt].lenlo;
			part_secur[part_index] = mbr->array[part_cnt].user_type;
			part_index ++;
			MBR_DBG("The %d disk offset = %x\n", part_index - 1, nand_disk_array[part_index - 1].offset);
			MBR_DBG("The %d disk size = %x\n", part_index - 1, nand_disk_array[part_index - 1].size);
	    }
	}
	nand_disk_array[part_index - 1].size = DiskSize - mbr->array[mbr->PartCount - 1].addrlo;
	_free_mbr();
	MBR_DBG("The %d disk size = %x\n", part_index - 1, nand_disk_array[part_index - 1].size);
	MBR_DBG("part total count = %d\n", part_index);

	nand_part_cnt = part_index;

	NandPartTable.magic = NAND_PART_TABLE_MAGIC;
	NandPartTable.part_cnt = part_index;
	for(part_cnt = 0; part_cnt<NandPartTable.part_cnt; part_cnt++)
	{
		NandPartTable.start_sec[part_cnt] = nand_disk_array[part_cnt].offset;
		NandPartTable.sec_cnt[part_cnt] = nand_disk_array[part_cnt].size;
		NandPartTable.part_type[part_cnt] = part_secur[part_cnt];
	}

	NAND_SetPartInfo(&NandPartTable);

	return part_index;
}

