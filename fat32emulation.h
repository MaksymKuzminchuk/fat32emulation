#ifndef _MAIN_H_
#define _MAIN_H_

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include<math.h>
#include<ctype.h>

#define DFLT_DISK_SIZE     20000000
#define MAX_FILE_NAME      255

typedef unsigned int uint;
typedef unsigned char uchar;
    
struct FS_INFO{
    FILE *fd;  
    bool is_fat32;
    char *path_cur_dir;    
    uint lba_root_dir;
    uint lba_cur_dir;
    uint lba_fat_table;
    uint byts_per_sec;
    uint sec_per_clus;
    uint lst_alloc_clus;
};    

typedef struct{
    char name[MAX_FILE_NAME];
    uint lba_file;
    uchar attr;
} FILE_INFO;
    
// BPB size of members
#define jmpBootSize    3
#define OEMNameSize    8
#define BytsPerSecSize 2 // size of sector
#define SecPerClusSize 1 // number of sectors in cluster
#define RsvdSecCntSize 2 // BPB reserved sections and it is LBA FAT table
#define NumFATsSize    1
#define RootEntCntSize 2
#define TotSec16Size   2
#define MediaSize      1
#define FATSz16Size    2
#define SecPerTrkSize  2
#define NumHeadsSize   2
#define HiddSecSize    4 // hidden sectors before start logic tom
#define TotSec32Size   4 // total sections of physical disk
#define FATSz32Size    4 // FAT table size by sectors
#define ExtFlagsSize   2
#define FSVerSize      2
#define RootClusSize   4 // cluster of root directory (offset in data block)
#define FSInfoSize     2
#define BkBootSecSize  2 // Copy of BPB
#define ReservedSize   12
#define DrvNumSize     1
#define ReservedlSize  1
#define BootSigSize    1
#define VolIDSize      4
#define VolLabSize     11
#define FilSysTypeSize 8 // type file system (string)

// BPB structure
typedef struct {
    unsigned char jmpBoot[jmpBootSize];
    unsigned char OEMName[OEMNameSize];
    unsigned char BytsPerSec[BytsPerSecSize];
    unsigned char SecPerClus[SecPerClusSize];
    unsigned char RsvdSecCnt[RsvdSecCntSize];
    unsigned char NumFATs[NumFATsSize];
    unsigned char RootEntCnt[RootEntCntSize];
    unsigned char TotSec16[TotSec16Size];
    unsigned char Media[MediaSize];
    unsigned char FATSz16[FATSz16Size];
    unsigned char SecPerTrk[SecPerTrkSize];
    unsigned char NumHeads[NumHeadsSize];
    unsigned char HiddSec[HiddSecSize];
    unsigned char TotSec32[TotSec32Size];
    unsigned char FATSz32[FATSz32Size];
    unsigned char ExtFlags[ExtFlagsSize];
    unsigned char FSVer[FSVerSize];
    unsigned char RootClus[RootClusSize];
    unsigned char FSInfo[FSInfoSize];
    unsigned char BkBootSec[BkBootSecSize];
    unsigned char Reserved[ReservedSize];
    unsigned char DrvNum[DrvNumSize];
    unsigned char Reservedl[ReservedlSize];
    unsigned char BootSig[BootSigSize];
    unsigned char VolID[VolIDSize];
    unsigned char VolLab[VolLabSize];
    unsigned char FilSysType[FilSysTypeSize];
} BPB;

// SFN size of members
#define SFN_FILENAME    8 // File name
#define SFN_EXTEN       3 // Extension
#define SFN_ATTR        1 // Attributes
#define SFN_RSV       1 // Reserved
#define SFN_CRTTIME     3 // Create time
#define SFN_CRTDATE     2 // Create date
#define SFN_LSTACCSDATE 2 // Last access date
#define SFN_FSTCLUSHI   2 // First cluster high part 
#define SFN_LSTMODTIME  2 // Last modification time
#define SFN_LSTMODDATE  2 // Last modification date
#define SFN_FSTCLUSLO   2 // First cluster low part
#define SFN_FILESIZE    4 // File size
			  
//SFN strucutre
typedef struct{
    unsigned char filename[SFN_FILENAME];
    unsigned char exten[SFN_EXTEN];
    unsigned char attr[SFN_ATTR];
    unsigned char reserved[SFN_RSV];
    unsigned char crttime[SFN_CRTTIME];
    unsigned char crtdate[SFN_CRTDATE];
    unsigned char lstaccsdate[SFN_LSTACCSDATE];
    unsigned char fstclushi[SFN_FSTCLUSHI];
    unsigned char lstmodtime[SFN_LSTMODTIME];
    unsigned char lstmoddate[SFN_LSTMODDATE];
    unsigned char fstcluslo[SFN_FSTCLUSLO];
    unsigned char filesize[SFN_FILESIZE];
} SFN;

//LFN size of members
#define LFN_SEQ         1 // Sequence LFN 0x40 mask
#define LFN_NAME1       10
#define LFN_ATTR        1
#define LFN_TYPE        1
#define LFN_CHECKSUM    1
#define LFN_NAME2       12
#define LFN_FSTCLUSLO   2
#define LFN_NAME3       4

// LFN structure
typedef struct{
    unsigned char sequence[LFN_SEQ];
    unsigned char name1[LFN_NAME1];
    unsigned char attr[LFN_ATTR];
    unsigned char type[LFN_TYPE];
    unsigned char checksum[LFN_CHECKSUM];
    unsigned char name2[LFN_NAME2];
    unsigned char fstcluslo[LFN_FSTCLUSLO];
    unsigned char name3[LFN_NAME3];
} LFN;

// SFN attributes
#define ATTR_READONL    0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUMEID   0x08
#define ATTR_LFN        0x0F
#define ATTR_DIR        0x10
#define ATTR_ARCH       0x20
#define ATTR_LFN        0x0F


// Functions prototype
void check_disk_exists(char *disk_name);
void init_fs_info();
void read_bpb();
void write_default_bpb();
void get_default_bpb(long int file_size);
void is_disk_fat32();
uint convert_str_to_hex(const unsigned char *str, int length);
uint get_lba_root_dir();
void get_files_cur_dir();
void get_lfn_name(LFN *lfn, char *name);
void get_sfn(SFN *sfn, int *sfn_size);
uint get_lba_by_name(char *qname);
void fil_lfn_name(char *name, LFN *lfn, int lfn_size);
uint get_last_alloc_lba();
bool is_elem_in_arr(uint *arr, int arr_size,uint elem);
uint get_largest_elem(uint *arr, int arr_size);
void write_data(uint start_point, void *data, uint data_size);
uint get_lst_sfn_index();
void execute_command(char *name);
void format_disk();
void list_files();
bool create_file(char *name, uchar attr);
bool change_path(char *path);
bool change_dir(char *name);

#endif
