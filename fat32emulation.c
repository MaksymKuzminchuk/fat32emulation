#include"fat32emulation.h"
 
struct FS_INFO fs_info;
BPB bpb;
FILE_INFO *file_info_cur_dir;
int num_files_cur_dir = 0;
int main(int argc, char **argv)
{
    char command_line[1000];
    char disk_name[50];
    char cwd[1000];

    if(argc > 1)
    {
	strcpy(disk_name, argv[1]);
    }
    else
    {
	printf("Missing file operand\n");
	exit(EXIT_FAILURE);
    }

    getcwd(cwd, sizeof(cwd));
    check_disk_exists(disk_name);
    init_fs_info();
    get_files_cur_dir();

    // Main loop
    while(1)
    {
	printf("\x1b[36m%s%s: \x1b[0m", cwd, fs_info.path_cur_dir);
	fflush(stdout);
	fgets(command_line, sizeof(command_line), stdin);
	execute_command(command_line);
    }
}

void check_disk_exists(char *disk_name)
{
    // Check if the file exists
    if(access(disk_name, F_OK) == 0)
    {
        fs_info.fd = fopen(disk_name, "r+b");
	if(fs_info.fd == NULL)
	{
	    perror("Failed open file");
	    exit(EXIT_FAILURE);
	}
	read_bpb();
        is_disk_fat32();
    }	
    // then create new one
    else
    {
	fs_info.fd = fopen(disk_name, "wb");
	if(fs_info.fd == NULL)
	{
	    perror("Failed create file");
	    exit(EXIT_FAILURE);
	}
	write_default_bpb();
    }
}
void init_fs_info()
{
    fs_info.lba_root_dir = get_lba_root_dir();
    fs_info.lba_cur_dir = fs_info.lba_root_dir;
    fs_info.lba_fat_table = convert_str_to_hex(bpb.RsvdSecCnt, RsvdSecCntSize);
    fs_info.sec_per_clus = convert_str_to_hex(bpb.SecPerClus, SecPerClusSize);
    fs_info.byts_per_sec = convert_str_to_hex(bpb.BytsPerSec, BytsPerSecSize);
    fs_info.path_cur_dir = malloc(1000);
    fs_info.path_cur_dir[0] = '/';
}
void read_bpb()
{
    fread(bpb.jmpBoot, sizeof(uchar), jmpBootSize, fs_info.fd);
    fread(bpb.OEMName, sizeof(uchar), OEMNameSize, fs_info.fd);
    fread(bpb.BytsPerSec, sizeof(uchar), BytsPerSecSize, fs_info.fd);
    fread(bpb.SecPerClus, sizeof(uchar), SecPerClusSize, fs_info.fd);
    fread(bpb.RsvdSecCnt, sizeof(uchar), RsvdSecCntSize, fs_info.fd);
    fread(bpb.NumFATs, sizeof(uchar), NumFATsSize, fs_info.fd);
    fread(bpb.RootEntCnt, sizeof(uchar), RootEntCntSize, fs_info.fd);
    fread(bpb.TotSec16, sizeof(uchar), TotSec16Size, fs_info.fd);
    fread(bpb.Media, sizeof(uchar), MediaSize, fs_info.fd);
    fread(bpb.FATSz16, sizeof(uchar), FATSz16Size, fs_info.fd);
    fread(bpb.SecPerTrk, sizeof(uchar), SecPerTrkSize, fs_info.fd);
    fread(bpb.NumHeads, sizeof(uchar), NumHeadsSize, fs_info.fd);
    fread(bpb.HiddSec, sizeof(uchar), HiddSecSize, fs_info.fd);
    fread(bpb.TotSec32, sizeof(uchar), TotSec32Size, fs_info.fd);
    fread(bpb.FATSz32, sizeof(uchar), FATSz32Size, fs_info.fd);
    fread(bpb.ExtFlags, sizeof(uchar), ExtFlagsSize, fs_info.fd);
    fread(bpb.FSVer, sizeof(uchar), FSVerSize, fs_info.fd);
    fread(bpb.RootClus, sizeof(uchar), RootClusSize, fs_info.fd);
    fread(bpb.FSInfo, sizeof(uchar), FSInfoSize, fs_info.fd);
    fread(bpb.BkBootSec, sizeof(uchar), BkBootSecSize, fs_info.fd);
    fread(bpb.Reserved, sizeof(uchar), ReservedSize, fs_info.fd);
    fread(bpb.DrvNum, sizeof(uchar), DrvNumSize, fs_info.fd);
    fread(bpb.Reservedl, sizeof(uchar), ReservedlSize, fs_info.fd);
    fread(bpb.BootSig, sizeof(uchar), BootSigSize, fs_info.fd);
    fread(bpb.VolID, sizeof(uchar), VolIDSize, fs_info.fd);
    fread(bpb.VolLab, sizeof(uchar), VolLabSize, fs_info.fd);
    fread(bpb.FilSysType, sizeof(uchar), FilSysTypeSize, fs_info.fd);
} 
void write_default_bpb()
{
    ftruncate(fileno(fs_info.fd), DFLT_DISK_SIZE);
    get_default_bpb(DFLT_DISK_SIZE);
    write_data(0, &bpb, sizeof(BPB));

    // Mark root dir
    SFN sfn = {0};
    sfn.filename[0] = 'R';
    sfn.filename[1] = 'O';
    sfn.filename[2] = 'O';
    sfn.filename[3] = 'T';
    sfn.attr[0] = ATTR_VOLUMEID;
    uint start_root_dir = get_lba_root_dir() * 512;

    write_data(start_root_dir, &sfn, sizeof(sfn));
    fs_info.is_fat32 = true;
}
void get_default_bpb(long int file_size)
{
    memset(&bpb, 0, sizeof(BPB));
    bpb.jmpBoot[0] = 0xEB;
    bpb.jmpBoot[1] = 0x58;
    bpb.jmpBoot[2] = 0x90;
    bpb.BytsPerSec[0] = 0x00;
    bpb.BytsPerSec[1] = 0x02;

    bpb.SecPerClus[0] = 0x10;

    bpb.RsvdSecCnt[0] = 0x40;
    bpb.RsvdSecCnt[1] = 0x19;

    uint tot_sec = file_size / 512;
    bpb.TotSec32[0] = tot_sec;
    bpb.TotSec32[1] = tot_sec >> 8;
    bpb.TotSec32[2] = tot_sec >> 16;
    bpb.TotSec32[3] = tot_sec >> 24;

    bpb.FATSz32[0] = 0x02;
    bpb.FATSz32[1] = 0x00;
    bpb.FATSz32[2] = 0x00;
    bpb.FATSz32[3] = 0x00;

    bpb.RootClus[0] = 0x02;
    bpb.RootClus[1] = 0x00;
    bpb.RootClus[2] = 0x00;
    bpb.RootClus[3] = 0x00;


    bpb.FilSysType[0] = 'F';
    bpb.FilSysType[1] = 'A';
    bpb.FilSysType[2] = 'T';
    bpb.FilSysType[3] = '3';
    bpb.FilSysType[4] = '2';
    bpb.FilSysType[5] = ' ';
    bpb.FilSysType[6] = ' ';
    bpb.FilSysType[7] = ' ';
}

