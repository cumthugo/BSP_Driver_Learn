/*************************************************************************
	> File Name: second.c
# Author: Zhang Yong
# mail: cumt_zhangyong@163.com
	> Created Time: Mon 28 Mar 2016 09:44:09 AM CST
 ************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#define SECOND_MAJOR 152

static int second_major = SECOND_MAJOR;

struct second_dev
{
	struct cdev cdev;
	atomic_t counter;
	struct delayed_work s_delaywork;
};

struct second_dev* second_devp;

static void second_delay_work_handle(struct work_struct* work)
{
	struct delayed_work* p_delay_work = (struct delayed_work*)work;
	
	schedule_delayed_work(p_delay_work,HZ);
	
	atomic_inc(&second_devp->counter);
	
	printk(KERN_NOTICE"current jiffies is %ld\n",jiffies);
	
}

int second_open(struct inode* inode, struct file* filp)
{
	INIT_DELAYED_WORK(&second_devp->s_delaywork,second_delay_work_handle);
	
	schedule_delayed_work(&second_devp->s_delaywork,HZ);
	
	atomic_set(&second_devp->counter,0);
	
	return 0;
}

int second_release(struct inode* inode, struct file* filp)
{
	cancel_delayed_work(&second_devp->s_delaywork);
	return 0;
}

static ssize_t second_read(struct file* filp, char __user *buf, size_t count, loff_t *ppos)
{
	int counter;
	counter = atomic_read(&second_devp->counter);
	if (put_user(counter,(int*)buf))
		return - EFAULT;
	else
		return sizeof(unsigned int);
}



static const struct file_operations second_fops = {
	.owner = THIS_MODULE,
	.read = second_read,
	.open = second_open,
	.release = second_release,
};

void second_setup_cdev(struct second_dev* dev, int index)
{
	int err, devno = MKDEV(second_major,index);

	cdev_init(&dev->cdev, &second_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev,devno,1);
	if(err)
		printk(KERN_NOTICE"Error %d adding second",err);
}

int second_init(void)
{
	int result;
	dev_t devno = MKDEV(second_major,0);
	printk("[Zhang Yong] second_init start\n");
	
	
	if (second_major)
	{
		result = register_chrdev_region(devno,1,"second");
	}
	else
	{
		result = alloc_chrdev_region(&devno,0,1,"second");
		second_major = MAJOR(devno);
	}

	if (result < 0)
		return result;

	second_devp = kmalloc(sizeof(struct second_dev),GFP_KERNEL);

	if(!second_devp)
	{
		result = -ENOMEM;
		goto fail_malloc;
	}

	memset(second_devp,0,sizeof(struct second_dev));

	second_setup_cdev(second_devp,0);
	return 0;
fail_malloc:
	unregister_chrdev_region(devno,1);
	return result;
}

void second_exit(void)
{
	cdev_del(&second_devp->cdev);
	kfree(second_devp);
	unregister_chrdev_region(MKDEV(second_major,0), 1);
}
module_param(second_major,int,S_IRUGO);
module_init(second_init);
module_exit(second_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hugo@yfve");
MODULE_DESCRIPTION("A second char device driver as an example");

