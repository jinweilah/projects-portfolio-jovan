/* 
* chardev.c: Creates a read&write character device driver
*/

#include <linux/kernel.h>   // print to kernel space
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>    // copy_from_user/copy_to_user
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/errno.h>    //linux error codes for C 
#include <linux/string.h>

#define SUCCESS 0
#define DEVICE_NAME "chardev"   //character device's name
#define BUF_LEN 512             // Max length of buffer size in power of 2

/* Declaration of chardev.c function prototypes */
int init_module(void);
void cleanup_module(void);
static int devi_open(struct inode *, struct file *);
static int devi_release(struct inode *, struct file *);
static ssize_t devi_read(struct file *, char *, size_t, loff_t *);
static ssize_t devi_write(struct file *, const char *, size_t, loff_t *);

// Global variables are declared as static, so are global within the file.
static int Major;               // Major number assigned to character device driver
static int Device_Open = 0;     // indicate device open? Used to prevent multiple access to device */
static char cd_Buffer[BUF_LEN]; // Buffer to store the sentences
static int counter = 1;         //counter for number of write done
static int counter2 = 1;        //counter for number of read done
/* pointer to the struct class */
static struct class *cls;
/* access functions */
static struct file_operations chardev_fops = 
{
    read : devi_read,
    write : devi_write,
    open : devi_open,
    release : devi_release
};

/*init_module function is called when the kernel module is loaded*/
int init_module(void)
{
    /* Registering device with the core*/
    Major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
    if (Major < 0) 
    {
        //printk alert with KERN_ALERT an alert-level message; 
        pr_alert("Registering char device failed with %d\n", Major);
        return Major;
    }

    //printk info with KERN_INFO loglevel, an info-level message
    pr_info("I was assigned major number %d.\n", Major);

    //create a virtual device class class_create(owner, name), /sys/class/chardev
    cls = class_create(THIS_MODULE, DEVICE_NAME);
    //create a character device and register it with sysfs
    device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
    //device name appear on dev/chardev
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return SUCCESS;
}
/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    device_destroy(cls, MKDEV(Major, 0)); //destroy character device
    class_destroy(cls); //destroys a struct class structure

    /* Unregister the device/Freeing the major number */
    unregister_chrdev(Major, DEVICE_NAME);
}

/* Called when user space app tries to open the character device file */
static int devi_open(struct inode *inode, struct file *file)
{
    if (Device_Open)    //count usage more than 0
        return -EBUSY;  //charaacter device busy

    Device_Open++;  //increment usage count
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/* Called when user space app tries to close the character device file */
static int devi_release(struct inode *inode, struct file *file)
{
    counter =1;     //reset write counter to 1
    counter2 =1;    //reset read counter to 1
    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    Device_Open--;          /*Ready for our next caller */

    module_put(THIS_MODULE);

    return SUCCESS;
}

/* Called when system call from user space, in which already opened the dev file, 
 * attempts to read from it. */
static ssize_t devi_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with sentence */
                           size_t length,       /* length of the buffer     */
                           loff_t *offset)
{
    size_t bytes_read = 0;
    int error_check = 0;
    //copies data from kernel data segment to the user data segment
    error_check = copy_to_user(buffer, cd_Buffer, length);

    if (error_check)
    {
        return -EFAULT; //Invalid address: Unable to read buffer from user
    }
    bytes_read = strlen(buffer)-1;   //number of bytes received from user buffer

    //print messages to kernel space based on project's requirements
    printk(KERN_INFO "%d no. of letters send from kernel space to user space application\n", bytes_read); 
    printk(KERN_INFO "Device has been read by %d times into kernal space\n", counter2++); 

    return bytes_read;  //return the number of letters in the sentences
}

/* Called when system call from user space, in which already opened the dev file, 
 * attempts to write into it. */
static ssize_t devi_write(struct file *filp,
                            const char *buff,
                            size_t len,
                            loff_t *off)
{
    size_t bytes_write = 0;
    int error_check =0;
    memset(cd_Buffer, 0, BUF_LEN);  //clear character device buffer

    //copies data from user data segment to the kernel data segment
    error_check = copy_from_user(cd_Buffer, buff, len);
    if (error_check)
    {
        return -EINVAL; //return invalid argument error
    }
    bytes_write = strlen(cd_Buffer)-1;    //number of bytes received from device buffer

    //print messages to kernel space based on project's requirements
    printk(KERN_INFO "%d no. of letters recevied from user application program\n", bytes_write); 
    printk(KERN_INFO "Device has been written by %d times into kernel space\n", counter++); 

    return bytes_write; //return the number of letters in the sentences
}

MODULE_LICENSE("GPL");                  	// The license type 
MODULE_DESCRIPTION("Read&Write Char Dev Module"); // The description of the loadable kernel module
MODULE_AUTHOR("P3-Group10-CSC1007");         // The author of the loadable kernel module
MODULE_VERSION("0.1a");                  	// The version of the loadable kernel module