void is_disk_fat32()
{
    char fat32[5] = {'F','A','T','3','2'};
    for(int i = 0; i < 5; i++)
    {
	if(bpb.FilSysType[i] != fat32[i])
	{
	    
	    fs_info.is_fat32 = false;
	    return;
	}
    }	
    fs_info.is_fat32 =  true;
    return;
}

uint convert_str_to_hex(const unsigned char* str, int length)
{
    uint ret = 0;
    for(int i = length - 1; i >= 0; i--)
    {
	ret |= (str[i] << (8 * i));
    }
    return ret;
}

uint get_lba_root_dir()
{
    uint lba_root_dir;
    uint rsvd_sec_nt = convert_str_to_hex(bpb.RsvdSecCnt, RsvdSecCntSize);
    uint fat_sz_32 = convert_str_to_hex(bpb.FATSz32, FATSz32Size);
    uint root_clus = convert_str_to_hex(bpb.RootClus, RootClusSize);
    uint sec_per_clus = convert_str_to_hex(bpb.SecPerClus, SecPerClusSize);

    lba_root_dir = rsvd_sec_nt  + (fat_sz_32 * 2) + ((root_clus - 2) * sec_per_clus);
    
    return lba_root_dir;
}

void get_files_cur_dir()
{
    // Total numb with deleted files
    int num_sfn = get_lst_sfn_index();
    SFN sfn[num_sfn];

    // Not including deleted files
    int valid_num_sfn = 0; 
    get_sfn(sfn, &valid_num_sfn); 
    free(file_info_cur_dir);
    file_info_cur_dir = malloc(valid_num_sfn * sizeof(FILE_INFO));
    num_files_cur_dir = 0;

    char name_parts[5][14];
    int lfn_seq = 0;
    int name_parts_len = 0;

    for(int i = 0; i < num_sfn; i++)
    {
	switch(sfn[i].attr[0])
	{
	    case ATTR_LFN:
		{
		    if (lfn_seq == 0)
		    {
			LFN* lfn = (LFN*)&sfn[i];
			lfn_seq = lfn->sequence[0] -  0x40;
			char name[27] = {'\0'};
			get_lfn_name(lfn, name);
			name_parts_len = 0;
			strcpy(name_parts[name_parts_len], name);

		    }
		    else
		    {
			name_parts_len++;
			LFN* lfn = (LFN*)&sfn[i];
			char name[27] = {'\0'};
			get_lfn_name(lfn, name);
			strcpy(name_parts[name_parts_len], name);
		    }

		    lfn_seq--;
		    break;
		}
	    case ATTR_ARCH:
	    case ATTR_DIR:
		{
		    char name[MAX_FILE_NAME] = {'\0'};
		    for(int i = name_parts_len; i >= 0; i--)
		    {
			strcat(name, name_parts[i]);		   
		    }
		    strcpy(file_info_cur_dir[num_files_cur_dir].name, name);

		    file_info_cur_dir[num_files_cur_dir].lba_file = get_lba_by_name(name);

		    if(sfn[i].attr[0] == ATTR_DIR)
			file_info_cur_dir[num_files_cur_dir].attr = ATTR_DIR;
		    else
			file_info_cur_dir[num_files_cur_dir].attr = ATTR_ARCH;
			
		    num_files_cur_dir += 1;
		    break;
		}
	}
    }
}

