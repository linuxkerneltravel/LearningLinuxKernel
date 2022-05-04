#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/path.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/list.h>

//模块许可证声明（必须）
MODULE_LICENSE("GPL");
//模块加载函数（必须）
static int test_dentry_init(void)
{
    //获取当前文件的入口目录
    struct dentry *dentry_pwd = current->fs->pwd.dentry;

    // 获取当前文件的上一级入口目录
    struct dentry *dentry_parent = current->fs->pwd.dentry->d_parent;

    // 获取当前文件的入口目录名结构体
    struct qstr name = current->fs->pwd.dentry->d_name;


    //显示父目录名称
    printk("The name of dentry_parent is %s\n", dentry_parent->d_name.name);

    //判断父子目录项对应super_block是否一样
    printk("dentry_parent->d_sb = %p, dentry_pwd->d_sb = %p\n", dentry_parent->d_sb, dentry_pwd->d_sb);

    struct dentry *dentry=NULL;
    // 分配一个目录项缓存结构体
    dentry = d_alloc(dentry_parent, &name);

    // 显示当前文件名
    printk("The name of current dentry is %s\n", dentry_pwd->d_name.name);

    // 显示返回结果文件名
    printk("After \"d_alloc\" the name of the dentry is %s\n", dentry->d_name.name);

    //判断d_alloc是否有对dentry的d_hash项和d_lru项进行初始化
    printk("dentry.d_hash is unhashed: %d, dentry.d_lru is NULL: %d\n", d_unhashed(dentry), dentry->d_lru.next == NULL);

    
    //遍历current dentry的子目录项链表
    struct dentry *tmp = NULL;
    int i = 0;
    if(!spin_trylock(&dentry_parent->d_lockref.lock)){
	printk("try lock fail\n");
        return 1;    
    }
    list_for_each_entry(tmp, &dentry_parent->d_subdirs, d_child){
	/*排除掉负状态的dentry，未添加这个判读的时候
        会将很多inode为空的dentry name打印出来数量有
	上千个甚至更多个，所以就可以理解为什么dentry
	在路径查找的时候会借助dentry_hashtable了*/
	if(tmp->d_inode == NULL)   
		continue;
	if(!spin_trylock(&tmp->d_lockref.lock)){
            printk("try lock fail\n");
            return 1;
        }

        printk("dentry_parent's child%d name is: %s\n", i++, tmp->d_name.name);
	
	spin_unlock(&tmp->d_lockref.lock);
    }
    spin_unlock(&dentry_parent->d_lockref.lock);

    return 0;
}
//模块卸载函数（必须）
static void test_dentry_exit(void)
{
    printk(KERN_INFO "Hello World exit/n");
}

module_init(test_dentry_init);
module_exit(test_dentry_exit);
