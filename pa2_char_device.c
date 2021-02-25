#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/uaccess.h>


#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");

/* Define device_buffer and other global data structures you will need here */

char* myBuffer;
char* tempBuffer;



int open_count = 0;
int close_count = 0;

int currentPosition = 0;


ssize_t pa2_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	*offset = currentPosition;
	if (strlen(myBuffer) == 0){
		printk("Nothing to read!\n");
		return -1;
	}
	printk("Read: Offset before: %lld\n", *offset);
	printk("Read: Length: %ld\n", length);

	if (strlen(myBuffer) > length){
		printk("User buffer is not big enough!\n");
		return -1;
	
	}

	if (*offset >= BUFFER_SIZE){
		printk("Trying to read past file!\n");
		return -1;
	}

	copy_to_user(buffer, myBuffer+*offset, length);
	
	*offset = *offset + length;
	printk("Read: Offset after: %lld\n", *offset);
	printk("Read: len of my buffer: %ld\n", strlen(myBuffer));
	currentPosition = *offset;
	return(length);
	
}
 


ssize_t pa2_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	
	tempBuffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	memset(tempBuffer, 0, BUFFER_SIZE);	
	*offset = currentPosition;

	
	if (*offset + length > BUFFER_SIZE){
		printk("Hitting past buffer!\n");
		return -1;
	}

	printk("Write: len of buffer before: %ld\n", strlen(myBuffer));
	printk("Write: Offset before: %lld\n", *offset);


	copy_from_user(tempBuffer, buffer, length);
	printk("lenTempBuffer: %ld\n", strlen(tempBuffer));
	strcpy(myBuffer+*offset, tempBuffer);
	//strcat adds tempBuffer to the end of myBuffer, position should be drawn out to demonstrate. Expression is to get accurate concatenation when file pointer isn't at the end. 
	*offset += strlen(tempBuffer);
	printk("Write: Offset after: %lld\n", *offset);
	printk("Write: len of buffer after: %ld\n", strlen(myBuffer));
	currentPosition = *offset;
	kfree(tempBuffer);
	return length;
	
}


int pa2_char_driver_open (struct inode *pinode, struct file *pfile)
{
	printk("Entering open\n");
	//open_count = open_count + 1;
	//printk(KERN_ALERT "Device is opened, for the %i time\n", open_count);
	return 0;
}

int pa2_char_driver_close (struct inode *pinode, struct file *pfile)
{
	printk("Entering close\n");
	//close_count = close_count + 1;
	//printk(KERN_ALERT "Device is closed, for the %i time\n", close_count);
	return 0;
}

loff_t pa2_char_driver_seek (struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */
	if (whence == 0){
		currentPosition = offset;
		return 0;
	}
	else if (whence == 1){
		currentPosition += offset;
		return 0;
	}
	else if (whence == 2){
		currentPosition = BUFFER_SIZE;
		return 0;
	}
	printk("Invalid input \n");
	return -1;
}


struct file_operations my_file_operations = {
.open = &pa2_char_driver_open,
.owner = THIS_MODULE,
.release =  &pa2_char_driver_close,
.read = pa2_char_driver_read,
.write = pa2_char_driver_write,
.llseek = pa2_char_driver_seek
};

static int pa2_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	/* register the device */
	
	myBuffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	memset(myBuffer, 0, BUFFER_SIZE);
	printk(KERN_ALERT "Init Function Called\n");
	register_chrdev(511, "simple_character_dev", &my_file_operations);
	
	return 0;
}

static void pa2_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the (un?)register_chrdev() function. */
	kfree(myBuffer);
	printk(KERN_ALERT "Exit Function Called\n");
	unregister_chrdev(511, "simple_character_dev");
	
	
}

module_init(pa2_char_driver_init);
module_exit(pa2_char_driver_exit);

/* add module_init and module_exit to point to the corresponding init and exit function*/