void get_lfn_name(LFN *lfn, char* name)
{
    int name_len = 0;
    int is_end = 0;

    for(int j = 0; j < LFN_NAME1; name_len++,j +=2)
    {
	if(lfn->name1[j] == 0x20)
	{
	    name[name_len] = '\0';
	    is_end = 1;
	    break;
	}
	name[name_len] = lfn->name1[j];
    }

    for(int j = 0; j < LFN_NAME2; name_len++, j+=2)
    {
	if(lfn->name2[j] == 0x20 || is_end)
	{
	    name[name_len] = '\0';
	    is_end = 1;
	    break;
	}
	name[name_len] = lfn->name2[j];
    }

    for(int j = 0; j < LFN_NAME3; name_len++,j+=2)
    {
	if(lfn->name3[j] == 0x20 || is_end)
	{
	    name[name_len] = '\0';
	    break;
	}
	name[name_len] = lfn->name3[j];
    }
}

void get_sfn(SFN* sfn, int *sfn_size)
{
    fseek(fs_info.fd, fs_info.lba_cur_dir * fs_info.byts_per_sec, SEEK_SET);
    for(int i = 0; i < 100; i++)
    {
	uchar fst_byte_sfn;
	fread(&fst_byte_sfn, 1, 1, fs_info.fd);
	fseek(fs_info.fd, -1, SEEK_CUR);

	// End of sfn structs
	if(fst_byte_sfn == 0x00)
	{
	    break;
	}
	// File was deleted
	else if(fst_byte_sfn == 0xE5 || fst_byte_sfn == 0x2E)
	{
	    fseek(fs_info.fd, sizeof(SFN), SEEK_CUR);
	}
	else
	{
            fread(sfn->filename, sizeof(uchar), SFN_FILENAME, fs_info.fd );
	    fread(sfn->exten, sizeof(uchar), SFN_EXTEN, fs_info.fd);
	    fread(sfn->attr, sizeof(uchar), SFN_ATTR, fs_info.fd);
	    fread(sfn->reserved, sizeof(uchar), SFN_RSV, fs_info.fd);
	    fread(sfn->crttime, sizeof(uchar), SFN_CRTTIME, fs_info.fd);
	    fread(sfn->crtdate, sizeof(uchar), SFN_CRTDATE, fs_info.fd);
	    fread(sfn->lstaccsdate, sizeof(uchar), SFN_LSTACCSDATE, fs_info.fd);
	    fread(sfn->fstclushi, sizeof(uchar), SFN_FSTCLUSHI, fs_info.fd);
	    fread(sfn->lstmodtime, sizeof(uchar), SFN_LSTMODTIME, fs_info.fd);
	    fread(sfn->lstmoddate, sizeof(uchar), SFN_LSTMODDATE, fs_info.fd);
	    fread(sfn->fstcluslo, sizeof(uchar), SFN_FSTCLUSLO, fs_info.fd);
	    fread(sfn->filesize, sizeof(uchar), SFN_FILESIZE, fs_info.fd);

	    sfn++;
	    *sfn_size+=1;
	}
    }
}
uint get_lst_sfn_index()
{
    uint index_end = 0;
    fseek(fs_info.fd, fs_info.lba_cur_dir * fs_info.byts_per_sec, SEEK_SET);
    int i =0;
    while(1)
    {
	uchar fst_byte_sfn;
	fread(&fst_byte_sfn, 1, 1, fs_info.fd);
	fseek(fs_info.fd, -1, SEEK_CUR);

	if(fst_byte_sfn == 0x00)
	{
	    // End of sfn structs
	    index_end = i;
	    break;
	}
	else
	{
	    // File was deleted, pass the struct
	    fseek(fs_info.fd, sizeof(SFN), SEEK_CUR);
            i++;
	}
    }

    return index_end; 
}
uint get_lba_by_name(char *qname)
{
    SFN sfn[100];
    int sfn_size = 0; 
    get_sfn(sfn, &sfn_size); 

    char part_name[5][26];
    int lfn_seq = 0;
    int len_part_name = 0;

    for(int i = 0; i < sfn_size; i++)
    {
	switch(sfn[i].attr[0])
	{
	    case ATTR_LFN:
		{
		    if (lfn_seq == 0)
		    {
			LFN* lfn = (LFN*)&sfn[i];
			lfn_seq = lfn->sequence[0] -  0x40;
			char name[27] = {'\0'};
			get_lfn_name(lfn, name);
			len_part_name = 0;
			strcpy(part_name[len_part_name], name);
		    }
		    else
		    {
			len_part_name++;
			LFN* lfn = (LFN*)&sfn[i];
			char name[27] = {'\0'};
			get_lfn_name(lfn, name);
			strcpy(part_name[len_part_name], name);
		    }

		    lfn_seq--;
		    break;
		}
	    case ATTR_ARCH:
	    case ATTR_DIR:
		{
		    char name[MAX_FILE_NAME] = {'\0'};
		    for(int i = len_part_name; i >= 0; i--)
		    {
			strcat(name, part_name[i]);
		    }
		    if(!strcmp(qname, name))
		    {
			uint dir_clus = 
			(sfn[i].fstclushi[1] << 24) | (sfn[i].fstclushi[0] << 16)
		       	| (sfn[i].fstcluslo[1] << 8) | sfn[i].fstcluslo[0];

			uint lba_dir = fs_info.lba_root_dir + ((dir_clus - 2)*fs_info.sec_per_clus);
			return lba_dir;
		    }
		    break;
		}
	}

    }
}


