#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>

#define DIRECTORY "yun"
#define FILE_1	"file1"
#define FILE_2 "file2"

#define FILE_MAX_SIZE	100

static struct proc_dir_entry *test_dir, *testfile_1, *testfile_2;	// parent directory

static int num = 0;		// arguments iface_num
static char *str="iface";

struct file_manager {
	char *parent_name;
	char *file_name;
	char *file_num;
	
	struct proc_dir_entry *proc_file;
};

module_param(num,int,0);	// Loading Modules arguments, network interface iface_num
module_param(str,charp,0);

static struct file_manager *file_1;		// ai_result file
static struct file_manager *file_2;		// net_stat file

static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *pos)
{
	int len;
	int target_idx;
	int i;
	char *file_name = file->f_path.dentry->d_iname;
	char *parent_name = file->f_path.dentry->d_parent->d_iname;

	len = (count > FILE_MAX_SIZE) ? FILE_MAX_SIZE : count;
	
	for (i = 0; i < num; i++) {
		if (!strcmp(parent_name, FILE_1)) {
			if (!strcmp(file_name, file_1[i].file_name)) {
				target_idx = i;
				break;
			}
		}
		else if (!strcmp(parent_name, FILE_2)) {
			if (!strcmp(file_name, file_2[i].file_name)) {
				target_idx = i;
				break;
			}
		}
	}

	if (!strcmp(parent_name, FILE_1)){
			if (raw_copy_from_user(file_1[target_idx].file_num, buf, len))    //write ���н� error return
				return -EFAULT; // address error
	}
	else if (!strcmp(parent_name, FILE_2)){
			if (raw_copy_from_user(file_2[target_idx].file_num, buf, len))    //write ���н� error return
				return -EFAULT; // address error
	}

	if (!strcmp(parent_name, FILE_1)){
		file_1[target_idx].file_num[len] = 0x00;
	}
	else if (!strcmp(parent_name, FILE_2)){
		file_2[target_idx].file_num[len] = 0x00;
	}

	return len;
}

static int proc_show(struct seq_file *m, void *v)
{
	int target_idx;
	int i;
	char *file_name = m->file->f_path.dentry->d_iname;
	char *parent_name = m->file->f_path.dentry->d_parent->d_iname;

	for (i = 0; i < num; i++) {
		if (!strcmp(parent_name, FILE_1)) {
			if (!strcmp(file_name, file_1[i].file_name)) {
				target_idx = i;
			}
		}
		else if (!strcmp(parent_name, FILE_2)) {
			if (!strcmp(file_name, file_2[i].file_name)) {
				target_idx = i;
			}
		}
	}

	if (!strcmp(parent_name, FILE_1))
		seq_printf(m, "%s", file_1[target_idx].file_num); // ��� ����
	else if (!strcmp(parent_name, FILE_2))
		seq_printf(m, "%s", file_2[target_idx].file_num);

	return 0;
}

static int proc_open(struct inode *inode, struct  file *file)
{
	return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_fops = {
	//.owner = THIS_MODULE,
	.proc_open = proc_open,
  	.proc_read = seq_read,
 	.proc_write = write,
 	.proc_lseek = seq_lseek,
 	.proc_release = single_release,
};

static int mod_procfs_init(void)
{
	int i; 
	char *token;

	file_1=kmalloc(sizeof(struct file_manager *)*num,GFP_KERNEL);
	file_2=kmalloc(sizeof(struct file_manager *)*num,GFP_KERNEL);
	
	for(i=0;i<num;i++){
		file_1[i].file_name=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);
		file_2[i].file_name=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);	
	}

	test_dir = proc_mkdir(DIRECTORY, NULL);
	testfile_1 = proc_mkdir(FILE_1, test_dir);
	testfile_2 = proc_mkdir(FILE_2, test_dir);
	
	token=strsep(&str,";");	

	for(i=0;token!=NULL;i++){	
		strcpy(file_1[i].file_name, token);
		strcpy(file_2[i].file_name, token);
		printk("%s,%s",file_1[i].file_name,file_2[i].file_name);
		token=strsep(&str,";");
	}
	
	for (i = 0; i < num; i++) {
		file_1[i].parent_name=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);
		file_2[i].parent_name=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);
		
		strcpy(file_1[i].parent_name, FILE_1);
		strcpy(file_2[i].parent_name, FILE_2);
		
		file_1[i].file_num=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);
		file_2[i].file_num=(char *)kmalloc(sizeof(char)*FILE_MAX_SIZE,GFP_KERNEL);
		
		file_1[i].proc_file = proc_create(file_ai[i].file_name, 0646, testfile_1, &proc_fops);
		file_2[i].proc_file = proc_create(file_net[i].file_name, 0646, testfile_2, &proc_fops);

		if (file_1[i].proc_file == NULL) {
			printk("%s - %s: error \n", file_1[i].parent_name, file_ai[i].file_name);
			return -EEXIST;
		}

		if (file_2[i].proc_file == NULL) {
			printk("%s - %s: error \n", file_2[i].parent_name, file_net[i].file_name);
			return -EEXIST;
		}
	}

	printk("%s\n", __FUNCTION__);
	return 0;
}

static void mod_procfs_exit(void) {
	int i;
	for (i = 0; i < num; i++) {
		remove_proc_entry(file_1[i].file_name, testfile_1);
		remove_proc_entry(file_2[i].file_name, testfile_2);
	}
	
	remove_proc_entry(FILE_1,test_dir);
	remove_proc_entry(FILE_2,test_dir);
	remove_proc_entry(DIRECTORY,NULL);
	
	printk("%s\n", __FUNCTION__);
}

module_init(mod_procfs_init);
module_exit(mod_procfs_exit);


MODULE_LICENSE("yun");
