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



#ifndef    __ND_MBR_H__
#define    __ND_MBR_H__

#include "nand_drv_cfg.h"

#define ND_MAX_PART_COUNT		120	 									//max part count
#define ND_MBR_COPY_NUM        4    									//mbr backup count

#define ND_MBR_START_ADDRESS	0x0										//mbr start address
//#define ND_MBR_SIZE			1024									//mbr size
//#define ND_MBR_RESERVED        (ND_MBR_SIZE - 20 - (ND_MAX_PART_COUNT * 64)) //mbr reserved space
#define ND_MBR_SIZE			   (16*1024)									//mbr size
#define ND_MBR_RESERVED        (ND_MBR_SIZE - 32 - (ND_MAX_PART_COUNT * sizeof(ND_PARTITION))) //mbr reserved space

extern struct __NandDriverGlobal_t NandDriverInfo;

extern struct __NandStorageInfo_t  NandStorageInfo;

#define DiskSize  (SECTOR_CNT_OF_SINGLE_PAGE * PAGE_CNT_OF_PHY_BLK * BLOCK_CNT_OF_DIE * \
            DIE_CNT_OF_CHIP * NandStorageInfo.ChipCnt  / 1024 * DATA_BLK_CNT_OF_ZONE)


struct nand_disk{
	unsigned long size;
	unsigned long offset;
	unsigned char type;
};

/* part info */
typedef struct nand_tag_PARTITION{
	unsigned  int       addrhi;				//��ʼ��ַ, ������Ϊ��λ	
	unsigned  int       addrlo;				//	
	unsigned  int       lenhi;				//����	
	unsigned  int       lenlo;				//	
	unsigned  char      classname[16];		//���豸��	
	unsigned  char      name[16];			//���豸��	
	unsigned  int       user_type;          //�û�����	
	unsigned  int       keydata;            //�ؼ����ݣ�Ҫ����������ʧ	
	unsigned  int       ro;                 //��д����	
	unsigned  char      reserved[68];		//�������ݣ�ƥ�������Ϣ128�ֽ�
}__attribute__ ((packed))ND_PARTITION;

/* mbr info */
typedef struct nand_tag_MBR{
        unsigned  int       crc32;                      // crc 1k - 4
        unsigned  int       version;                    // �汾��Ϣ�� 0x00000100
        unsigned  char      magic[8];                   //"softw411"
        unsigned  int       copy;                       //����
        unsigned  int       index;                      //�ڼ���MBR����
        unsigned  int       PartCount;                  //��������
        unsigned  int       stamp[1];                   //����
        ND_PARTITION        array[ND_MAX_PART_COUNT];  //
        unsigned  char      res[ND_MBR_RESERVED];
}__attribute__ ((packed)) ND_MBR;

int mbr2disks(struct nand_disk* disk_array);
int NAND_PartInit(void);

#endif    //__MBR_H__