void write_data(uint start_point, void* data, uint data_size)
{
  fseek(fs_info.fd, start_point, SEEK_SET);
  fwrite(data, sizeof(uchar), data_size, fs_info.fd);  
}
void fil_lfn_name(char* name, LFN* lfn, int lfn_size)
{
    int is_end = 0;
    int name_len = strlen(name) + 1;
    int index_name = 0;

    for(int k = lfn_size - 1; k >= 0; k--)
    {
	lfn[k].sequence[0] = 0x01;
	for(int j = 0; j < LFN_NAME1 && index_name < name_len;index_name++,j +=2)
	{
	    lfn[k].name1[j] = name[index_name];
	}

	for(int j = 0; j < LFN_NAME2 && index_name < name_len; index_name++, j+=2)
	{
	    lfn[k].name2[j] = name[index_name];
	}

	for(int j = 0; j < LFN_NAME3 && index_name < name_len; index_name++,j+=2)
	{
	    lfn[k].name3[j] = name[index_name];
	}
    }
}


uint get_last_alloc_lba()
{
    uint lba_open_dir = fs_info.lba_cur_dir;
    FILE_INFO *p_file_info = file_info_cur_dir;
    uint checked_lba_file[100] = {0};
    checked_lba_file[0] = fs_info.lba_root_dir;
    // Start search from root dir
    fs_info.lba_cur_dir = fs_info.lba_root_dir;
    get_files_cur_dir();
    int num_files = 0;
    num_files = num_files_cur_dir;

    // If root dir is empty
    if(num_files == 0)
    {
	return fs_info.lba_root_dir;
    }

    int i = 1;
    while(1)
    {
	// No more files in cur dir back to root 
	if(num_files <= 0)
	{
	    // Add checked dir
	    checked_lba_file[i] = fs_info.lba_cur_dir;
	    // Back to root dir
	    fs_info.lba_cur_dir = fs_info.lba_root_dir;
	    // Get files of root dir
	    get_files_cur_dir();
	    num_files = num_files_cur_dir;
	    p_file_info = file_info_cur_dir;

	    i++;
	}

	// If we checked all files in root dir then finished
	if(fs_info.lba_cur_dir == fs_info.lba_root_dir)
	{
	    bool checked_all_files = true;
	    for( int l = 0; l < num_files_cur_dir; l++)
	    {
		if(!is_elem_in_arr(checked_lba_file, 100, file_info_cur_dir[l].lba_file))
		{
		    checked_all_files = false;
		}
	    }

	    if(checked_all_files)
	    {
		break;
	    }
	}

	// The file was checked
        if(is_elem_in_arr(checked_lba_file, 100, p_file_info->lba_file))
	{
	    p_file_info++;
	    num_files--;	
	}
	else if(p_file_info->attr == ATTR_DIR)
	{
	    fs_info.lba_cur_dir = p_file_info->lba_file;
	    get_files_cur_dir();
	    num_files = num_files_cur_dir;
	    p_file_info = file_info_cur_dir;
	}
	else if(p_file_info->attr == ATTR_ARCH)
	{
	    checked_lba_file[i] = p_file_info->lba_file; 
	    p_file_info++;
	    num_files--;	
	    i++;
	}
    }

    fs_info.lba_cur_dir = lba_open_dir;
    return get_largest_elem(checked_lba_file, 100);
}
bool is_elem_in_arr(uint *arr, int arr_size,uint elem)
{
    for(int i =0; i< arr_size; i++)
    {
	if(*arr == elem)
	{
	    return true;
	}
	arr++;
    }
    
    return false;
}

uint get_largest_elem(uint *arr, int arr_size)
{
    for(int i = 1; i < arr_size; i++)
    {
	if(arr[0] < arr[i])
	{
	    arr[0] = arr[i];
	}
    }

    return arr[0];

}

void execute_command(char *name)
{
    int name_len = strlen(name) + 1;
    char key[10];
    char value[name_len];
    int key_len = 0;

    for(int i =0; i < name_len; i++)
    {
	if(name[i] == '\n')
	{
	    if(key_len > 0)
	    {
		value[i - key_len] = '\0';	
	    }
	    else
	    {
		key[i] ='\0';
	    }
	    break;

	}
	else if(name[i] == ' ')
	{
	    key[i] = '\0';
	    key_len = i + 1;
	}
	else
	{
	    if(key_len > 0)
	    {
		value[i - key_len] = name[i];	
	    }
	    else
	    {
		key[i] = name[i];
	    }
	}
    }

    if(!fs_info.is_fat32 && (!strcmp(key, "format")))
    {
	format_disk();
    }
    else if(!fs_info.is_fat32)
    {
	printf("Unknown disk format\n");
	return;
    }

    if(!strcmp(key, "ls"))
    {
	list_files();
    }
    else if(!strcmp(key, "cd"))
    {
        if(value[0] == '/' && change_path(value))
	{
	    strcpy(fs_info.path_cur_dir, value);
	}
	else
	{
	    printf("Not such directory\n");
	    fs_info.lba_cur_dir = fs_info.lba_cur_dir;
	    fs_info.path_cur_dir[0] = '/';
	    fs_info.path_cur_dir[1] = '\0';
	}
        get_files_cur_dir();
    }
    else if(!strcmp(key, "mkdir"))
    {
    	if(create_file(value, ATTR_DIR))
	    printf("Ok\n");
	else
	    printf("Something went wrong\n");

	get_files_cur_dir();
    }
    else if(!strcmp(key, "touch"))
    {
	if(create_file(value, ATTR_ARCH))
	    printf("Ok\n");
	else
	    printf("File exists\n");

	get_files_cur_dir();
    }
    else if(!strcmp(key, "format"))
    {
	format_disk();
	printf("Ok\n");
    }
    else if(!strcmp(key, "exit"))
    {
	fclose(fs_info.fd);
	exit(EXIT_SUCCESS);
    }
    else
    {
	printf("Command '%s' not found\n", key);	
    }
}
void format_disk()
{
    fseek(fs_info.fd, 0L, SEEK_END);
    long int file_size = ftell(fs_info.fd);    

    ftruncate(fileno(fs_info.fd), 0);
    ftruncate(fileno(fs_info.fd), file_size);

    get_default_bpb(file_size);
    write_data(0, &bpb, sizeof(BPB));

    // Mark root dir
    SFN sfn = {0};
    sfn.filename[0] = 'R';
    sfn.filename[1] = 'O';
    sfn.filename[2] = 'O';
    sfn.filename[3] = 'T';
    sfn.attr[0] = ATTR_VOLUMEID;
    uint start_root_dir = get_lba_root_dir() * 512;

    write_data(start_root_dir, &sfn, sizeof(sfn));

    fs_info.is_fat32 = true;
    get_files_cur_dir();
    fs_info.path_cur_dir[0] = '/';
    fs_info.path_cur_dir[1] = '\0';
}
void list_files()
{
    get_files_cur_dir();
    for(int i = 0; i < num_files_cur_dir; i++)
    {
       if(file_info_cur_dir[i].attr == ATTR_DIR)
       {
	printf("%s/\n", file_info_cur_dir[i].name);
       }
       else if(file_info_cur_dir[i].attr == ATTR_ARCH)
       {

	printf("%s\n", file_info_cur_dir[i].name);
       }
    }
}

bool change_path(char *path)
{
    fs_info.lba_cur_dir = fs_info.lba_root_dir;
    get_files_cur_dir();
    char path_cpy[strlen(path) + 1];
    strcpy(path_cpy, path);
    char *dir_name;
    dir_name = strtok(path_cpy, "/");

    while(dir_name != NULL)
    {
	if(change_dir(dir_name))
	{
	    dir_name = strtok(NULL, "/");
	}
	else
	{
	    return false;
	}
    }
    return true;
}
bool change_dir(char *dir_name)
{
	
	for(int i = 0; i < num_files_cur_dir; i++)
	{
	    if(file_info_cur_dir[i].attr == ATTR_DIR && !strcmp(file_info_cur_dir[i].name, dir_name))
	    {
		fs_info.lba_cur_dir = file_info_cur_dir[i].lba_file;
		get_files_cur_dir();
		return true;
	    }
	}
	return false;
}

bool create_file(char* name, uchar attr)
{
   // Check if we have the same name in cur dir
   for(int i= 0; i < num_files_cur_dir; i++)
   {
	if(!strcmp(file_info_cur_dir[i].name, name))
	{
	    return false;
	}
   }

   int name_len = strlen(name);
   int lfn_size = ceil(name_len / 13.0f);
   lfn_size += 1; // alloc mem for SFN too
   LFN *lfn;

   lfn = calloc(lfn_size * sizeof(LFN), sizeof(uchar));

   for(int i =0; i < lfn_size - 1; i++)
   {
	lfn[i].attr[0] = ATTR_LFN;
   }

   fil_lfn_name(name, lfn, lfn_size - 1);
   // Add LFN sequence 
   lfn[0].sequence[0] = 0x40 + (lfn_size - 1);
    
   SFN *sfn = (SFN*)&lfn[lfn_size -1];
   sfn->attr[0] = attr;
   for( int i = 0; i < SFN_FILENAME; i++)
   {
	sfn->filename[i] = toupper(name[i]);
   }

   // Fill allocated cluster for new file
   uint lst_alloc_lba = get_last_alloc_lba();
   uint dir_clus = ((lst_alloc_lba/16) - (fs_info.lba_root_dir/16)) + 3;
   sfn->fstclushi[0] = dir_clus >> 24;
   sfn->fstclushi[1] = dir_clus >> 16;
   sfn->fstcluslo[1] = dir_clus >> 8;
   sfn->fstcluslo[0] = dir_clus;


   uint lba_dir_bytes = lst_alloc_lba * fs_info.byts_per_sec;
   
   uint start_point = (get_lst_sfn_index()* sizeof(SFN)) + fs_info.lba_cur_dir * fs_info.byts_per_sec;
   write_data(start_point, lfn, lfn_size * sizeof(LFN));
   return true;

}
